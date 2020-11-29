#include "interpreter.h"
#include "interpreter/test_stdlib.h"

#include <opencv2/core/types.hpp>

namespace {
    AsaImage random_image(double width, double height) {
        cv::Mat img(static_cast<int>(width), static_cast<int>(height), CV_8UC3);
        cv::randu(img, cv::Scalar(0, 0, 0), cv::Scalar(255, 255, 255));
        return AsaImage { img };
    }

    void show_image(AsaImage image) {
        cv::imshow("", image.data);
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

        const int cols = mat.cols;
        const int rows = mat.rows;
        const int step = mat.channels();

        for (int y = 0; y < rows; y++) {
            uint8_t *p_row = mat.ptr(y) + static_cast<size_t>(channel);
            uint8_t *row_end = p_row + cols*step;
            for (; p_row != row_end; p_row += step) {
                *p_row *= value;
            }
        }
        // for(int y=0; y < rows; ++y) {
        //     for(int x=0; x < cols; ++x) {
        //         cv::Vec3b &color = mat.at<cv::Vec3b>(y,x);
        //         color[chnnl] += val;
        //     }
        // }
    }
}

std::vector<std::pair<std::string, Function>> get_stdlib_functions() {
    return {
        std::make_pair("random_image", make_asampl_function(random_image)),
        std::make_pair("show_image", make_asampl_function(show_image)),
        std::make_pair("dbg", make_asampl_function(dbg)),
        std::make_pair("is_undefined", make_asampl_function(is_undefined)),
        std::make_pair("change_color_channel", make_asampl_function(change_color_channel)),
        std::make_pair("change_color", make_asampl_function(change_color))
    };
}
