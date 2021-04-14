#include <functional>

#include <boost/python.hpp>

#include "interpreter/library.h"
#include "interpreter/function.h"
#include "interpreter/ffi_conversion.h"

namespace Asampl::Interpreter::Library {

namespace py = boost::python;

ValuePtr python_function_call(py::object func, Utils::Slice<ValuePtr> args) {
    py::list py_args;
    for (const auto& arg : args) {
        py_args.append(convert_to_python(arg));
    }

    return convert_from_python(func(*py::tuple(py_args)));
}

class PythonLibrary : public ILibrary {
public:
    PythonLibrary(std::filesystem::path path) {
        path.replace_extension("py");
        py::exec_file(path.c_str(), ns);
    }

    virtual std::vector<Function> get_functions() override {
        std::vector<Function> functions;

        auto keys = ns.keys();
        const auto len = py::len(keys);
        functions.reserve(len);

        for (std::size_t i = 0; i < len; i++) {
            auto key = py::extract<std::string>(keys[i])();
            if (key.find("asampl_") == 0) {
                const std::string function_name = key.substr(strlen("asampl_"));
                functions.emplace_back(Function{ function_name, std::bind(python_function_call, ns[key], std::placeholders::_1) });
            }
        }

        return functions;
    }

private:
    py::dict ns;
};

}
