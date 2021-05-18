#include <algorithm>

#include "interpreter.h"
#include "interpreter/test_stdlib.h"
#include "utils.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

namespace {

using namespace Asampl::Interpreter;

    //int convert_index(int index, size_t size) {
        //if (size == 0) {
            //throw InterpreterException("Cannot index empty array");
        //}

        //if (index < 0) {
            //index = static_cast<int>(size) + index + 1;
        //}

        //if (index < 0) {
            //throw InterpreterException("Invalid array index");
        //}

        //return index;
    //}
    
cv::Mat as_mat(Image& image) {
    return cv::Mat (static_cast<int>(image.height), static_cast<int>(image.width), CV_8UC3, image.data.data());
}

Image from_mat(const cv::Mat& mat) {
    std::vector<Byte> data(mat.datastart, mat.dataend);
    return Image{ static_cast<size_t>(mat.cols), static_cast<size_t>(mat.rows), std::move(data) };
}

cv::Size to_size(Map& map) {
    return {map.strict_get("x")->get<Number>().as_int(), map.strict_get("y")->get<Number>().as_int()};
}

cv::Point_<int> to_pos(Map& map) {
    return {map.strict_get("x")->get<Number>().as_int(), map.strict_get("y")->get<Number>().as_int()};
}

cv::Scalar to_color(Map& map) {
    return cv::Scalar(map.strict_get("r")->get<Number>().value, map.strict_get("g")->get<Number>().value, map.strict_get("b")->get<Number>().value, CV_8UC3);
}

}

namespace {

Image random_image(Number width, Number height) {
    const auto w = width.as_size();
    const auto h = width.as_size();

    cv::Mat img(width.as_int(), height.as_int(), CV_8UC3);
    cv::randu(img, cv::Scalar(0, 0, 0), cv::Scalar(255, 255, 255));

    std::vector<Byte> data;
    data.resize(w * h * img.channels());
    std::copy(const_cast<const uchar*>(img.data), img.dataend, data.data());

    return Image { w, h, data };
}

void show_image(Image& image) {
    cv::Mat bgr;
    cv::cvtColor(as_mat(image), bgr, cv::COLOR_RGB2BGR);
    cv::imshow("", bgr);
    cv::waitKey(43);
}

void i_overlay(Image& background, Image& image, Map& pos_) {
    auto pos = to_pos(pos_);
    std::cout << pos << std::endl;
    cv::Rect target_rect{pos.x, pos.y, static_cast<int>(image.width), static_cast<int>(image.height)};

    auto bg = as_mat(background);
    cv::Rect bg_rect{0, 0, bg.cols, bg.rows};
    auto img = as_mat(image);
    cv::Rect source_rect{0, 0, img.cols, img.rows};

    if (bg.empty() || img.empty()) {
        return;
    }

    if (target_rect.tl().x < 0) {
        target_rect.x = 0;
    }
    if (target_rect.tl().y < 0) {
        target_rect.y = 0;
    }
    if (target_rect.br().x > bg_rect.br().x) {
        const auto diff_x = target_rect.br().x - bg_rect.br().x;
        target_rect.width -= diff_x;
        source_rect.width = target_rect.width;
    }
    if (target_rect.br().y > bg_rect.br().y) {
        const auto diff_y = target_rect.br().y - bg_rect.br().y;
        target_rect.height -= diff_y;
        source_rect.height = target_rect.height;
    }

    img(source_rect).copyTo(bg(target_rect));
}

Image load_image(const String& path)
{
    auto bgr = cv::imread(path.value);
    cv::Mat rgb;
    cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);
    return from_mat(rgb);
}

Image scale_image(Image& image, Map& size, ArgsRest rest) {
    cv::InterpolationFlags inter = cv::INTER_LINEAR;
    if (rest.tuple.values.size() > 1) {
        const std::string& inter_str = rest.tuple[0]->get<String>().value;
        if (inter_str == "linear") {
            inter = cv::INTER_LINEAR;
        } else if (inter_str == "nearest") {
            inter = cv::INTER_NEAREST;
        } else {
            throw InterpreterException("Unknown interpolation parameter");
        }
    }

    cv::Mat result;
    cv::resize(as_mat(image), result, to_size(size), 0.0, 0.0, inter);
    return from_mat(result);
}

Image i_solid_color(Map& color, Map& size)
{
    const auto cv_size = to_size(size);
    cv::Mat image(cv_size, CV_8UC3, to_color(color));
    return from_mat(image);
}

ValuePtr dbg(const ValuePtr& value) {
    std::cout << "DEBUG: " << value->to_string() << std::endl;
    return value;
}

Bool is_undefined(const Value& value) {
    return Bool{value.is<Undefined>()};
}

Undefined undefined() {
    return Undefined{};
}

//void change_color(ValuePtr channel, ValuePtr value, AsaImage image) {

//}

//void change_color_channel(double channel, double value, AsaImage image) {
    //cv::Mat &mat = image.data;
    //if (mat.channels() < channel) {
        //throw InterpreterException("Invalid channel value");
    //}

    //cv::Scalar channel_scale(1.0, 1.0, 1.0);
    //channel_scale[channel] = value;

    //cv::multiply(mat, channel_scale, mat);
//}

Bool s_contains(const String& s, const String& sample) {
    return Bool{s.value.find(sample.value) != std::string::npos};
}

String s_concat(ArgsRest args) {
    String result{""};

    for (const auto& arg : args.tuple.values) {
        result.value += arg->get<String>().value;
    }

    return result;
}

Number i_width(const Image& img) {
    return Number{img.width};
}

Number i_height(const Image& img) {
    return Number{img.height};
}

Number t_len(const Tuple& t) {
    return Number{t.values.size()};
}

Tuple t_map(Tuple& in, Function& f) {
    Tuple result;
    result.values.reserve(in.values.size());

    for (auto& value : in.values) {
        result.values.push_back(f.func(Asampl::Utils::Slice{&value, 1}));
    }

    return result;
}

//std::vector<ValuePtr> array() {
    //return {};
//}

//void array_push(std::vector<ValuePtr>& array, const ValuePtr& value) {
    //array.push_back(value);
//}

//ValuePtr array_pop(std::vector<ValuePtr>& array) {
    //if (!array.empty()) {
        //auto value = std::move(array.back());
        //array.pop_back();
        //return value;
    //} else {
        //return std::make_shared<UndefinedValue>();
    //}
//}

//ValuePtr array_get(std::vector<ValuePtr>& array, int idx) {
    //if (idx >= 0 && idx < array.size()) {
        //return array[idx];
    //} else {
        //return std::make_shared<UndefinedValue>();
    //}
//}

//void array_set(std::vector<ValuePtr>& array, double idx_, const ValuePtr& value) {
    //int idx = convert_index(idx_, array.size());

    //while (array.size() <= idx) {
        //array.push_back(std::make_shared<UndefinedValue>());
    //}
    //array[idx] = value;
//}

//double array_size(const std::vector<ValuePtr>& array) {
    //return array.size();
//}

//std::vector<ValuePtr> array_slice(const std::vector<ValuePtr>& array, double from_, double to_) {
    //int from = convert_index(from_, array.size());
    //int to = convert_index(to_, array.size());
    //std::vector<ValuePtr> result;

    //for (int i = from; i < to && i < array.size(); i++) {
        //result.push_back(array[i]);
    //}

    //return result;
//}

//std::vector<ValuePtr> array_concat(const std::vector<ValuePtr>& fst_, const std::vector<ValuePtr>& snd) {
    //auto fst = fst_;

    //for (const auto& value : snd) {
        //fst.push_back(value);
    //}

    //return fst;
//}
}

namespace Asampl::Interpreter {

std::vector<Function> get_stdlib_functions() {
    return {
        MAKE_ASAMPL_FUNCTION(dbg),
        MAKE_ASAMPL_FUNCTION(is_undefined),
        MAKE_ASAMPL_FUNCTION(undefined),
        MAKE_ASAMPL_FUNCTION(s_contains),
        MAKE_ASAMPL_FUNCTION(s_concat),
        MAKE_ASAMPL_FUNCTION(random_image),
        MAKE_ASAMPL_FUNCTION(load_image),
        MAKE_ASAMPL_FUNCTION(show_image),
        MAKE_ASAMPL_FUNCTION(i_overlay),
        MAKE_ASAMPL_FUNCTION(scale_image),
        MAKE_ASAMPL_FUNCTION(i_solid_color),
        MAKE_ASAMPL_FUNCTION(i_width),
        MAKE_ASAMPL_FUNCTION(i_height),
        MAKE_ASAMPL_FUNCTION(t_map),
        MAKE_ASAMPL_FUNCTION(t_len),
    };
}

}
