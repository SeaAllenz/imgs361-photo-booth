#include "photo_booth/ImageProcessing.hpp"

#include <opencv2/imgproc.hpp>
#include <stdexcept>
#include <string>

namespace photo_booth {

namespace {

void validateImage(const cv::Mat& image, const char* function_name) {
  if (image.empty()) {
    throw std::invalid_argument(std::string(function_name) +
                                ": input image is empty");
  }

  if (image.type() != CV_8UC3) {
    throw std::invalid_argument(std::string(function_name) +
                                ": input image must be CV_8UC3");
  }
}

}  // namespace

cv::Mat swapRedBlueChannels(const cv::Mat& image) {
  validateImage(image, "swapRedBlueChannels()");

  cv::Mat output;

  cv::cvtColor(image, output, cv::COLOR_BGR2RGB);

  return output;
}

cv::Mat invertImage(const cv::Mat& image) {
  validateImage(image, "invertImage()");

  cv::Mat output;

  cv::bitwise_not(image, output);

  return output;
}

}  // namespace photo_booth
