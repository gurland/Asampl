#pragma once

#include <vector>
#include <utility>

#include "interpreter/function.h"

std::vector<std::pair<std::string, Function>> get_stdlib_functions();
