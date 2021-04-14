#pragma once

#include <filesystem>
#include <memory>

#include "interpreter/function.h"

namespace Asampl::Interpreter::Library {

class ILibrary {
public:
    virtual ~ILibrary() = default;

    virtual std::vector<Function> get_functions() = 0;
};

std::unique_ptr<ILibrary> open_library(const std::filesystem::path& path);

}

