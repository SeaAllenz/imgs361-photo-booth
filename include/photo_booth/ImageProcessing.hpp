#pragma once

#include <opencv2/core.hpp>
#include <tuple>
#include <vector>

namespace photo_booth {

/**
 * @brief Calculates the histogram of each channel of an 8-bit BGR image.
 *
 * Channel histograms are returned in the rows of a 3x256 CV_32S.
 */
cv::Mat calcHist(const cv::Mat& image);

/**
 * @brief Swaps the blue and red channels of an 8-bit BGR image.
 */
cv::Mat swapRedBlueChannels(const cv::Mat& image);

/**
 * @brief Inverts every channel of an 8-bit BGR image.
 *
 * Each output channel value is 255 minus the corresponding input value.
 */
cv::Mat invertImage(const cv::Mat& image);

/**
 * @brief Spreads out high density areas an 8-bit BGR image and histogram.
 *
 * Histogram Equalization equation is used.
 * 
 * Uses getRgbChannelPixels()
 */
std::tuple<cv::Mat, cv::Mat> histogramEqualization(const cv::Mat& histogram, const cv::Mat& image);


/**
 * @brief Swap the image intensities old the old image with the new image
 *
 * Histogram Matching equation is used.
 * 
 * Uses calcHist()
 * Uses getRgbChannelCDF()
 */
std::tuple<cv::Mat, cv::Mat> histogramMatching(const cv::Mat& histogram, const cv::Mat& image, 
    const std::vector<double>& newblueArray, const std::vector<double>& newgreenArray, const std::vector<double>& newredArray);

/**
 * @brief Places the pixel density into an three vector arrays by order of intensity for each color channels
 */
std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> getRgbChannelPixels(const cv::Mat&histogram, const int totalPixelCount);

/**
 * @brief Places the CDF calcuation into an three vector arrays by order of intensity for each color channels
 */
std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> getRgbChannelCDF(const cv::Mat&histogram, const int totalPixelCount);

/**
 * @brief Helper function to take whatever image is in the images folder and return it
 */
cv::Mat receiveimage();
}  // namespace photo_booth

