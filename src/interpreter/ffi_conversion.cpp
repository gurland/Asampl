#include <cmath>

#include "interpreter/exception.h"
#include <interpreter/ffi_conversion.h>

namespace Asampl::Interpreter {

AsaValueContainer convert_to_ffi(const ValuePtr& value) {
    throw InterpreterException("convert to ffi not implemented");
}

ValuePtr convert_from_ffi(const AsaValueContainer& value) {
    switch (value.type) {
        case ASA_DOUBLE: {
            const double val = reinterpret_cast<const AsaDouble*>(value.data)->value;
            return Number{val};
        }

        case ASA_STRING: {
            const auto* string = reinterpret_cast<const AsaString*>(value.data);
            std::string string_value{string->data, string->size};
            return String{string_value};
        }

        case ASA_VIDEO_FRAME: {
            auto video_frame = reinterpret_cast<AsaVideoFrame*>(value.data);
            std::vector<Byte> data;
            data.resize(video_frame->width * video_frame->height * 3u);
            std::copy(video_frame->data, video_frame->data + data.size(), data.data());
            return Image { video_frame->width, video_frame->height, std::move(data) };
        }

        default:
            return Undefined{};
    }
}

}

#ifdef ASAMPL_ENABLE_PYTHON

#include <boost/python/numpy.hpp>

namespace Asampl::Interpreter {

namespace py = boost::python;
namespace np = py::numpy;

struct PythonVisitor : ValueVisitor<py::object> {
    const ValuePtr& ptr;

    PythonVisitor(const ValuePtr& ptr)
        : ValueVisitor<py::object>()
        , ptr{ptr}
    {
    }

    Result operator()(Undefined&) override {
        return py::object{};
    }

    Result operator()(Number& x) override {
        return py::object(x.value);
    }

    Result operator()(Bool& x) override {
        return py::object(x.value);
    }

    Result operator()(String& x) override {
        return py::str(x.value);
    }

    Result operator()(Image& x) override {
        py::tuple shape = py::make_tuple(x.height, x.width, x.channels);
        py::tuple stride = py::make_tuple(x.channels * x.width, x.channels, 1);
        np::dtype dt = np::dtype::get_builtin<uint8_t>();
        np::ndarray array = np::from_data(x.data.data(), dt, shape, stride, py::object(ptr.ptr));

        return std::move(array);
    }

    Result operator()(Tuple& x) override {
        py::list list;
        for (auto& value : x.values) {
            PythonVisitor visitor{value};
            list.append(value->apply(visitor));
        }
        return std::move(list);
    }

    Result operator()(Map&) override {
        throw InterpreterException("Maps not implemented");
    }

    Result operator()(ByteArray& x) override {
        py::tuple shape = py::make_tuple(x.data.size());
        py::tuple stride = py::make_tuple(1);
        np::dtype dt = np::dtype::get_builtin<uint8_t>();
        np::ndarray array = np::from_data(x.data.data(), dt, shape, stride, py::object(ptr.ptr));

        return std::move(array);
    }

    Result operator()(Function& x) override {
        throw InterpreterException("Functions not implemented");
    }
};

py::object convert_to_python(const ValuePtr& value) {
    PythonVisitor visitor{value};
    return value->apply(visitor);
}

ValuePtr convert_from_python(py::object value) {
    if (auto extract = py::extract<double>(value); extract.check()) {
        return Number{extract()};
    }
    if (auto extract = py::extract<std::string>(value); extract.check()) {
        return String{extract()};
    }
    if (auto extract = py::extract<np::ndarray>(value); extract.check()) {
        auto array = extract().astype(np::dtype::get_builtin<uint8_t>());
        if (array.get_nd() == 3) {
            size_t sizes[2] = { static_cast<size_t>(array.shape(0)), static_cast<size_t>(array.shape(1)) };

            std::vector<Byte> data;
            data.resize(sizes[0] * sizes[1] * 3);
            std::copy(array.get_data(), array.get_data() + data.size(), data.data());
            return Image{ sizes[0], sizes[1], std::move(data) };
        } else if (array.get_nd() == 1) {
            size_t size = array.shape(0);

            std::vector<Byte> data;
            data.resize(size);
            std::copy(array.get_data(), array.get_data() + size, data.data());
            return ByteArray{ std::move(data) };
        } else {
            throw InterpreterException("Invalid number of dimensions");
        }
    }

    return Undefined{};
}


}

#endif

void Asampl::Interpreter::init_conversions() {
#ifdef ASAMPL_ENABLE_PYTHON

    py::class_<Value, py::bases<>, std::shared_ptr<Value>, boost::noncopyable>("AsaValue", py::no_init);

#endif
}
