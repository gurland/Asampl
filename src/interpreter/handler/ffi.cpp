#include "interpreter/exception.h"
#include "interpreter/handler.h"
#include "interpreter/ffi_conversion.h"

#include <dynalo/dynalo.hpp>

extern "C" {
#include <asampl-ffi/ffi.h>
}

namespace {

std::string to_native_name(const std::filesystem::path& path) {
    const auto filename = dynalo::to_native_name(path.filename().string());

    return (path.parent_path() / filename).string();
}

template< typename T >
void load_function(dynalo::library& lib, std::function<T>& func, const char* name) {
    func = lib.get_function<T>(name);
}

}

class FFIHandler;

class FFIContextDownload : public IHandlerContextDownload {
public:
    FFIContextDownload(FFIHandler& handler);

    void push(const std::vector<uint8_t>& data) override;
    HandlerDownloadResponse download() override;

private:
    FFIHandler& handler_;
    AsaHandler* context_;
};

class FFIHandler : public IHandler {
public:
    FFIHandler(std::filesystem::path path)
        : library_(to_native_name(path))
    {
        load_function(library_, open_download_, "asa_handler_open_download");
        load_function(library_, open_upload_, "asa_handler_open_upload");
        load_function(library_, close_, "asa_handler_close");
        load_function(library_, push_, "asa_handler_push");
        load_function(library_, download_, "asa_handler_download");
        load_function(library_, upload_, "asa_handler_upload");
    }

    std::unique_ptr<IHandlerContextDownload> open_download() override {
        return std::unique_ptr<FFIContextDownload>(new FFIContextDownload(*this));
    }

    std::unique_ptr<IHandlerContextUpload> open_upload() override {
        throw InterpreterException("FFI upload is not supported yet");
    }

private:
    dynalo::library library_;

    std::function<AsaHandler*()> open_download_;
    std::function<AsaHandler*()> open_upload_;
    std::function<void(AsaHandler*)> close_;

    std::function<int(AsaHandler*, const AsaBytes*)> push_;

    std::function<AsaHandlerResponse(AsaHandler*)> download_;
    std::function<AsaHandlerResponse(AsaHandler*)> upload_;

private:
    friend class FFIContextDownload;
};

FFIContextDownload::FFIContextDownload(FFIHandler& handler)
    : handler_{handler}
    , context_{handler.open_download_()}
{
    if (context_ == nullptr) {
        throw InterpreterException("Coult not open handler for download");
    }
}

void FFIContextDownload::push(const std::vector<uint8_t>& data) {
    AsaBytes bytes;
    bytes.size = data.size();
    bytes.data = const_cast<uint8_t*>(data.data());

    handler_.push_(context_, &bytes);
}

HandlerDownloadResponse FFIContextDownload::download() {
    AsaHandlerResponse response = handler_.download_(context_);
    switch (response.status) {
        case ASA_STATUS_FATAL: {
            std::string msg = response.error;
            asa_deinit_response(&response);
            throw InterpreterException(msg);
        }
        case ASA_STATUS_NOT_READY:
            asa_deinit_response(&response);
            return HandlerDownloadResponse::new_not_ready();
        case ASA_STATUS_EOI:
            asa_deinit_response(&response);
            return HandlerDownloadResponse::new_out_of_data();
        default: {
            const double timestamp = response.value->timestamp;
            auto value = convert_from_ffi(*response.value);
            asa_deinit_container(response.value);
            asa_free(response.value);
            return HandlerDownloadResponse::new_valid(std::move(value), timestamp);
        }
    }
}
