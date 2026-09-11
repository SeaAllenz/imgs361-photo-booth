#pragma once

#include <opencv2/core.hpp>
#include <tuple>

namespace photo_booth {

/**
 * @brief Calculates the histogram of each channel of an 8-bit BGR image.
 *
 * Channel histograms are returned in the rows of a 3x256 CV_32S.
 */
std::tuple<cv::Mat, cv::Mat> calcHist(const cv::Mat& image);

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
 * @brief Spreads out high density areas an 8-bit BGR image and histrogram.
 *
 * Histrogram Equalization equation is used.
 */
std::tuple<cv::Mat, cv::Mat> histogramEqualization(const cv::Mat& histogram, const cv::Mat& image);
}  // namespace photo_booth