#pragma once

#include <filesystem>
#include <memory>
#include <vector>
#include <fstream>

#include "interpreter/value.h"

namespace Asampl::Interpreter::Handler {

struct DownloadResponse {
    bool enough_data;
    double timestamp;
    ValuePtr value;

    bool is_valid() const {
        return enough_data && value != nullptr;
    }

    static DownloadResponse new_out_of_data() {
        return DownloadResponse { false, 0.0, nullptr };
    }

    static DownloadResponse new_not_ready() {
        return DownloadResponse { true, 0.0, nullptr };
    }

    static DownloadResponse new_valid(ValuePtr value, double timestamp) {
        return DownloadResponse { true, timestamp, std::move(value) };
    }
};

struct UploadResponse {
    std::vector<std::uint8_t> bytes;
    bool enough_data;

    bool is_valid() const {
        return enough_data && bytes.size() != 0;
    }

    bool is_finished() const {
        return enough_data && bytes.size() != 0;
    }

    bool is_not_enough_data() const {
        return !enough_data;
    }

    static UploadResponse new_valid(std::vector<uint8_t> bytes) {
        return UploadResponse { std::move(bytes), true };
    }

    static UploadResponse new_not_ready() {
        return UploadResponse { {}, false };
    }
};

class IHandlerContextDownload {
public:
    virtual ~IHandlerContextDownload() = default;

    virtual void push(const std::vector<uint8_t>& data) = 0;
    virtual DownloadResponse download() = 0;
};

class IHandlerContextUpload {
public:
    virtual ~IHandlerContextUpload() = default;

    virtual UploadResponse pull() = 0;
    virtual void upload(const ValuePtr& value) = 0;
};

class IHandler {
public:
    virtual ~IHandler() = default;

    virtual std::unique_ptr<IHandlerContextDownload> open_download() = 0;
    virtual std::unique_ptr<IHandlerContextUpload> open_upload() = 0;
};

std::unique_ptr<IHandler> open_handler(std::filesystem::path path);

struct ActiveDownload {
    std::ifstream stream_in;
    std::unique_ptr<IHandlerContextDownload> context;

    ActiveDownload(const std::filesystem::path& filename, IHandler& handler);

    bool fill_data();
    DownloadResponse download();
};

}

