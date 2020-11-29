
#include "interpreter/timeline.h"
#include "handler_interface.h"
#include "interpreter.h"
#include "interpreter/exception.h"
#include "interpreter/image.h"
#include "interpreter/value.h"

#include <limits>
#include <cstdlib>
#include <memory>

#define FCOMP(f1, f2) std::abs((f1) - (f2)) < 0.001

Timeline::Timeline(Program *program) : 
    program_(program)
{
}

void Timeline::add_download(ActiveDownload *dwnld, const std::string &var_id) {
    if (!dwnld)
        InterpreterException("Empty download");

    DwnldData ddata{
        .var_id = var_id,
        .cur_frame = dwnld->download_frame(),
        .next_frame = dwnld->download_frame()
    };
    downloads_data_.emplace(dwnld, ddata);
}

bool Timeline::prepare_iteration() {
    cur_time = std::numeric_limits<float>::max();
    for (auto &dwnld_data : downloads_data_) {
        // auto dwnld = dwnld_data.first;
        // auto frame = dwnld_data.second.frame;
        auto &cur_frame = dwnld_data.second.cur_frame;
        // if (!cur_frame) {
        //     cur_frame = dwnld_data.second.next_frame;
        //     dwnld_data.second.next_frame = dwnld->download_frame();
        // }
        if (cur_frame && cur_frame->time < cur_time) {
            cur_time = cur_frame->time;
        }
        // if (cur_frame != frame)
        //     dwnld_data.second.cur_frame = frame;
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
        if (cur_frame) {
            if (cur_frame->time <= cur_time) {
                auto ret = dwnld->frame_to_value(cur_frame);
                dwnld->handler->free(dwnld->handler_ctx, cur_frame);
                cur_frame = nullptr;
                variable_it->second = ret;
            }
        }
        if (next_frame && (!cur_frame || next_frame->time <= cur_time)) {
            cur_frame = dwnld_data.second.next_frame;
            dwnld_data.second.next_frame = dwnld->download_frame();
        }
        if (cur_frame || next_frame) {
            result = true;
        }
    }
    return result;
}