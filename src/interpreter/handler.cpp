#include "interpreter/handler.h"

#include <filesystem>

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
    load_function(library_, open_download, "asa_handler_open_download");
    load_function(library_, open_upload, "asa_handler_open_upload");
    load_function(library_, close, "asa_handler_close");
    load_function(library_, push, "asa_handler_push");
    load_function(library_, get_type, "asa_handler_get_type");
    load_function(library_, download, "asa_handler_download");
    load_function(library_, upload, "asa_handler_upload");
    load_function(library_, free, "asa_handler_free");
}


ActiveDownload::ActiveDownload(const std::string& filename, Handler* handler)
    : stream_in{filename, std::ifstream::binary}
{
    if (!stream_in.is_open()) {
        throw InterpreterException("Could not open '" + filename + "'");
    }

    this->handler = handler;

    handler_ctx = handler->open_download();
    if (handler_ctx == nullptr) {
        throw InterpreterException("Could not open download");
    }
}

ActiveDownload::~ActiveDownload() {
    if (handler != nullptr && handler_ctx != nullptr) {
        handler->close(handler_ctx);
        handler = nullptr;
        handler_ctx = nullptr;
    }
}

ActiveDownload::ActiveDownload(ActiveDownload&& other) {
    stream_in = std::move(other.stream_in);
    handler = other.handler;
    handler_ctx = other.handler_ctx;

    other.handler = nullptr;
    other.handler_ctx = nullptr;
}

bool ActiveDownload::fill_data() {
    if (stream_in.eof() || stream_in.peek() == EOF) {
        return false;
    }

    std::vector<uint8_t> data{std::istreambuf_iterator<char>{stream_in}, {}};
    AsaData packet;
    packet.size = data.size();
    packet.data = data.data();
    handler->push(handler_ctx, &packet);
    return true;
}

ValuePtr ActiveDownload::download_frame() {
    fill_data();

    while (true) {
        auto frame = handler->download(handler_ctx);
        switch (frame->status) {
            case ASA_STATUS_FATAL: {
                std::string msg = frame->error;
                handler->free(handler_ctx, frame);
                throw InterpreterException(msg);
            }
            case ASA_STATUS_AGAIN:
            case ASA_STATUS_EOI:
                handler->free(handler_ctx, frame);
                if (fill_data()) {
                    continue;
                } else {
                    return std::make_shared<UndefinedValue>();
                }
            default: {
                auto ret = frame_to_value(frame);
                handler->free(handler_ctx, frame);
                return ret;
            }
        }
    }
}

ValuePtr ActiveDownload::frame_to_value(const AsaData* frame) {
    switch (handler->get_type(handler_ctx)) {
        case ASA_VIDEO: {
            auto video_frame = reinterpret_cast<AsaVideoData*>(frame->data);
            cv::Mat image(video_frame->height, video_frame->width, CV_8UC3, video_frame->frame);
            AsaImage ret { image.clone() };
            return std::make_shared<Value<AsaImage>>(std::move(ret));
        }

        case ASA_NUMBER: {
            const double value = *reinterpret_cast<double*>(frame->data);
            return std::make_shared<Value<double>>(value);
        }

        default:
            throw InterpreterException("Handler type not supported");
    }
}
