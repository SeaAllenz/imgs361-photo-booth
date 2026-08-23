# Photo Booth

`Photo Booth` is the starting C++20 project for the semester-long photo booth assignment in **IMGS.361 Image Processing**. The starter code provides camera acquisition, typed TOML configuration, a minimal live-preview example, and a closely related photo-booth application with an explicit image-processing pipeline for students to extend throughout the semester.

The project intentionally separates reusable support code from application-specific behavior. Camera acquisition, configuration loading, and starter image-processing functions live in the `photo_booth` namespace; the applications contain their own control flow and runtime state.

## Starter applications

The project builds three executables:

- `live_preview` — minimal camera acquisition and display example
- `photo_booth` — semester project baseline with an explicit processing pipeline
- `capture_single_image` — diagnostic/example utility that saves one acquired frame

### `live_preview`

`apps/live_preview.cpp` intentionally has no `processFrame()` function. It acquires each camera frame, applies only display-oriented preview rotation/mirroring, and displays the result:

```text
camera -> display
```

This provides a simple reference showing that an explicit processing stage is not required merely to acquire and display camera imagery.

### `photo_booth`

`apps/photo_booth.cpp` inserts an explicit processing stage between acquisition and display:

```text
camera -> processFrame() -> display
```

The starter pipeline demonstrates two different kinds of processing controls:

- **Baseline operation:** `swapRedBlueChannels()` is controlled by `processing.channel_swap_enabled` in `config.toml` and, when enabled, is applied to every frame.
- **Optional operation:** `invertImage()` is toggled interactively by pressing `i` while the program is running. Its state is independent of other optional operations that may be added later.

These simple operations are intended to demonstrate architecture rather than serve as substantive course algorithms.

## Image representation convention

Successful `ImageCapture::read()` calls provide **8-bit, three-channel BGR images (`CV_8UC3`)**. This is the image representation contract for the Photo Booth processing pipeline.

Even an operation that conceptually produces grayscale imagery should return a three-channel BGR image, with the grayscale value replicated in the B, G, and R channels. Maintaining one image representation throughout the pipeline simplifies composition of processing operations.

OpenCV's property is named `CAP_PROP_CONVERT_RGB`, but normal decoded OpenCV color imagery uses BGR channel order. `ImageCapture` requests this conversion internally and verifies the returned frame type; it is not exposed as a TOML option.

## Project organization

```text
imgs361-photo-booth/
├── CMakeLists.txt
├── config.toml
├── LICENSE
├── README.md
├── apps/
│   ├── capture_single_image.cpp
│   ├── live_preview.cpp
│   └── photo_booth.cpp
├── include/
│   └── photo_booth/
│       ├── AppConfig.hpp
│       ├── ImageCapture.hpp
│       └── ImageProcessing.hpp
├── src/
│   ├── AppConfig.cpp
│   ├── ImageCapture.cpp
│   └── ImageProcessing.cpp
└── notes/
    └── git_workflows
```

`ImageCapture` wraps OpenCV's `cv::VideoCapture`. `AppConfig` defines and loads the typed application configuration. `ImageProcessing` contains reusable processing functions used by the photo-booth pipeline.

## Student-facing files

Most image-processing work during the semester should be concentrated in a small part of the project:

```text
apps/photo_booth.cpp
include/photo_booth/ImageProcessing.hpp
src/ImageProcessing.cpp
config.toml
```

When a new processing operation needs a persistent configuration value, students will also modify:

```text
include/photo_booth/AppConfig.hpp
src/AppConfig.cpp
```

`ImageCapture` and most of the top-level CMake configuration can be treated as supplied infrastructure unless a project extension specifically requires changes there. Ordinary processing operations added to the existing `ImageProcessing.cpp` do **not** require a CMake change.

## Requirements

- CMake 3.30 or later
- a C++20 compiler
- OpenCV 4.x or 5.x with `core`, `videoio`, `highgui`, `imgcodecs`, and `imgproc`
- Boost with the `program_options` component
- Eigen3
- toml++ 3.4.0, downloaded automatically by CMake using `FetchContent`

Boost.Program_options and Eigen are intentionally pre-provisioned for student applications even though the starter applications do not yet use them directly. This allows students to use either library later in the semester without changing the project build environment.

## Build

From the project root:

```sh
cmake -S . -B build
cmake --build build
```

Compiled executables are placed in `build/bin`:

```text
build/bin/
├── capture_single_image
├── live_preview
└── photo_booth
```

The project requests strict ISO C++20 (`CXX_EXTENSIONS OFF`) and enables common compiler warnings for both the core library and all student-facing applications.

## Configuration

All applications accept one optional positional argument: the TOML configuration filename. When omitted, `config.toml` in the current working directory is used.

The supplied configuration is:

```toml
[camera]
device = 0
width = 1280
height = 720
fps = 30.0
fourcc = ""

[preview]
mirror = false
rotation = 0
window_name = "Photo Booth"

[capture]
save_directory = "captures"
warmup_frames = 10

[processing]
channel_swap_enabled = true
```

Missing values use defaults declared in the typed C++ configuration structures. Unknown sections, unknown options, incorrect value types, and invalid values are reported as errors.

## Run the minimal live preview

```sh
./build/bin/live_preview
```

or:

```sh
./build/bin/live_preview webcam.toml
```

Press `Esc` or `q` to quit.

## Run the photo booth

```sh
./build/bin/photo_booth
```

or:

```sh
./build/bin/photo_booth webcam.toml
```

Controls:

- `i` — toggle image inversion on/off
- `p` — toggle the performance overlay on/off
- `Space` — save the current processed image
- `Esc` or `q` — quit

The performance overlay displays the effective application frame rate in the preview window. The reported frame rate reflects the complete acquisition, processing, analysis, and display loop, so computationally expensive operations may reduce the displayed FPS. The overlay is added only to the preview image and does not modify the processed image.

Captured images are written to the directory specified by capture.save_directory using timestamp-based filenames.                                               

The configured channel-swap baseline operation is applied before the optional inversion operation. Future operations can be added to `processFrame()` in the order desired for the processing pipeline.

## Adding an Operation

The starter project intentionally uses a simple, explicit extension pattern. Processing algorithms are ordinary free functions, while `processFrame()` determines which operations run and in what order. Avoid introducing a class hierarchy or general-purpose processing framework until the growing application provides a clear reason to do so.

### Adding an optional runtime operation

For an operation that the user turns on and off while the application is running:

1. Declare the processing function in `include/photo_booth/ImageProcessing.hpp`.
2. Implement the function in `src/ImageProcessing.cpp`.
3. Add the operation's runtime state to `ProcessingState` in `apps/photo_booth.cpp`.
4. Add a keyboard control in `handleKey()` to toggle or adjust that state.
5. Add the operation to the optional-operations portion of `processFrame()` in the desired pipeline order.

For example, a threshold operation might eventually add:

```cpp
bool threshold_enabled{false};
```

to `ProcessingState`, toggle it from `handleKey()`, and apply `thresholdImage()` from `processFrame()` when the state is enabled. Each optional operation should have independent state so that multiple operations can be active at the same time.

### Adding a baseline configurable operation

For an operation whose initial behavior is established by `config.toml` and applied automatically to every frame when enabled:

1. Declare the processing function in `include/photo_booth/ImageProcessing.hpp`.
2. Implement the function in `src/ImageProcessing.cpp`.
3. Add the operation's enable flag and any parameters to the flat `ProcessingConfig` structure in `include/photo_booth/AppConfig.hpp`.
4. Add matching values to the `[processing]` section of `config.toml`.
5. Add those key names to the processing validation list in `src/AppConfig.cpp`, read the values with `readOptional()`, and add any necessary range/value checks to `validateValues()`.
6. Add the operation to the baseline-operations portion of `processFrame()` in the desired pipeline order.

For example, a future configurable quantization operation could use:

```toml
[processing]
channel_swap_enabled = true
quantization_enabled = false
quantization_levels = 8
```

with a correspondingly simple configuration structure:

```cpp
struct ProcessingConfig {
  bool channel_swap_enabled{false};
  bool quantization_enabled{false};
  int quantization_levels{8};
};
```

The flat `[processing]` section is intentional. It keeps the mechanics of adding a configuration value visible and repetitive, so that most student effort remains focused on the image-processing algorithm itself. If the configuration becomes unwieldy later in the semester, that is an appropriate opportunity to consider refactoring.

## Single-image camera utility

```sh
./build/bin/capture_single_image
```

or:

```sh
./build/bin/capture_single_image laboratory_camera.toml
```

The output filename and camera warmup count are controlled by the `[capture]` section.

All three applications support `--help`.

## ImageCapture component

`ImageCapture` contains no photo-booth user-interface or image-processing behavior. It acquires camera frames and exposes the most recent frame as a `cv::Mat`.

```cpp
#include "photo_booth/ImageCapture.hpp"

#include <iostream>

int main() {
  photo_booth::ImageCapture camera;

  if (!camera.open()) {
    std::cerr << camera.errorMessage() << '\n';
    return 1;
  }

  if (!camera.read()) {
    std::cerr << camera.errorMessage() << '\n';
    return 1;
  }

  const cv::Mat& image = camera.image();

  // image is CV_8UC3 using BGR channel order.
}
```

Camera properties such as image dimensions, frame rate, and FOURCC are requests rather than guarantees. The hardware, operating system, driver, and OpenCV backend may ignore a requested property or select a nearby supported mode. Use `cameraInfo()` after opening the device to inspect the values reported by the backend.

## Suggested evolution during the course

The contrast between `live_preview` and `photo_booth` provides a simple starting lesson in application structure. `live_preview` demonstrates direct use of an acquired frame. `photo_booth` demonstrates how a growing application benefits from an explicit processing pipeline.

As capabilities accumulate, students should continue adding simple processing functions and explicit configuration/runtime state. `processFrame()` and `handleKey()` are expected to become somewhat more crowded as the project grows; that pressure can provide a concrete reason to refactor toward additional classes or modules when those abstractions become useful. Potential additions include quantization, dynamic contrast enhancement, histogram operations, spatial and frequency-domain filtering, sharpening, geometric transformations, perspective correction, artistic filters, feature detection, segmentation, and capture/review behavior.

## License

This project is licensed under the GNU General Public License v3.0. See `LICENSE` for details.

## Contact

### Author

Carl Salvaggio, Ph.D.  
Professor of Imaging Science  
Director, Digital Imaging and Remote Sensing (DIRS) Laboratory

### E-mail

carl.salvaggio@rit.edu

### Organization

Chester F. Carlson Center for Imaging Science  
Rochester Institute of Technology  
Rochester, New York, 14623  
United States
