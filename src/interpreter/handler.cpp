#include "interpreter/handler.h"
#include "interpreter/exception.h"

#include "handler/ffi.cpp"
#ifdef ASAMPL_ENABLE_PYTHON
#include "handler/python.cpp"
#endif

ActiveDownload::ActiveDownload(const std::filesystem::path& filename, IHandler& handler) 
    : stream_in{filename, std::ifstream::binary}
{
    if (!stream_in.is_open()) {
        throw InterpreterException("Coult not open '" + filename.string() + "'");
    }

    context = handler.open_download();
}

bool ActiveDownload::fill_data() {
    if (stream_in.eof() || stream_in.peek() == EOF) {
        return false;
    }

    std::vector<uint8_t> data{std::istreambuf_iterator<char>{stream_in}, {}};
    context->push(data);
    return true;
}

HandlerDownloadResponse ActiveDownload::download() {
    fill_data();

    while (true) {
        auto response = context->download();
        if (!response.enough_data) {
            if (fill_data()) {
                continue;
            } else {
                return HandlerDownloadResponse::new_out_of_data();
            }
        } else {
            return response;
        }
    }
}

std::unique_ptr<IHandler> open_handler(std::filesystem::path path) {
#ifdef ASAMPL_ENABLE_PYTHON
    try {
        return std::make_unique<PythonHandler>(path);
    } catch (const py::error_already_set&) {}
#endif

    try {
        return std::make_unique<FFIHandler>(path);
    }  catch (...) {}

    throw InterpreterException("Coult not open handler");
}
