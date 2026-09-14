#include "photo_booth/ImageProcessing.hpp"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <stdexcept>
#include <string>
#include <vector>
#include <tuple>
#include <cmath>
#include <iostream>

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
  validateImage(image, "HistrogramEqualizedImage()");
  
  int totalPixelCount = image.rows * image.cols;

  // Equalization equation for each color channel in a pixel -> intensity level
  auto [redArray, greenArray, blueArray] = getRgbChannelPixels(histogram);

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


cv::Mat histrogramMatching(const cv::Mat&histrogram, constt cv::Mat& image, const cv::Mat&newImage){
  cv::Mat newHistrogram = calcHist(newImage);
  cv::Mat matchingImage = image.clone();
  cv::Mat matchingHistogram = cv::Mat::zeros(histogram.size(), histogram.type());

  auto [blueCDFArray, greenArray, redArray] = getRgbChannelCDF(histogram);
  auto [newblueArray, newgreenArray, newredArray] = getRgbChannelCDF(newhistogram);

  std::vector<int> blueMatchingPixels(256);
  std::vector<int> greenMatchingPixels(256);
  std::vector<int> redMatchingPixels(256);

  //Blue
  for (int intensity = 0; intensity < 256; ++intensity){
    //Get new CDF for pixelCount -- editing equalize to return a bluearray for use
    //loop through the newBlueArray, see which one matches, 
    //-- while also seeing if the cdf past the histrogram cdf,
    //--- and comparing between +1 and -1

    //NOTE:: this is just the cdf, so you'll need the 255* thing whenever
    // -Afterwards repeat for each color channel, so red double for loop and green one
    // - then run through rows and col, make value = matchingImage -> value[0] = blueMatchingPixels, etc
    double CDF = blueArray.at(intensity);
    for (int newIntensity=0; newIntensity < 256; ++newIntensity){
      double newCDF = newblueArray.at(newIntensity);
      if (newCDF > CDF){
        double lowerCDF;
        if (newIntensity == 0){ //Out of bounds edge
          lowerCDF = 0.0
        }else{
          lowerCDF = CDF - (newblueArray.at(newIntensity - 1));
        }
        double higherCDF = (newblueArray.at(newIntensity)) - CDF;

        //Ternary opertation to see what's close to the target CDF
        blueMatchingPixels.at(intensity) = (lowerCDF < higherCDF) ? newIntensity - 1 : newIntensity;

        break;
      }
    }



  }

}

std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> getRgbChannelPixels(const cv::Mat&histrogram){
  
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

std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> getRgbChannelCDF(const cv::Mat&histrogram){
  
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
    cv::Mat image = cv::imread("images/image.jpg");

    if (image.empty()) {
        std::cerr << "Could not open or find the image at path: " << imagePath << std::endl;
        return 1;
    }

    std::cout << "Image loaded successfully (" << image.cols << "x" << image.rows << ")" << std::endl;

    cv::imshow("Loaded Image", image);
    cv::waitKey(0);

    return image;
}

}  // namespace photo_booth
