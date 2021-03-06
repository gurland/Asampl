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
    DownloadResponse download() override;

private:
    py::object context_;
};

class PythonHandlerUpload : public IHandlerContextUpload {
public:
    PythonHandlerUpload(PythonHandler& handler);

    UploadResponse pull() override;
    void upload(const ValuePtr& value) override;

private:
    py::object context_;
};

class PythonHandler : public IHandler {
public:
    PythonHandler(std::filesystem::path path)
    {
        path.replace_extension("py");
        py::exec_file(path.c_str(), ns);
        download_class = ns["Download"];
        upload_class = ns["Upload"];
    }

    std::unique_ptr<IHandlerContextDownload> open_download() override {
        return std::unique_ptr<PythonHandlerDownload>(new PythonHandlerDownload(*this));
    }

    std::unique_ptr<IHandlerContextUpload> open_upload() override {
        throw InterpreterException("FFI upload is not supported yet");
    }

private:
    py::dict ns;
    py::object download_class;
    py::object upload_class;

private:
    friend class PythonHandlerDownload;
    friend class PythonHandlerUpload;
};


PythonHandlerDownload::PythonHandlerDownload(PythonHandler& handler)
    : context_{handler.download_class()}
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

DownloadResponse PythonHandlerDownload::download() {
    py::object response = context_.attr("download")();
    if (response.contains("status")) {
        if (response["status"] == "fatal") {
            throw InterpreterException(py::extract<std::string>(response["error"]));
        }
        if (response["status"] == "not_ready") {
            return DownloadResponse::new_not_ready();
        }
        if (response["status"] == "out_of_data") {
            return DownloadResponse::new_out_of_data();
        }
        throw InterpreterException("Unknown response status");
    }

    const double timestamp = py::extract<double>(response["timestamp"]);
    auto value = convert_from_python(response["value"]);
    return DownloadResponse::new_valid(std::move(value), timestamp);
}


PythonHandlerUpload::PythonHandlerUpload(PythonHandler& handler)
    : context_(handler.upload_class())
{
}

UploadResponse PythonHandlerUpload::pull() {
    py::object response = context_.attr("pull")();
    if (auto extract = py::extract<np::ndarray>(response); extract.check()) {
        np::ndarray array = extract();
        if (array.get_nd() != 1) {
            throw InterpreterException("Wrong number of dimensions. Expected 1");
        }

        std::vector<uint8_t> data;
        data.resize(array.shape(0));
        std::copy(array.get_data(), array.get_data() + array.shape(0), data.data());

        return UploadResponse::new_valid(std::move(data));
    } else if (response.is_none()) {
        return UploadResponse::new_not_ready();
    } else {
        throw InterpreterException("Unexpected return value from handler");
    }
}

void PythonHandlerUpload::upload(const ValuePtr& value) {
    context_.attr("upload")(convert_to_python(value));
}

}
