#include "photo_booth/ImageProcessing.hpp"

#include <opencv2/imgproc.hpp>
#include <stdexcept>
#include <string>
#include <cmath>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

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

cv::Mat calcHist(const cv::Mat& image) {
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

  return histogram;
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

cv::Mat quantization(const cv::Mat& image) {
  validateImage(image, "quantization()");

  cv::Mat quantizedImage = image.clone();

  for (int row = 0; row < image.rows; ++row) {
    for (int column = 0; column < image.cols; ++column) {
      auto& value = quantizedImage.at<cv::Vec3b>(row, column);

      auto levelB = value[0] / 32;
      auto levelG = value[1] / 32;
      auto levelR = value[2] / 32;

      value[0] = (levelB * 32) + 16;
      value[1] = (levelG * 32) + 16;
      value[2] = (levelR * 32) + 16;
    }
  }

  return quantizedImage;
}

cv::Mat histogramEqualization(const cv::Mat& image) {
    validateImage(image, "histogramEqualization()");

    const int totalPixelCount = image.rows * image.cols;

    cv::Mat histogram = calcHist(image);

    auto [blueArray, greenArray, redArray] =
        getRgbChannelPixels(histogram, totalPixelCount);

    cv::Mat equalizedImage = image.clone();

    for (int row = 0; row < image.rows; ++row) {
        for (int column = 0; column < image.cols; ++column) {
            auto& value = equalizedImage.at<cv::Vec3b>(row, column);

            value[0] = blueArray[value[0]];
            value[1] = greenArray[value[1]];
            value[2] = redArray[value[2]];
        }
    }

    return equalizedImage;
}

std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> getRgbChannelPixels(const cv::Mat& histogram, const int totalPixelCount ){

    double blueCDF = 0.0, redCDF = 0.0, greenCDF = 0.0;
    std::vector<int> blueArray;
    std::vector<int> greenArray;
    std::vector<int> redArray;

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

    return {blueArray, greenArray, redArray};
}

cv::Mat histogramMatching(const cv::Mat& image, const cv::Mat& newImage){
  int sourcePixelCount = image.rows * image.cols;
  int newsourcePixelCount = newImage.rows * newImage.cols;

  cv::Mat histogram = calcHist(image);
  cv::Mat newHistogram = calcHist(newImage);;

  auto [blueCDFArray, greenCDFArray, redCDFArray] = getRgbChannelCDF(histogram, sourcePixelCount);
  auto[newblueArray, newgreenArray, newredArray] = getRgbChannelCDF(newHistogram, newsourcePixelCount);

  std::vector<int> blueMatchingIntensity(256);
  std::vector<int> greenMatchingIntensity(256);
  std::vector<int> redMatchingIntensity(256);

  //RBG -> Intensity -> CDF Matching -> Inserting Intensity into new (Matching) Array

  //Blue
  for (int intensity = 0; intensity < 256; ++intensity){
    double CDF = blueCDFArray.at(intensity);
    for (int newIntensity=0; newIntensity < 256; ++newIntensity){
      double newCDF = newblueArray.at(newIntensity);
      if (newCDF >= CDF){
        double lowerCDF;
        if (newIntensity == 0){ //Out of bounds edge
          blueMatchingIntensity.at(intensity) = 0;
          break;
        }else{
          lowerCDF = CDF - (newblueArray.at(newIntensity - 1));
        }
        double higherCDF = (newblueArray.at(newIntensity)) - CDF;

        //Ternary opertation to see what's close to the target CDF
        blueMatchingIntensity.at(intensity) = (lowerCDF < higherCDF) ? newIntensity - 1 : newIntensity;

        break;
      }
    }
  }

  //Green
  for (int intensity = 0; intensity < 256; ++intensity){
    double CDF = greenCDFArray.at(intensity);
    for (int newIntensity=0; newIntensity < 256; ++newIntensity){
      double newCDF = newgreenArray.at(newIntensity);
      if (newCDF >= CDF){
        double lowerCDF;
        if (newIntensity == 0){ //Out of bounds edge
          greenMatchingIntensity.at(intensity) = 0;
          break;
        }else{
          lowerCDF = CDF - (newgreenArray.at(newIntensity - 1));
        }
        double higherCDF = (newgreenArray.at(newIntensity)) - CDF;

        greenMatchingIntensity.at(intensity) = (lowerCDF < higherCDF) ? newIntensity - 1 : newIntensity;

        break;
      }
    }
  }

  //Red
  for (int intensity = 0; intensity < 256; ++intensity){
    double CDF = redCDFArray.at(intensity);
    for (int newIntensity=0; newIntensity < 256; ++newIntensity){
      double newCDF = newredArray.at(newIntensity);
      if (newCDF >= CDF){
        double lowerCDF;
        if (newIntensity == 0){ //Out of bounds edge
          redMatchingIntensity.at(intensity) = 0;
          break;
        }else{
          lowerCDF = CDF - (newredArray.at(newIntensity - 1));
        }
        double higherCDF = (newredArray.at(newIntensity)) - CDF;

        redMatchingIntensity.at(intensity) = (lowerCDF < higherCDF) ? newIntensity - 1 : newIntensity;

        break;
      }
    }
  }
  
  cv::Mat matchingImage = image.clone();

  for (int row = 0; row < image.rows; ++row) {
    for (int column = 0; column < image.cols; ++column) {
      auto& pixel = matchingImage.at<cv::Vec3b>(row, column);
      
      //Blue
      int oldBlue = pixel[0];
      int newBlue = blueMatchingIntensity[oldBlue];
      pixel[0] = newBlue;

      //Green
      int oldGreen = pixel[1];
      int newGreen = greenMatchingIntensity[oldGreen];
      pixel[1] = newGreen;

      //Red 
      int oldRed = pixel[2];
      int newRed = redMatchingIntensity[oldRed];
      pixel[2] = newRed;
    }
  }

  return matchingImage;
}

std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> getRgbChannelCDF(const cv::Mat& histogram, const int totalPixelCount ){
  
  double blueCDF = 0.0, redCDF = 0.0, greenCDF = 0.0;
  std::vector<double> blueArray;
  std::vector<double> greenArray;
  std::vector<double> redArray;

  for (int intensity = 0; intensity < 256; ++intensity) {
      double bluePixelCount = histogram.at<int>(0, intensity);
      double greenPixelCount = histogram.at<int>(1, intensity);
      double redPixelCount = histogram.at<int>(2, intensity);

      blueCDF += bluePixelCount / totalPixelCount;
      greenCDF += greenPixelCount / totalPixelCount;
      redCDF += redPixelCount / totalPixelCount;

      blueArray.push_back(blueCDF);
      greenArray.push_back(greenCDF);
      redArray.push_back(redCDF);
  }

  return {blueArray, greenArray, redArray};
}

cv::Mat receiveimage(){
    std::string imagePath = "images/images.jpg";
    cv::Mat image = cv::imread(imagePath);


    if (image.empty()) {
        std::cerr << "Could not open or find the image at path: " << imagePath << std::endl;
        return cv::Mat{};
    }

    std::cout << "Image loaded successfully (" << image.cols << "x" << image.rows << ")" << std::endl;

    return image;
}

cv::Mat translateImage(const cv::Mat& image, int shift_x, int shift_y){
  validateImage(image, "translateImage()");
  cv::Mat output = cv::Mat::zeros(image.size(), image.type());
  
  for (int row = 0; row < output.rows; ++row) {
    for (int column = 0; column < output.cols; ++column) {
      int source_x = column - shift_x;
      int source_y = row - shift_y;

      // Check whether the source coordinates are inside the input, copy if so
      if (source_x >= 0 && source_x < image.cols && source_y >= 0 && source_y < image.rows){
        output.at<cv::Vec3b>(row, column) = image.at<cv::Vec3b>(source_y, source_x);
      }
    }
}

  return output;
}

cv::Mat scaleImage(const cv::Mat& image, double scale_x, double scale_y) {
  validateImage(image, "scaleImage()");

  if (!std::isfinite(scale_x) || !std::isfinite(scale_y) ||
      scale_x <= 0.0 || scale_y <= 0.0) {
    throw std::invalid_argument(
        "numbers invalid - scaleImage");
  }

  cv::Mat output = cv::Mat::zeros(image.size(), image.type());

  for (int row = 0; row < output.rows; ++row) {
    for (int column = 0; column < output.cols; ++column) {
      double source_x = column / scale_x;
      double source_y = row / scale_y;

      double nearest_x = std::round(source_x);
      double nearest_y = std::round(source_y);

      if (nearest_x >= 0 && nearest_x < image.cols &&
          nearest_y >= 0 && nearest_y < image.rows) {
        int source_column = static_cast<int>(nearest_x);
        int source_row = static_cast<int>(nearest_y);

        output.at<cv::Vec3b>(row, column) = image.at<cv::Vec3b>(source_row, source_column);
      }
    }
  }

  return output;
}

cv::Mat rotateImage(const cv::Mat& image, double angle_degrees){
  validateImage(image, "rotateImage()");

  if(!std::isfinite(angle_degrees)){
    throw std::invalid_argument("no angle - rotateImage");
  }

  const double pi = std::acos(-1.0);
  double radians = std::remainder(angle_degrees, 360.0) * pi / 180;
  double cos_angle = std::cos(radians);
  double sin_angle = std::sin(radians);

  double center_x = (image.cols - 1) / 2.0;
  double center_y = (image.rows - 1) / 2.0;

  cv::Mat output = cv::Mat::zeros(image.size(), image.type());

  for (int row = 0; row < output.rows; ++row) {
    for (int column = 0; column < output.cols; ++column) {
      double x = column - center_x;
      double y = row - center_y;

      double source_x = center_x + x * cos_angle + y * sin_angle;
      double source_y = center_y - x * sin_angle + y * cos_angle;

      double nearest_x = std::round(source_x);
      double nearest_y = std::round(source_y);

      if (nearest_x >= 0 && nearest_x < image.cols &&
          nearest_y >= 0 && nearest_y < image.rows) {
        int source_column = static_cast<int>(nearest_x);
        int source_row = static_cast<int>(nearest_y);

        output.at<cv::Vec3b>(row, column) = image.at<cv::Vec3b>(source_row, source_column);
      }
    }
  }


  return output;
}

}  // namespace photo_booth
