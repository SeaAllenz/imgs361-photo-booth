#include <chrono>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <string>

#include "photo_booth/AppConfig.hpp"
#include "photo_booth/ImageCapture.hpp"

int main(int argc, char* argv[]) {
  try {
    //
    // Determine which configuration file to use.
    //
    std::filesystem::path config_path{"config.toml"};

    if (argc > 2) {
      std::cerr << "Usage: " << argv[0] << " [config.toml]\n";

      return EXIT_FAILURE;
    }

    if (argc == 2) {
      const std::string argument{argv[1]};

      if (argument == "-h" || argument == "--help") {
        std::cout << "Usage: " << argv[0] << " [config.toml]\n\n"
                  << "Captures one image from a built-in or USB camera.\n";

        return EXIT_SUCCESS;
      }

      config_path = argument;
    }

    //
    // Load the application configuration.
    //
    const auto config = photo_booth::loadConfig(config_path);

    //
    // Configure and open the camera.
    //
    photo_booth::ImageCapture camera{
        photo_booth::makeImageCaptureConfiguration(config.camera)};

    if (!camera.open()) {
      std::cerr << camera.errorMessage() << '\n';

      return EXIT_FAILURE;
    }

    //
    // Discard startup frames while auto-exposure and white balance settle.
    //
    for (int frame = 0; frame < config.capture.warmup_frames; ++frame) {
      if (!camera.read()) {
        std::cerr << camera.errorMessage() << '\n';

        return EXIT_FAILURE;
      }
    }

    //
    // Always capture one final frame, even when warmup_frames is zero.
    //
    if (!camera.read()) {
      std::cerr << camera.errorMessage() << '\n';

      return EXIT_FAILURE;
    }

    //
    // Form the capture filename.
    //
    const std::filesystem::path save_directory{config.capture.save_directory};

    std::filesystem::create_directories(save_directory);

    const auto now = std::chrono::system_clock::now();
    std::string timestamp =
        std::format("{:%Y-%m-%dT%H-%M-%S}.png",
                    std::chrono::floor<std::chrono::milliseconds>(now));

    const auto output_filename = save_directory / timestamp;

    //
    // Save the acquired image.
    //
    if (!cv::imwrite(output_filename.string(), camera.image())) {
      std::cerr << "Could not write " << output_filename << ".\n";

      return EXIT_FAILURE;
    }

    std::cout << "Saved " << output_filename << " using camera "
              << config.camera.device << ".\n";

  } catch (const cv::Exception& error) {
    std::cerr << "OpenCV error: " << error.what() << '\n';

    return EXIT_FAILURE;

  } catch (const std::exception& error) {
    std::cerr << "Error: " << error.what() << '\n';

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
