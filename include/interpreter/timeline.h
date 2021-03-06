#pragma once

#include <unordered_map>
#include <vector>
#include <stdbool.h>
#include <optional>

#include "interpreter/handler.h"

namespace Asampl::Interpreter {

class Program;

}

namespace Asampl::Interpreter::Timeline {

struct DwnldData {
    std::string var_id;
    Handler::DownloadResponse cur_frame;
    Handler::DownloadResponse next_frame;
};

class Timeline {
public:
    Timeline(Program *program);
    Timeline(const Timeline&) = delete;
    Timeline(Timeline&&) = delete;

    void add_download(Handler::ActiveDownload *dwnld, const std::string &var_id);

    bool prepare_iteration();

    std::optional<double> start;
    std::optional<double> end;
private:
    Program *program_;
private:
    std::unordered_map<Handler::ActiveDownload*, DwnldData> downloads_data_;

    double cur_time = 0;
};

}

