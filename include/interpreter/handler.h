#pragma once

#include <functional>
#include <string>
#include <fstream>
#include <memory>
#include <optional>

#include <dynalo/dynalo.hpp>

#include "handler_interface.h"
#include "interpreter/exception.h"
#include "interpreter/image.h"

class Handler {
public:
    Handler(const std::string& path);

    std::function<AsaHandler*()> open_download;
    std::function<AsaHandler*()> open_upload;
    std::function<void(AsaHandler*)> close;

    std::function<int(AsaHandler*, const AsaData*)> push;

    std::function<AsaValueType(AsaHandler*)> get_type;
    std::function<AsaData*(AsaHandler*)> download;
    std::function<AsaData*(AsaHandler*)> upload;
    std::function<void(AsaHandler*, AsaData*)> free;

protected:
    dynalo::library library_;
};

struct ActiveDownload {
    std::ifstream stream_in;
    AsaHandler* handler_ctx;
    Handler* handler;

    ActiveDownload(const std::string& filename, Handler* handler);
    ~ActiveDownload();

    ActiveDownload(const ActiveDownload&) = delete;
    ActiveDownload(ActiveDownload&& other);

    bool fill_data();
    std::optional<AsaImage> download_frame();
};
