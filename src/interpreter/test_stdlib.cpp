#include "interpreter.h"
#include "interpreter/test_stdlib.h"

namespace {

AsaImage random_image(double width, double height) {
    cv::Mat img(static_cast<int>(width), static_cast<int>(height), CV_8UC3);
    cv::randu(img, cv::Scalar(0, 0, 0), cv::Scalar(255, 255, 255));
    return AsaImage { img };
}

void show_image(AsaImage image) {
    cv::imshow("", image.data);
    cv::waitKey(10);
}

void dbg(ValuePtr value) {
    std::cout << "DEBUG: " << value->to_string() << std::endl;
}

}

std::vector<std::pair<std::string, Function>> get_stdlib_functions() {
    std::function<AsaImage(double, double)> random_image_fn = random_image;
    std::function<void(AsaImage)> show_image_fn = show_image;
    std::function<void(ValuePtr)> dbg_fn = dbg;

    return {
        std::make_pair("random_image", make_asampl_function(random_image_fn)),
        std::make_pair("show_image", make_asampl_function(show_image_fn)),
        std::make_pair("dbg", make_asampl_function(dbg_fn))
    };
}
