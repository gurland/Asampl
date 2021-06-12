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

        Handler::ActiveDownload* active_download = active_downloads.back().get();
        cur_frames.emplace_back(DOWNLOAD_FIRST_FRAME(active_download, start.value_or(0)));
        next_frames.emplace_back(active_download->download());
        prev_frames.emplace_back(Handler::DownloadResponse::new_out_of_data());
    }
}

void Timeline::run() {
    while (iteration()) {}
}

#define def_related_to_index \
    auto &cur_frame = cur_frames[i]; \
    auto &next_frame = next_frames[i]; \
    auto &prev_frame = prev_frames[i];

bool Timeline::iteration() {
    cur_time = std::numeric_limits<float>::max();
    for (auto &frame : cur_frames) {
        if (frame.is_valid() && frame.timestamp < cur_time)
            cur_time = frame.timestamp;
    }
    
    if (cur_time > end.value_or(std::numeric_limits<float>::max())) {
        return false;
    }

    bool result = false;
    uint32_t ad_count = active_downloads.size();
    std::vector<ValuePtr> args;
    args.reserve(ad_count);

    for (uint32_t i = 0; i < ad_count; i++) {
        def_related_to_index;
        ValuePtr value = prev_frame.value;

        if (cur_frame.is_valid() && cur_frame.timestamp <= cur_time) {
            value = cur_frame.value;
            prev_frame = cur_frame;
            cur_frame = next_frame;
            if (next_frame.is_valid()) {
                auto tmp_frame = active_downloads[i]->download();
                next_frame = (tmp_frame.is_valid() && (!end || tmp_frame.timestamp <= end)) ?
                    tmp_frame : DownloadResponse::new_not_ready();
            }
            result = true;
        }

        args.push_back(std::move(value));
    }
    callback.func(args);

    return result;
}

}
