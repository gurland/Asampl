#pragma once

#include <filesystem>
#include <memory>
#include <vector>
#include <fstream>

#include "interpreter/value.h"

struct HandlerResponse {
    bool enough_data;
    double timestamp;
    ValuePtr value;

    bool is_valid() const {
        return enough_data && value != nullptr;
    }

    static HandlerResponse new_out_of_data() {
        return HandlerResponse { false, 0.0, nullptr };
    }

    static HandlerResponse new_not_ready() {
        return HandlerResponse { true, 0.0, nullptr };
    }

    static HandlerResponse new_ready(ValuePtr value, double timestamp) {
        return HandlerResponse { true, timestamp, std::move(value) };
    }
};

class IHandlerContextDownload {
public:
    virtual ~IHandlerContextDownload() = default;

    virtual void push(const std::vector<uint8_t>& data) = 0;
    virtual HandlerResponse download() = 0;
};

class IHandler {
public:
    virtual ~IHandler() = default;

    virtual std::unique_ptr<IHandlerContextDownload> open_download() = 0;
};

std::unique_ptr<IHandler> open_handler(std::filesystem::path path);

struct ActiveDownload {
    std::ifstream stream_in;
    std::unique_ptr<IHandlerContextDownload> context;

    ActiveDownload(const std::filesystem::path& filename, IHandler& handler);

    bool fill_data();
    HandlerResponse download();
};
