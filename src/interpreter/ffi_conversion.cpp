#include <cmath>

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

#ifdef ASAMPL_ENABLE_PYTHON

#include <boost/python/numpy.hpp>

namespace py = boost::python;
namespace np = py::numpy;

np::ndarray mat_to_ndarray(const cv::Mat& mat) {
    py::tuple shape = py::make_tuple(mat.rows, mat.cols, mat.channels());
    py::tuple stride = py::make_tuple(mat.channels() * mat.cols * sizeof(uchar), mat.channels() * sizeof(uchar), sizeof(uchar));
    np::dtype dt = np::dtype::get_builtin<uchar>();
    np::ndarray array = np::from_data(mat.data, dt, shape, stride, py::object());

    return array;
}

py::object convert_to_python(const ValuePtr& value) {
    switch (value->get_type()) {
        case ValueType::NUMBER: {
            return py::object(value->try_get<double>());
        }

        case ValueType::STRING: {
            return py::object(value->try_get<std::string>());
        }

        case ValueType::IMAGE: {
            return mat_to_ndarray(value->try_get<AsaImage>().data);
        }

        default:
            throw InterpreterException("Could not covert type to python");
    }
}

ValuePtr convert_from_python(py::object value) {
    if (auto extract = py::extract<double>(value); extract.check()) {
        return std::make_shared<Value<double>>(extract());
    }
    if (auto extract = py::extract<std::string>(value); extract.check()) {
        return std::make_shared<Value<std::string>>(extract());
    }
    if (auto extract = py::extract<np::ndarray>(value); extract.check()) {
        auto array = extract().astype(np::dtype::get_builtin<uint8_t>());
        if (array.get_nd() == 3) {
            int sizes[2] = { static_cast<int>(array.shape(0)), static_cast<int>(array.shape(1)) };
            size_t steps[2] = { static_cast<size_t>(array.strides(0)), static_cast<size_t>(array.strides(1)) };

            cv::Mat image(2, sizes, CV_8UC3, array.get_data(), steps);

            AsaImage ret { image.clone() };
            return std::make_shared<Value<AsaImage>>(std::move(ret));
        } else if (array.get_nd() == 1) {
            throw InterpreterException("Audio is not supported yet");
        } else {
            throw InterpreterException("Invalid number of dimensions");
        }
    }

    return std::make_shared<UndefinedValue>();
}

#endif
