#pragma once

#include <stdexcept>

class InterpreterException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};
