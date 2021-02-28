#include "interpreter/exception.h"
#include <interpreter/ffi_conversion.h>


AsaValueContainer convert_to_ffi(const ValuePtr& value) {
    throw InterpreterException("convert to ffi not implemented");
}

ValuePtr convert_from_ffi(const AsaValueContainer& value) {
    switch (value.type) {
        case ASA_DOUBLE: {
            const double val = reinterpret_cast<const AsaDouble*>(value.data)->value;
            return std::make_shared<Value<double>>(val);
        }

        case ASA_STRING: {
            const auto* string = reinterpret_cast<const AsaString*>(value.data);
            std::string string_value{string->data, string->size};
            return std::make_shared<Value<std::string>>(string_value);
        }

        case ASA_VIDEO_FRAME: {
            auto video_frame = reinterpret_cast<AsaVideoFrame*>(value.data);
            cv::Mat image(video_frame->height, video_frame->width, CV_8UC3, video_frame->data);
            AsaImage ret { image.clone() };
            return std::make_shared<Value<AsaImage>>(std::move(ret));
        }

        default:
            return std::make_shared<UndefinedValue>();
    }
}
