#pragma once

#include <filesystem>
#include <memory>
#include <vector>
#include <fstream>

#include "interpreter/value.h"

struct HandlerDownloadResponse {
    bool enough_data;
    double timestamp;
    ValuePtr value;

    bool is_valid() const {
        return enough_data && value != nullptr;
    }

    static HandlerDownloadResponse new_out_of_data() {
        return HandlerDownloadResponse { false, 0.0, nullptr };
    }

    static HandlerDownloadResponse new_not_ready() {
        return HandlerDownloadResponse { true, 0.0, nullptr };
    }

    static HandlerDownloadResponse new_valid(ValuePtr value, double timestamp) {
        return HandlerDownloadResponse { true, timestamp, std::move(value) };
    }
};

struct HandlerUploadResponse {
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

    static HandlerUploadResponse new_valid(std::vector<uint8_t> bytes) {
        return HandlerUploadResponse { std::move(bytes), true };
    }

    static HandlerUploadResponse new_not_ready() {
        return HandlerUploadResponse { {}, false };
    }
};

class IHandlerContextDownload {
public:
    virtual ~IHandlerContextDownload() = default;

    virtual void push(const std::vector<uint8_t>& data) = 0;
    virtual HandlerDownloadResponse download() = 0;
};

class IHandlerContextUpload {
public:
    virtual ~IHandlerContextUpload() = default;

    virtual HandlerUploadResponse pull() = 0;
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
    HandlerDownloadResponse download();
};
