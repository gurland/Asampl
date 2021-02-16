#include "interpreter/handler.h"
#include "interpreter/ffi_conversion.h"

#include <filesystem>
#include <vector>

namespace {

std::string to_native_name(const std::string& path) {
    const auto fs_path = std::filesystem::path(path);
    const auto filename = dynalo::to_native_name(fs_path.filename().string());

    return (fs_path.parent_path() / filename).string();
}

template< typename T >
void load_function(dynalo::library& lib, std::function<T>& func, const char* name) {
    func = lib.get_function<T>(name);
}

}

Handler::Handler(const std::string& path)
    : library_(to_native_name(path))
{
    load_function(library_, open_download_, "asa_handler_open_download");
    load_function(library_, open_upload_, "asa_handler_open_upload");
    load_function(library_, close_, "asa_handler_close");
    load_function(library_, push_, "asa_handler_push");
    load_function(library_, download_, "asa_handler_download");
    load_function(library_, upload_, "asa_handler_upload");
}

std::unique_ptr<HandlerContextDownload> Handler::open_download()
{
    return std::unique_ptr<HandlerContextDownload>(new HandlerContextDownload(this));
}


HandlerContextDownload::HandlerContextDownload(Handler* handler)
    : handler_(handler)
    , context_(handler->open_download_())
{
    if (context_ == nullptr) {
        throw InterpreterException("Could not open handler for download");
    }
}

void HandlerContextDownload::push(const std::vector<uint8_t>& data) {
    AsaBytes bytes;
    bytes.size = data.size();
    bytes.data = const_cast<uint8_t*>(data.data());

    handler_->push_(context_, &bytes);
}

AsaHandlerResponse HandlerContextDownload::download() {
    return handler_->download_(context_);
}

HandlerContextDownload::~HandlerContextDownload() {
    handler_->close_(context_);
}


ActiveDownload::ActiveDownload(const std::string& filename, Handler* handler)
    : stream_in{filename, std::ifstream::binary}
{
    if (!stream_in.is_open()) {
        throw InterpreterException("Could not open '" + filename + "'");
    }

    handler_ctx = handler->open_download();
}

bool ActiveDownload::fill_data() {
    if (stream_in.eof() || stream_in.peek() == EOF) {
        return false;
    }

    std::vector<uint8_t> data{std::istreambuf_iterator<char>{stream_in}, {}};
    handler_ctx->push(data);
    return true;
}

ValuePtr ActiveDownload::download_frame_val() {
    auto frame = download_frame();
    if (frame)
    {
        auto value = convert_from_ffi(*frame);
        asa_deinit_container(frame);
        asa_free(frame);
        return value;
    }
    else
    {
        return std::make_shared<UndefinedValue>();
    }
}

AsaValueContainer* ActiveDownload::download_frame() {
    fill_data();

    while (true) {
        auto response = handler_ctx->download();
        switch (response.status) {
            case ASA_STATUS_FATAL: {
                std::string msg = response.error;
                asa_deinit_response(&response);
                throw InterpreterException(msg);
            }
            case ASA_STATUS_NOT_READY:
            case ASA_STATUS_EOI:
                asa_deinit_response(&response);
                if (fill_data()) {
                    continue;
                } else {
                    return nullptr;
                }
            default: {
                return response.value;
            }
        }
    }
}
