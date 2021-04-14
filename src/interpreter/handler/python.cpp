#include "interpreter/exception.h"
#include "interpreter/handler.h"
#include "interpreter/ffi_conversion.h"

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

namespace Asampl::Interpreter::Handler {

namespace py = boost::python;
namespace np = py::numpy;

class PythonHandler;

class PythonHandlerDownload : public IHandlerContextDownload {
public:
    PythonHandlerDownload(PythonHandler& handler);

    void push(const std::vector<uint8_t>& data) override;
    HandlerResponse download() override;

private:
    py::object context_;
};

class PythonHandler : public IHandler {
public:
    PythonHandler(std::filesystem::path path)
    {
        path.replace_extension("py");
        py::exec_file(path.c_str(), ns);
        handler_class = ns["Handler"];
    }

    std::unique_ptr<IHandlerContextDownload> open_download() override {
        return std::unique_ptr<PythonHandlerDownload>(new PythonHandlerDownload(*this));
    }

private:
    py::dict ns;
    py::object handler_class;

private:
    friend class PythonHandlerDownload;
};

PythonHandlerDownload::PythonHandlerDownload(PythonHandler& handler)
    : context_{handler.handler_class()}
{
}

void PythonHandlerDownload::push(const std::vector<uint8_t>& data) {
    auto ptr = reinterpret_cast<uint8_t*>(malloc(data.size()));
    std::copy(data.begin(), data.end(), ptr);

    auto array = np::from_data(
        ptr,
        np::dtype::get_builtin<uint8_t>(),
        py::make_tuple(data.size()), py::make_tuple(1),
        py::object());

    context_.attr("push")(array);
}

HandlerResponse PythonHandlerDownload::download() {
    py::object response = context_.attr("download")();
    if (response.contains("status")) {
        if (response["status"] == "fatal") {
            throw InterpreterException(py::extract<std::string>(response["error"]));
        }
        if (response["status"] == "not_ready") {
            return HandlerResponse::new_not_ready();
        }
        if (response["status"] == "out_of_data") {
            return HandlerResponse::new_out_of_data();
        }
        throw InterpreterException("Unknown response status");
    }

    const double timestamp = py::extract<double>(response["timestamp"]);
    auto value = convert_from_python(response["value"]);
    return HandlerResponse::new_ready(std::move(value), timestamp);
}

}
