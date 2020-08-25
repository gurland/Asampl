#pragma once

#include <functional>
#include <string>

#include <dynalo/dynalo.hpp>

#include "handler_interface.h"

class Handler {
public:
    Handler(const std::string& path);

    std::function<AsaHandler*()> open;
    std::function<void(AsaHandler*)> close;

    std::function<int(AsaHandler*, const AsaData*)> push;

    std::function<AsaValueType(AsaHandler*)> get_type;
    std::function<AsaData*(AsaHandler*)> download;
    std::function<AsaData*(AsaHandler*)> upload;
    std::function<void(AsaHandler*, AsaData*)> free;

protected:
    dynalo::library library_;
};
