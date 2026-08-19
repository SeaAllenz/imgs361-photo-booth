#!/usr/bin/env bash

# ---------------------------------------------------------------------------
# IMGS.361 Photo Booth - Virtual Camera
#
# Stops the FFmpeg process started by start_camera.sh.
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

PID_FILE="${SCRIPT_DIR}/.virtual_camera.pid"

if [[ ! -f "${PID_FILE}" ]]; then
    echo "Virtual camera is not running."
    exit 0
fi

PID=$(cat "${PID_FILE}")

if kill -0 "${PID}" 2>/dev/null; then

    echo "Stopping IMGS.361 virtual camera (PID ${PID})..."

    kill "${PID}"

    # Wait briefly for FFmpeg to exit cleanly.
    for _ in {1..20}; do
        if ! kill -0 "${PID}" 2>/dev/null; then
            break
        fi
        sleep 0.1
    done

    # Force termination if necessary.
    if kill -0 "${PID}" 2>/dev/null; then
        echo "FFmpeg did not exit cleanly; forcing termination."
        kill -9 "${PID}"
    fi

    echo "Virtual camera stopped."

else
    echo "Virtual camera process ${PID} is no longer running."
fi

rm -f "${PID_FILE}"
