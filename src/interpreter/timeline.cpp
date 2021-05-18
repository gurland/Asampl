#include "interpreter/timeline.h"
#include "interpreter/exception.h"
#include "interpreter/value.h"
#include "interpreter/ffi_conversion.h"
#include "interpreter.h"

#include <string>
#include <limits>
#include <cstdlib>
#include <memory>

#define FCOMP(f1, f2) std::abs((f1) - (f2)) < 0.001

#define DOWNLOAD_FIRST_FRAME(dwnld, start_time)    \
    ({                                             \
        auto __frame = dwnld->download(); \
        while(__frame.is_valid()) {                \
            if (__frame.timestamp >= start_time)   \
                break;                             \
            __frame = dwnld->download();  \
        }                                          \
        std::move(__frame);                        \
    })

namespace Asampl::Interpreter::Timeline {

using namespace Handler;

Timeline::Timeline(Map& params, Function& callback)
    : params{params}
    , callback{callback}

{
    if (params.has("from")) {
        start = params.get("from")->get<Number>().value;
    }
    if (params.has("to")) {
        end = params.get("to")->get<Number>().value;
    }

    for (const auto& handler_params_ptr : params.strict_get("downloads")->get<Tuple>().values) {
        auto& handler_params = handler_params_ptr->get<Map>();
        const auto& handler = handler_params.strict_get("handler")->get<HandlerValue>().handler_ptr;
        const auto& source = handler_params.strict_get("source")->get<String>().value;

        active_downloads.emplace_back(std::make_unique<ActiveDownload>(source, *handler));

        Handler::ActiveDownload* dwnld = active_downloads.back().get();
        DwnldData ddata{
            .cur_frame = DOWNLOAD_FIRST_FRAME(dwnld, start.value_or(0)),
            .next_frame = dwnld->download()
        };
        downloads_data.insert(std::make_pair(dwnld, std::move(ddata)));
    }
}

void Timeline::run() {
    while (iteration()) {}
}

bool Timeline::iteration() {
    cur_time = std::numeric_limits<float>::max();
    for (auto &dwnld_data : downloads_data) {
        auto &cur_frame = dwnld_data.second.cur_frame;
        if (cur_frame.is_valid() && cur_frame.timestamp < cur_time) {
            cur_time = cur_frame.timestamp;
        }
    }
    
    if (cur_time > end.value_or(std::numeric_limits<float>::max())) {
        return false;
    }

    std::vector<ValuePtr> args;
    args.reserve(downloads_data.size());

    bool result = false;
    bool all_current_valid = true;
    for (auto& [dwnld, dwnld_data] : downloads_data) {
        auto &cur_frame = dwnld_data.cur_frame;
        auto &next_frame = dwnld_data.next_frame;

        ValuePtr value = Undefined{};

        if (cur_frame.is_valid()) {
            value = cur_frame.value;
            if (cur_frame.timestamp <= cur_time) {
                cur_frame = Handler::DownloadResponse::new_not_ready();
            }
        } else {
            all_current_valid = false;
        }
        if (next_frame.is_valid() && (!cur_frame.is_valid() || next_frame.timestamp <= cur_time)) {
            cur_frame = dwnld_data.next_frame;
            auto tmp = dwnld->download();
            if (tmp.is_valid() && (!end || tmp.timestamp <= end))
                dwnld_data.next_frame = tmp;
            else
                dwnld_data.next_frame = DownloadResponse::new_not_ready();
        }
        if (cur_frame.is_valid() || next_frame.is_valid()) {
            result = true;
        }

        args.push_back(std::move(value));
    }

    if (all_current_valid) {
        callback.func(args);
    }

    return result;
}

}
