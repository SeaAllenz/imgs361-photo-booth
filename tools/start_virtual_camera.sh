#!/usr/bin/env bash

# ---------------------------------------------------------------------------
# IMGS.361 Photo Booth - Virtual Camera
#
# Feeds an MP4 video file into a v4l2loopback virtual camera.
# ---------------------------------------------------------------------------

set -e

# Directory containing this script.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

VIDEO_FILE="${SCRIPT_DIR}/female_model_2_720p.mp4"
VIDEO_DEVICE="/dev/video10"
PID_FILE="${SCRIPT_DIR}/.virtual_camera.pid"
LOG_FILE="${SCRIPT_DIR}/.virtual_camera.log"

# ---------------------------------------------------------------------------
# Check prerequisites.
# ---------------------------------------------------------------------------

if ! command -v ffmpeg >/dev/null 2>&1; then
    echo "ERROR: ffmpeg is not installed."
    exit 1
fi

if [[ ! -e "${VIDEO_DEVICE}" ]]; then
    echo "ERROR: Virtual camera ${VIDEO_DEVICE} does not exist."
    echo "       The v4l2loopback device must be created first."
    exit 1
fi

if [[ ! -f "${VIDEO_FILE}" ]]; then
    echo "ERROR: Video file not found:"
    echo "       ${VIDEO_FILE}"
    exit 1
fi

# ---------------------------------------------------------------------------
# Check whether the virtual camera is already running.
# ---------------------------------------------------------------------------

if [[ -f "${PID_FILE}" ]]; then
    PID=$(cat "${PID_FILE}")

    if kill -0 "${PID}" 2>/dev/null; then
        echo "Virtual camera is already running (PID ${PID})."
        exit 0
    else
        # Stale PID file.
        rm -f "${PID_FILE}"
    fi
fi

# ---------------------------------------------------------------------------
# Start FFmpeg.
# ---------------------------------------------------------------------------

echo "Starting IMGS.361 virtual camera..."
echo
echo "Video:  ${VIDEO_FILE}"
echo "Device: ${VIDEO_DEVICE}"
echo

ffmpeg \
    -loglevel error \
    -re \
    -stream_loop -1 \
    -i "${VIDEO_FILE}" \
    -r 30 \
    -c:v mjpeg \
    -pix_fmt yuvj420p \
    -q:v 2 \
    -f v4l2 \
    "${VIDEO_DEVICE}" \
    >"${LOG_FILE}" 2>&1 &

PID=$!

echo "${PID}" > "${PID_FILE}"

# Give FFmpeg a moment to start.
sleep 1

# Make sure it is still running.
if ! kill -0 "${PID}" 2>/dev/null; then
    echo "ERROR: Virtual camera failed to start."
    echo
    echo "See:"
    echo "    ${LOG_FILE}"
    rm -f "${PID_FILE}"
    exit 1
fi

echo "Virtual camera started."
echo
echo "Camera device: ${VIDEO_DEVICE}"
echo "Camera index:  10"
echo "Resolution:    1280 x 720"
echo "Frame rate:    30 fps"
echo "Process ID:    ${PID}"
