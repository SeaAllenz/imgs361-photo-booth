#include "photo_booth/AppConfig.hpp"

#include <cmath>
#include <set>
#include <stdexcept>
#include <string>
#include <toml++/toml.hpp>

namespace photo_booth {

namespace {

void validateKeys(const toml::table& table,
                  const std::set<std::string>& valid_keys,
                  const std::string& section) {
  for (const auto& [key, value] : table) {
    (void)value;
    const std::string key_string{key.str()};

    if (valid_keys.find(key_string) == valid_keys.end()) {
      throw std::runtime_error("Unknown configuration option: " + section +
                               "." + key_string);
    }
  }
}

void validateConfigKeys(const toml::table& table) {
  const std::set<std::string> valid_sections{
      "camera",
      "preview",
      "capture",
      "processing",
  };

  for (const auto& [key, value] : table) {
    const std::string section{key.str()};

    if (valid_sections.find(section) == valid_sections.end()) {
      throw std::runtime_error("Unknown configuration section: " + section);
    }

    if (!value.is_table()) {
      throw std::runtime_error("Configuration section '" + section +
                               "' must be a TOML table.");
    }
  }

  if (const auto* camera = table["camera"].as_table()) {
    validateKeys(*camera, {"device", "width", "height", "fps", "fourcc"},
                 "camera");
  }

  if (const auto* preview = table["preview"].as_table()) {
    validateKeys(*preview, {"mirror", "rotation", "window_name"}, "preview");
  }

  if (const auto* capture = table["capture"].as_table()) {
    validateKeys(*capture, {"save_directory", "warmup_frames"}, "capture");
  }

  if (const auto* processing = table["processing"].as_table()) {
    // Add new baseline-processing configuration keys to this list.
    validateKeys(*processing, {"channel_swap_enabled"}, "processing");
  }
}

template <typename T>
void readOptional(const toml::table& table, const std::string& section,
                  const std::string& key, T& destination) {
  const auto node = table[section][key];

  if (!node) {
    return;
  }

  const auto value = node.template value<T>();
  if (!value) {
    throw std::runtime_error("Configuration option " + section + "." + key +
                             " has the wrong type.");
  }

  destination = *value;
}

void validateValues(const AppConfig& config) {
  if (config.camera.device < 0) {
    throw std::runtime_error("camera.device must be >= 0.");
  }

  if (config.camera.width < 0) {
    throw std::runtime_error("camera.width must be >= 0.");
  }

  if (config.camera.height < 0) {
    throw std::runtime_error("camera.height must be >= 0.");
  }

  if (!std::isfinite(config.camera.fps) || config.camera.fps < 0.0) {
    throw std::runtime_error("camera.fps must be finite and >= 0.");
  }

  if (!config.camera.fourcc.empty() && config.camera.fourcc.size() != 4) {
    throw std::runtime_error(
        "camera.fourcc must be empty or contain exactly four characters.");
  }

  if (config.preview.rotation != 0 && config.preview.rotation != 90 &&
      config.preview.rotation != 180 && config.preview.rotation != 270) {
    throw std::runtime_error("preview.rotation must be 0, 90, 180, or 270.");
  }

  if (config.preview.window_name.empty()) {
    throw std::runtime_error("preview.window_name cannot be empty.");
  }

  if (config.capture.save_directory.empty()) {
    throw std::runtime_error("capture.save_directory cannot be empty.");
  }

  if (config.capture.warmup_frames < 0) {
    throw std::runtime_error("capture.warmup_frames must be >= 0.");
  }
}

}  // namespace

AppConfig loadConfig(const std::filesystem::path& filename) {
  AppConfig config;
  toml::table table;

  try {
    table = toml::parse_file(filename.string());
  } catch (const toml::parse_error& error) {
    throw std::runtime_error("Error parsing configuration file '" +
                             filename.string() +
                             "': " + std::string{error.description()});
  }

  validateConfigKeys(table);

  readOptional(table, "camera", "device", config.camera.device);
  readOptional(table, "camera", "width", config.camera.width);
  readOptional(table, "camera", "height", config.camera.height);
  readOptional(table, "camera", "fps", config.camera.fps);
  readOptional(table, "camera", "fourcc", config.camera.fourcc);
  readOptional(table, "preview", "mirror", config.preview.mirror);
  readOptional(table, "preview", "rotation", config.preview.rotation);
  readOptional(table, "preview", "window_name", config.preview.window_name);

  readOptional(table, "capture", "save_directory",
               config.capture.save_directory);
  readOptional(table, "capture", "warmup_frames", config.capture.warmup_frames);

  // Read new baseline-processing configuration values here.
  readOptional(table, "processing", "channel_swap_enabled",
               config.processing.channel_swap_enabled);

  validateValues(config);
  return config;
}

ImageCapture::Configuration makeImageCaptureConfiguration(
    const CameraConfig& camera_config) {
  ImageCapture::Configuration configuration;
  configuration.device_index = camera_config.device;
  configuration.width = camera_config.width;
  configuration.height = camera_config.height;
  configuration.frames_per_second = camera_config.fps;
  configuration.fourcc = camera_config.fourcc;
  return configuration;
}

}  // namespace photo_booth
