#pragma once

#include <unordered_map>
#include <vector>
#include <stdbool.h>
#include <optional>

extern "C" {
#include <asampl-ffi/types.h>
}

#include "interpreter/handler.h"

struct DwnldData {
    std::string var_id;
    AsaValueContainer* cur_frame;
    AsaValueContainer* next_frame;
};

class Program;

class Timeline {
public:
    Timeline(Program *program);
    Timeline(const Timeline&) = delete;
    Timeline(Timeline&&) = delete;

    void add_download(ActiveDownload *dwnld, const std::string &var_id);

    bool prepare_iteration();

    std::optional<double> start;
    std::optional<double> end;
private:
    Program *program_;
private:
    std::unordered_map<ActiveDownload *, DwnldData> downloads_data_;

    double cur_time = 0;
};
