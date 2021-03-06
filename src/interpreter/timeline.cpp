#include "interpreter/timeline.h"
#include "interpreter/exception.h"
#include "interpreter/image.h"
#include "interpreter/value.h"
#include "interpreter/ffi_conversion.h"
#include "interpreter.h"

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

Timeline::Timeline(Program *program) :
    program_(program)
{
}

void Timeline::add_download(ActiveDownload *dwnld, const std::string &var_id) {
    if (!dwnld)
        InterpreterException("Empty download");

    DwnldData ddata{
        .var_id = var_id,
        .cur_frame = DOWNLOAD_FIRST_FRAME(dwnld, start.value_or(0)),
        .next_frame = dwnld->download()
    };
    downloads_data_.emplace(dwnld, ddata);
}

bool Timeline::prepare_iteration() {
    cur_time = std::numeric_limits<float>::max();
    for (auto &dwnld_data : downloads_data_) {
        auto &cur_frame = dwnld_data.second.cur_frame;
        if (cur_frame.is_valid() && cur_frame.timestamp < cur_time) {
            cur_time = cur_frame.timestamp;
        }
    }

    bool result = false;
    for (auto &dwnld_data : downloads_data_) {
        auto dwnld = dwnld_data.first;
        auto &cur_frame = dwnld_data.second.cur_frame;
        auto &next_frame = dwnld_data.second.next_frame;
        auto &var_id = dwnld_data.second.var_id;

        auto variable_it = program_->variables_.find(var_id);
        if (variable_it == program_->variables_.end()) {
            throw InterpreterException("Variable with id '" + var_id + "' does not exist");
        }
        if (cur_frame.is_valid()) {
            if (cur_frame.timestamp <= cur_time) {
                variable_it->second = cur_frame.value;
                cur_frame = HandlerDownloadResponse::new_not_ready();
            }
        }
        if (next_frame.is_valid() && (!cur_frame.is_valid() || next_frame.timestamp <= cur_time)) {
            cur_frame = dwnld_data.second.next_frame;
            auto tmp = dwnld->download();
            if (tmp.is_valid() && (!end || tmp.timestamp <= end))
                dwnld_data.second.next_frame = tmp;
            else
                dwnld_data.second.next_frame = HandlerDownloadResponse::new_not_ready();
        }
        if (cur_frame.is_valid() || next_frame.is_valid()) {
            result = true;
        }
    }
    return result;
}
