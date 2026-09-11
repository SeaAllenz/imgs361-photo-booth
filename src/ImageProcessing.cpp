#include "photo_booth/ImageProcessing.hpp"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <stdexcept>
#include <string>
#include <vector>
#include <tuple>
#include <cmath>

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

std::tuple<cv::Mat, cv::Mat> calcHist(const cv::Mat& image) {
  validateImage(image, "calcHist()");

  cv::Mat histogram = cv::Mat_<int>::zeros(3, 256);
  for (int row = 0; row < image.rows; ++row) {
    for (int column = 0; column < image.cols; ++column) {
      auto value = image.at<cv::Vec3b>(row, column);
      histogram.at<int>(0, value[0])++;
      histogram.at<int>(1, value[1])++;
      histogram.at<int>(2, value[2])++;
    }
  }

  return {histogram, image};
}

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

std::tuple<cv::Mat, cv::Mat> histogramEqualization(const cv::Mat& histogram, const cv::Mat& image) {
  int totalPixelCount = image.rows * image.cols;
  double blueCDF = 0.0, redCDF = 0.0, greenCDF = 0.0;
  std::vector<int> blueArray;
  std::vector<int> greenArray;
  std::vector<int> redArray;

  // Equalization equation for each color channel in a pixel -> intensity level
  for (int intensity = 0; intensity < 256; ++intensity) {
      double bluePixelCount = histogram.at<int>(0, intensity);
      double greenPixelCount = histogram.at<int>(1, intensity);
      double redPixelCount = histogram.at<int>(2, intensity);

      blueCDF += bluePixelCount / totalPixelCount;
      greenCDF += greenPixelCount / totalPixelCount;
      redCDF += redPixelCount / totalPixelCount;

      blueArray.push_back(std::round(255.0 * blueCDF));
      greenArray.push_back(std::round(255.0 * greenCDF));
      redArray.push_back(std::round(255.0 * redCDF));
  }

  // New intensities added to new histogram and image
  cv::Mat equalizedImage = image.clone();
  cv::Mat equalizedHistogram = cv::Mat::zeros(histogram.size(), histogram.type());

  for (int row = 0; row < image.rows; ++row) {
    for (int column = 0; column < image.cols; ++column) {
        auto& value = equalizedImage.at<cv::Vec3b>(row, column);

        value[0] = blueArray[value[0]];
        value[1] = greenArray[value[1]];
        value[2] = redArray[value[2]];

        equalizedHistogram.at<int>(0, value[0])++;
        equalizedHistogram.at<int>(1, value[1])++;
        equalizedHistogram.at<int>(2, value[2])++;
      }
  }

  return {equalizedHistogram, equalizedImage};
}

}  // namespace photo_booth
