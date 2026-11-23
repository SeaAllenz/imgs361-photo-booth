#pragma once

#include <opencv2/core.hpp>
#include <tuple>
#include <vector>
#include <array>
#include <string>

namespace photo_booth {

enum class FrequencyFilterShape {
  Ideal,
  Gaussian,
  Butterworth
};

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
 * @brief Quantizes the intensity values of every channel of an 8-bit BGR image.
 *
 * Each output channel value is divided by 32. As 256/8 is 32.
 * Then  new_intensity_value = lowerbound + (width_of_level/2)
 * So its, new_intensity_value = (level * 32) + (16)
 */
cv::Mat quantization(const cv::Mat& image);

/**
 * @brief Spreads out high density areas an 8-bit BGR image and histogram.
 *
 * Histogram Equalization equation is used.
 * 
 */
cv::Mat histogramEqualization(const cv::Mat& image);

/**
 * @brief Places the pixel density into an three vector arrays by order of intensity for each color channels
 */
std::tuple<std::vector<int>, std::vector<int>, std::vector<int>> getRgbChannelPixels(const cv::Mat& histogram, const int totalPixelCount );

/**
 * @brief Swap the image intensities old the old image with the new image
 *
 * Histogram Matching equation is used.
 * 
 */
cv::Mat histogramMatching(const cv::Mat& image, const cv::Mat& newImage);

/**
 * @brief Places the CDF calcuation into an three vector arrays by order of intensity for each color channels
 */
std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> getRgbChannelCDF(const cv::Mat& histogram, const int totalPixelCount );

/**
 * @brief Helper function to take whatever image is in the images folder and return it
 */
cv::Mat receiveimage();

/**
 * @brief
 */
cv::Mat translateImage(const cv::Mat& image, int shift_x, int shift_y);

/**
 * @brief
 */
cv::Mat scaleImage(const cv::Mat& image, double scale_x, double scale_y);

/**
 * @brief
 */
cv::Mat rotateImage(const cv::Mat& image, double angle_degrees);

/**
 * @brief
 */
cv::Mat perspectiveImage(const cv::Mat& image, const cv::Matx33d& map_to_source);

/**
 * @brief
 * cv::Point2d stores one (x, y) pair using doubles.
 * std::array<cv::Point2d, 4> stores exactly four points
 * 
 */
cv::Matx33d calculatePerspectiveMap(const std::array<cv::Point2d, 4>& source_points, const std::array<cv::Point2d, 4>& destination_points);

/**
 * @brief Creates a log-polar view using angle and distance from the image center.
 *
 * Rows represent angle, while columns represent logarithmically spaced distance.
 */
cv::Mat logPolarImage(const cv::Mat& image);

/**
 *@brief Calculates each output pixel using weighted values from neighboring pixels.
 *
 * The kernel controls the effect. Absolute response makes negative results positive.
 */
cv::Mat spatialFilter(const cv::Mat& image, const cv::Mat& kernel, bool absolute_response = false);

/**
 *@brief Filters image frequencies using an ideal, Gaussian, or Butterworth shape.
 *
 * Lowpass smooths the image, while highpass emphasizes detail.
 * Cutoff controls the frequency boundary, and order controls Butterworth steepness.
 */
cv::Mat frequencyFilter(const cv::Mat& image, double cutoff, bool highpass, FrequencyFilterShape shape = FrequencyFilterShape::Ideal, int order = 2);

/**
 *@brief Loads an icon and prepares it as a floating-point grascale message
 */
cv::Mat prepareMessageIcon(const std::string& filename);

/**
 *@brief Creates a reproducible random-phase carrier in the frequency domain
 */
cv::Mat createPhaseCarrierSpectrum(cv::Size size, unsigned int seed);

/**
 *@brief produces dispersed patten by multiplying icons complex coefficeints by the carriers
 */
cv::Mat disperseMessage(const cv::Mat& message, const cv::Mat& carrier_spectrum);

/**
 *@brief Recovers a message by reversing the phase spreading with the same carrier.
 */
cv::Mat recoverMessage(const cv::Mat& signal, const cv::Mat& carrier_spectrum);

/**
 *@brief Adds a repeated dispersed message to an image using the selected strength.
 */
cv::Mat embedMessage(const cv::Mat& image, const cv::Mat& dispersed, double strength);

/**
 *@brief Averages aligned image tiles and uses the carrier to recover the embedded message.
 */
cv::Mat extractEmbeddedMessage(const cv::Mat& image, const cv::Mat& carrier_spectrum);
}  // namespace photo_booth