#include <chrono>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <string>

#include "photo_booth/AppConfig.hpp"
#include "photo_booth/ImageCapture.hpp"
#include "photo_booth/ImageProcessing.hpp"

namespace {

struct ProcessingState {
  bool inversion_enabled{false};
};

cv::Mat processFrame(const cv::Mat& frame,
                     const photo_booth::ProcessingConfig& config,
                     const ProcessingState& state) {
  cv::Mat processed_frame = frame.clone();

  //
  // Baseline pipeline operations
  //
  // These operations are controlled by the configuration file and are
  // applied automatically to every acquired frame when enabled.
  //
  if (config.channel_swap_enabled) {
    processed_frame = photo_booth::swapRedBlueChannels(processed_frame);
  }

  //
  // Optional pipeline operations
  //
  // These operations are controlled interactively while the application
  // is running.
  //
  if (state.inversion_enabled) {
    processed_frame = photo_booth::invertImage(processed_frame);
  }

  return processed_frame;
}

void showPreviewFrame(const cv::Mat& frame,
                      const photo_booth::PreviewConfig& config) {
  cv::Mat preview_frame = frame.clone();

  switch (config.rotation) {
    case 0:
      break;

    case 90:
      cv::rotate(preview_frame, preview_frame, cv::ROTATE_90_CLOCKWISE);
      break;

    case 180:
      cv::rotate(preview_frame, preview_frame, cv::ROTATE_180);
      break;

    case 270:
      cv::rotate(preview_frame, preview_frame, cv::ROTATE_90_COUNTERCLOCKWISE);
      break;
  }

  if (config.mirror) {
    cv::flip(preview_frame, preview_frame, 1);
  }

  cv::imshow(config.window_name, preview_frame);
}

bool handleKey(const int key, ProcessingState& state) {
  switch (key) {
    case 27:
    case 'q':
    case 'Q':
      return false;

    case 'i':
    case 'I':
      state.inversion_enabled = !state.inversion_enabled;

      std::cout << "Image inversion: "
                << (state.inversion_enabled ? "ON" : "OFF") << '\n';

      break;

    default:
      break;
  }

  return true;
}

std::string makeTimestampFilename() {
  const auto now = std::chrono::system_clock::now();

  return std::format("{:%Y-%m-%dT%H-%M-%S}.png",
                     std::chrono::floor<std::chrono::milliseconds>(now));
}

}  // namespace

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
        std::cout
            << "Usage: " << argv[0] << " [config.toml]\n\n"
            << "Runs the semester photo-booth image-processing application.\n";

        return EXIT_SUCCESS;
      }

      config_path = argument;
    }

    //
    // Load the application configuration.
    //
    const auto config = photo_booth::loadConfig(config_path);

    //
    // Create the save directory if it does not already exist.
    //
    const std::filesystem::path save_directory{config.capture.save_directory};

    std::filesystem::create_directories(save_directory);
    std::cout << "Capture directory: " << save_directory << '\n';

    //
    // Configure and open the camera.
    //
    photo_booth::ImageCapture camera(
        photo_booth::makeImageCaptureConfiguration(config.camera));

    if (!camera.open()) {
      std::cerr << camera.errorMessage() << '\n';

      return EXIT_FAILURE;
    }

    std::cout << camera << '\n';

    //
    // Create the preview window.
    //
    cv::namedWindow(config.preview.window_name, cv::WINDOW_AUTOSIZE);

    //
    // Runtime state for optional processing operations.
    //
    ProcessingState processing_state;

    //
    // Main application loop.
    //
    while (true) {
      if (!camera.read()) {
        std::cerr << camera.errorMessage() << '\n';

        return EXIT_FAILURE;
      }

      //
      // Apply the image-processing pipeline.
      //
      cv::Mat processed_frame =
          processFrame(camera.image(), config.processing, processing_state);

      //
      // Display the processed frame.
      //
      showPreviewFrame(processed_frame, config.preview);

      //
      // Process keyboard input.
      //
      const int key = cv::waitKey(1);

      if (!handleKey(key, processing_state)) {
        break;
      }

      if (key == ' ') {                                          
        const auto filename = save_directory / makeTimestampFilename();
                                                                 
        if (cv::imwrite(filename.string(), processed_frame)) {
          std::cout << "Captured: " << filename << '\n';
        } else {                     
          std::cerr << "Failed to save image: " << filename << '\n';
        }                                                        
      }                                                            
    }

    cv::destroyAllWindows();

  } catch (const cv::Exception& error) {
    std::cerr << "OpenCV error: " << error.what() << '\n';

    return EXIT_FAILURE;

  } catch (const std::exception& error) {
    std::cerr << "Error: " << error.what() << '\n';

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
