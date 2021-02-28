#include "interpreter.h"
#include "interpreter/test_stdlib.h"

#include <opencv2/core/types.hpp>

namespace {
    int convert_index(int index, size_t size) {
        if (size == 0) {
            throw InterpreterException("Cannot index empty array");
        }

        if (index < 0) {
            index = static_cast<int>(size) + index + 1;
        }

        if (index < 0) {
            throw InterpreterException("Invalid array index");
        }

        return index;
    }
}

namespace {
    AsaImage random_image(double width, double height) {
        cv::Mat img(static_cast<int>(width), static_cast<int>(height), CV_8UC3);
        cv::randu(img, cv::Scalar(0, 0, 0), cv::Scalar(255, 255, 255));
        return AsaImage { img };
    }

    void show_image(AsaImage image) {
        cv::Mat bgr;

        cv::cvtColor(image.data, bgr, cv::COLOR_RGB2BGR);
        cv::imshow("", bgr);
        cv::waitKey(43);
    }

    void dbg(ValuePtr value) {
        std::cout << "DEBUG: " << value->to_string() << std::endl;
    }

    bool is_undefined(ValuePtr value) {
        return value->get_type() == ValueType::UNDEFINED;
    }

    void change_color(ValuePtr channel, ValuePtr value, AsaImage image) {

    }

    void change_color_channel(double channel, double value, AsaImage image) {
        cv::Mat &mat = image.data;
        if (mat.channels() < channel) {
            throw InterpreterException("Invalid channel value");
        }

        cv::Scalar channel_scale(1.0, 1.0, 1.0);
        channel_scale[channel] = value;

        cv::multiply(mat, channel_scale, mat);
    }

    bool string_contains(const std::string& s, const std::string& sample) {
        return s.find(sample) != std::string::npos;
    }

    double width(const AsaImage& img) {
        return img.data.cols;
    }

    double height(const AsaImage& img) {
        return img.data.rows;
    }

    AsaImage overlay_image(AsaImage& background, const AsaImage& img, double x, double y) {
        img.data.copyTo(background.data(cv::Rect(x, y, img.data.cols, img.data.rows)));
        return background;
    }

    AsaImage load_image(const std::string& path)
    {
        return { cv::imread(path) };
    }

    std::vector<ValuePtr> array() {
        return {};
    }

    void array_push(std::vector<ValuePtr>& array, const ValuePtr& value) {
        array.push_back(value);
    }

    ValuePtr array_pop(std::vector<ValuePtr>& array) {
        if (!array.empty()) {
            auto value = std::move(array.back());
            array.pop_back();
            return value;
        } else {
            return std::make_shared<UndefinedValue>();
        }
    }

    ValuePtr array_get(std::vector<ValuePtr>& array, int idx) {
        if (idx >= 0 && idx < array.size()) {
            return array[idx];
        } else {
            return std::make_shared<UndefinedValue>();
        }
    }

    void array_set(std::vector<ValuePtr>& array, double idx_, const ValuePtr& value) {
        int idx = convert_index(idx_, array.size());

        while (array.size() <= idx) {
            array.push_back(std::make_shared<UndefinedValue>());
        }
        array[idx] = value;
    }

    double array_size(const std::vector<ValuePtr>& array) {
        return array.size();
    }

    std::vector<ValuePtr> array_slice(const std::vector<ValuePtr>& array, double from_, double to_) {
        int from = convert_index(from_, array.size());
        int to = convert_index(to_, array.size());
        std::vector<ValuePtr> result;

        for (int i = from; i < to && i < array.size(); i++) {
            result.push_back(array[i]);
        }

        return result;
    }

    std::vector<ValuePtr> array_concat(const std::vector<ValuePtr>& fst_, const std::vector<ValuePtr>& snd) {
        auto fst = fst_;

        for (const auto& value : snd) {
            fst.push_back(value);
        }

        return fst;
    }
}

std::vector<std::pair<std::string, Function>> get_stdlib_functions() {
    return {
        std::make_pair("random_image", make_asampl_function(random_image)),
        std::make_pair("show_image", make_asampl_function(show_image)),
        std::make_pair("dbg", make_asampl_function(dbg)),
        std::make_pair("is_undefined", make_asampl_function(is_undefined)),
        std::make_pair("change_color_channel", make_asampl_function(change_color_channel)),
        std::make_pair("change_color", make_asampl_function(change_color)),
        std::make_pair("string_contains", make_asampl_function(string_contains)),
        std::make_pair("width", make_asampl_function(width)),
        std::make_pair("height", make_asampl_function(height)),
        std::make_pair("overlay_image", make_asampl_function(overlay_image)),
        std::make_pair("load_image", make_asampl_function(load_image)),
        std::make_pair("array", make_asampl_function(array)),
        std::make_pair("array_push", make_asampl_function(array_push)),
        std::make_pair("array_pop", make_asampl_function(array_pop)),
        std::make_pair("array_get", make_asampl_function(array_get)),
        std::make_pair("array_set", make_asampl_function(array_set)),
        std::make_pair("array_size", make_asampl_function(array_size)),
        std::make_pair("array_slice", make_asampl_function(array_slice)),
        std::make_pair("array_concat", make_asampl_function(array_concat)),
    };
}
