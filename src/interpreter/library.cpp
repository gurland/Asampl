#include "interpreter/library.h"
#include "interpreter/exception.h"

#ifdef ASAMPL_ENABLE_PYTHON

#include "library/python.cpp"

#endif

namespace Asampl::Interpreter::Library {

std::unique_ptr<ILibrary> open_library(const std::filesystem::path& path) {
#ifdef ASAMPL_ENABLE_PYTHON
        return std::make_unique<PythonLibrary>(path);
#endif
        throw InterpreterException("Could not open library");
}

}
