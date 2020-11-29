#pragma once

#include <unordered_map>
#include <vector>
#include <stdbool.h>

#include "interpreter/handler.h"

struct DwnldData {
    std::string var_id;
    const AsaData *cur_frame;
    const AsaData *next_frame;
};

class Program;

class Timeline {
public:
    Timeline(Program *program);
    Timeline(const Timeline&) = delete;
    Timeline(Timeline&&) = delete;

    void add_download(ActiveDownload *dwnld, const std::string &var_id);

    bool prepare_iteration();
private:
    Program *program_;
private:
    std::unordered_map<ActiveDownload *, DwnldData> downloads_data_;

    float cur_time = 0;
};