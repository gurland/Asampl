#pragma once

#include <filesystem>
#include <memory>

#include "interpreter/function.h"

class ILibrary {
public:
    virtual ~ILibrary() = default;

    virtual std::vector<std::pair<std::string, Function>> get_functions() = 0;
};

std::unique_ptr<ILibrary> open_library(const std::filesystem::path& path);
