#ifndef PHOTO_BOOTH_APP_CONFIG_HPP
#define PHOTO_BOOTH_APP_CONFIG_HPP

#include <filesystem>
#include <string>

#include "photo_booth/ImageCapture.hpp"

namespace photo_booth {

/**
 * @brief Camera settings used by the photo booth and camera utilities.
 */
struct CameraConfig {
  int device{0};
  int width{1280};
  int height{720};
  double fps{30.0};
  std::string fourcc{};
};

/**
 * @brief Settings controlling the photo booth preview window.
 */
struct PreviewConfig {
  bool mirror{false};
  int rotation{0};
  std::string window_name{"Photo Booth"};
};

/**
 * @brief Settings used when an application saves a captured image.
 */
struct CaptureConfig {
  std::string output_filename{"captured_image.png"};
  int warmup_frames{10};
};

/**
 * @brief Settings used for baseline processing operations.
 *
 * Keep these settings intentionally simple and flat. New configurable
 * processing operations can add their enable flag and parameters here.
 */
struct ProcessingConfig {
  bool channel_swap_enabled{false};
};

/**
 * @brief Complete application configuration loaded from TOML.
 *
 * Additional configuration groups can be added here as the photo booth grows
 * during the semester.
 */
struct AppConfig {
  CameraConfig camera;
  PreviewConfig preview;
  CaptureConfig capture;
  ProcessingConfig processing;
};

/**
 * @brief Loads and validates an application configuration from a TOML file.
 *
 * Missing values use the defaults declared in AppConfig. Unknown sections,
 * unknown keys, incorrect value types, and invalid values are rejected.
 *
 * @throws std::runtime_error when the file cannot be parsed or validated.
 */
AppConfig loadConfig(const std::filesystem::path& filename);

/**
 * @brief Converts application camera settings to ImageCapture settings.
 */
ImageCapture::Configuration makeImageCaptureConfiguration(
    const CameraConfig& camera_config);

}  // namespace photo_booth

#endif
