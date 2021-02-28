#pragma once

#include <functional>
#include <string>
#include <fstream>
#include <memory>
#include <optional>

#include <dynalo/dynalo.hpp>

extern "C" {
#include <asampl-ffi/ffi.h>
}

#include "interpreter/exception.h"
#include "interpreter/image.h"
#include "interpreter/value.h"

class TimeLine;

class Handler;

class HandlerContextDownload {
public:
    void push(const std::vector<uint8_t>& data);
    AsaHandlerResponse download();

    ~HandlerContextDownload();

    HandlerContextDownload(const HandlerContextDownload& other) = delete;
    HandlerContextDownload(HandlerContextDownload&& other) = delete;

protected:
    HandlerContextDownload(Handler* handler);

    Handler* handler_;
    AsaHandler* context_;

private:
    friend class Handler;
};

class Handler {
public:
    Handler(const std::string& path);

    std::unique_ptr<HandlerContextDownload> open_download();
    //std::unique_ptr<HandlerContext> open_upload();

protected:
    dynalo::library library_;

    std::function<AsaHandler*()> open_download_;
    std::function<AsaHandler*()> open_upload_;
    std::function<void(AsaHandler*)> close_;

    std::function<int(AsaHandler*, const AsaBytes*)> push_;

    std::function<AsaHandlerResponse(AsaHandler*)> download_;
    std::function<AsaHandlerResponse(AsaHandler*)> upload_;

private:
    friend class HandlerContextDownload;
};

struct ActiveDownload {
    std::ifstream stream_in;
    std::unique_ptr<HandlerContextDownload> handler_ctx;

    ActiveDownload() {};
    ActiveDownload(const std::string& filename, Handler* handler);

    ActiveDownload(const ActiveDownload&) = delete;
    ActiveDownload(ActiveDownload&&) = default;

    bool fill_data();
    ValuePtr download_frame_val();

private:
    AsaValueContainer* download_frame();
    ValuePtr frame_to_value(AsaValueContainer* data);
private:
    friend class Timeline;
};
