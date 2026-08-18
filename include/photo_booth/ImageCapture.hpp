#ifndef PHOTO_BOOTH_IMAGE_CAPTURE_HPP
#define PHOTO_BOOTH_IMAGE_CAPTURE_HPP

#include <cstdint>
#include <iosfwd>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <string>

namespace photo_booth {

/**
 * @brief A small, synchronous wrapper around cv::VideoCapture.
 *
 * ImageCapture opens a built-in camera or USB webcam and stores the most
 * recently captured image in a cv::Mat.
 *
 * Camera properties such as width, height, frame rate, and FOURCC are requests.
 * The camera, operating-system driver, or OpenCV backend may select different
 * values. Use cameraInfo() after opening the camera to inspect the reported
 * settings.
 *
 * Successful read() calls provide 8-bit, three-channel BGR images (CV_8UC3).
 * This representation is the image contract used throughout the Photo Booth
 * processing pipeline.
 */
class ImageCapture {
 public:
  /**
   * @brief Requested camera configuration.
   *
   * A width, height, or frames_per_second value of zero means that the
   * camera/backend default should be used.
   *
   * fourcc should either be empty or contain exactly four characters,
   * such as "MJPG".
   */
  struct Configuration {
    int device_index{0};
    int api_preference{cv::CAP_ANY};

    int width{0};
    int height{0};
    double frames_per_second{0.0};

    std::string fourcc{};
  };

  /**
   * @brief Camera properties reported by OpenCV after the camera is opened.
   */
  struct CameraInfo {
    int device_index{0};
    int backend_id{0};

    int width{0};
    int height{0};
    double frames_per_second{0.0};

    std::string fourcc{};
    std::string backend_name{};
  };

  ImageCapture() = default;
  explicit ImageCapture(Configuration configuration);

  ~ImageCapture();

  ImageCapture(const ImageCapture&) = delete;
  ImageCapture& operator=(const ImageCapture&) = delete;
  ImageCapture(ImageCapture&&) = delete;
  ImageCapture& operator=(ImageCapture&&) = delete;

  /**
   * @brief Opens the camera described by the current configuration.
   *
   * @return true when the camera opens successfully.
   */
  bool open();

  /**
   * @brief Replaces the current configuration and opens that camera.
   *
   * The new configuration is validated before the current camera is closed.
   * An invalid replacement configuration therefore leaves the current camera
   * unchanged. A valid replacement configuration closes the current camera
   * before attempting to open the newly configured device.
   *
   * @return true when the camera opens successfully.
   */
  bool open(const Configuration& configuration);

  /**
   * @brief Releases the camera and clears the current image.
   */
  void close() noexcept;

  /**
   * @brief Returns true when the camera is open.
   */
  [[nodiscard]] bool isOpen() const;

  /**
   * @brief Captures the next frame into the internal cv::Mat.
   *
   * @return true when a nonempty image was captured.
   */
  bool read();

  /**
   * @brief Returns the most recently captured image without copying it.
   *
   * The reference remains valid until read(), open(), or close() is called,
   * or until the ImageCapture object is destroyed.
   */
  [[nodiscard]] const cv::Mat& image() const noexcept;

  /**
   * @brief Returns an independent copy of the most recently captured image.
   */
  [[nodiscard]] cv::Mat imageCopy() const;

  /**
   * @brief Returns true when a nonempty image is available.
   */
  [[nodiscard]] bool hasImage() const;

  /**
   * @brief Returns the requested camera configuration.
   */
  [[nodiscard]] const Configuration& configuration() const noexcept;

  /**
   * @brief Returns camera settings reported by OpenCV.
   *
   * When the camera is not open, numeric values other than device_index
   * remain zero and string values remain empty.
   */
  [[nodiscard]] CameraInfo cameraInfo() const;

  /**
   * @brief Attempts to set an arbitrary OpenCV capture property.
   *
   * Property support depends on the camera, driver, and OpenCV backend.
   */
  bool setProperty(int property, double value);

  /**
   * @brief Reads an arbitrary OpenCV capture property.
   *
   * @return the reported property value, or 0.0 when the camera is not open.
   */
  [[nodiscard]] double property(int property) const;

  /**
   * @brief Returns a description of the most recent error.
   *
   * A successful open() or read() clears the previous error.
   */
  [[nodiscard]] const std::string& errorMessage() const noexcept;

  /**
   * @brief Returns the number of successfully captured frames.
   *
   * The counter is reset whenever the camera is opened or closed.
   */
  [[nodiscard]] std::uint64_t frameNumber() const noexcept;

 private:
  void applyConfiguration();
  void clearImage() noexcept;
  void setError(std::string message);
  void clearError() noexcept;

  static int fourccFromString(const std::string& value);
  static std::string fourccToString(int value);

  Configuration configuration_{};
  cv::VideoCapture capture_{};
  cv::Mat image_{};

  std::string error_message_{};
  std::uint64_t frame_number_{0};
};

std::ostream& operator<<(std::ostream& output,
                         const ImageCapture::CameraInfo& information);

std::ostream& operator<<(std::ostream& output, const ImageCapture& camera);

}  // namespace photo_booth

#endif
