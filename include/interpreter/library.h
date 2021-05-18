#pragma once

#include <filesystem>
#include <memory>

#include "interpreter/value.h"

namespace Asampl::Interpreter::Library {

class ILibrary {
public:
    virtual ~ILibrary() = default;

    virtual ValuePtr get_value() = 0;
};

std::unique_ptr<ILibrary> open_library(const std::filesystem::path& path);

}

