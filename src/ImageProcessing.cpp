#include "photo_booth/ImageProcessing.hpp"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <stdexcept>
#include <string>
#include <vector>
#include <tuple>
#include <cmath>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

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

std::tuple<cv::Mat, cv::Mat> histogramEqualization(const cv::Mat& histogram, const cv::Mat& image) {
  validateImage(image, "HistogramEqualizedImage()");
  
  int totalPixelCount = image.rows * image.cols;

  // Equalization equation for each color channel in a pixel -> intensity level
  auto [blueArray, greenArray, redArray] = getRgbChannelPixels(histogram, totalPixelCount);

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



//Only need to be calculated once

std::tuple<cv::Mat, cv::Mat> histogramMatching(const cv::Mat& histogram, const cv::Mat& image,
                    const std::vector<double>& newblueArray, const std::vector<double>& newgreenArray, const std::vector<double>& newredArray){
  int sourcePixelCount = image.rows * image.cols;

  cv::Mat matchingImage = image.clone();
  cv::Mat matchingHistogram = cv::Mat::zeros(histogram.size(), histogram.type());

  auto [blueCDFArray, greenCDFArray, redCDFArray] = getRgbChannelCDF(histogram, sourcePixelCount);

  std::vector<int> blueMatchingIntensity(256);
  std::vector<int> greenMatchingIntensity(256);
  std::vector<int> redMatchingIntensity(256);

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

  for (int row = 0; row < image.rows; ++row) {
    for (int column = 0; column < image.cols; ++column) {
      auto& pixel = matchingImage.at<cv::Vec3b>(row, column);
      
      //Blue
      int oldBlue = pixel[0];
      int newBlue = blueMatchingIntensity[oldBlue];
      pixel[0] = newBlue;
      matchingHistogram.at<int>(0, pixel[0])++;

      //Green
      int oldGreen = pixel[1];
      int newGreen = greenMatchingIntensity[oldGreen];
      pixel[1] = newGreen;
      matchingHistogram.at<int>(1, pixel[1])++;

      //Red 
      int oldRed = pixel[2];
      int newRed = redMatchingIntensity[oldRed];
      pixel[2] = newRed;
      matchingHistogram.at<int>(2, pixel[2])++;
    }
  }

  return {matchingHistogram, matchingImage};
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

}  // namespace photo_booth
