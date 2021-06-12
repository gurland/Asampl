#pragma once

#include <map>
#include <vector>
#include <optional>

#include "interpreter/value.h"
#include "interpreter/handler.h"

namespace Asampl::Interpreter {

class Program;

}

namespace Asampl::Interpreter::Timeline {

struct DwnldData {
    Handler::DownloadResponse cur_frame;
    Handler::DownloadResponse next_frame;
};

class Timeline {
public:
    Timeline(Map& params, Function& callback);
    Timeline(const Timeline&) = delete;
    Timeline(Timeline&&) = delete;

    void run();
    bool iteration();

    std::optional<double> start;
    std::optional<double> end;
private:
    Map& params;
    Function& callback;

    std::vector<std::unique_ptr<Handler::ActiveDownload>> active_downloads;
    std::vector<Handler::DownloadResponse> cur_frames = {};
    std::vector<Handler::DownloadResponse> prev_frames = {};
    std::vector<Handler::DownloadResponse> next_frames = {};

    double cur_time = 0;
};

}

