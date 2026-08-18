#ifndef PHOTO_BOOTH_IMAGE_PROCESSING_HPP
#define PHOTO_BOOTH_IMAGE_PROCESSING_HPP

#include <opencv2/core.hpp>

namespace photo_booth {

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

}  // namespace photo_booth

#endif
