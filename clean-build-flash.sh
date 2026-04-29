#!/bin/zsh
# Full clean, rebuild, and flash for the ESP32-S3 Knob project
set -e

PORT="${1:-/dev/cu.usbmodem1101}"
BAUD="${2:-460800}"

echo "=== Setting up ESP-IDF environment ==="
export PATH="$HOME/.espressif/python_env/idf5.3_py3.9_env/bin:$PATH"
source ~/esp/esp-idf/export.sh > /dev/null 2>&1 || true
export IDF_PYTHON_ENV_PATH="$HOME/.espressif/python_env/idf5.3_py3.9_env"

cd "$(dirname "$0")"

echo "=== Cleaning managed components ==="
rm -rf managed_components/

echo "=== Full clean ==="
idf.py fullclean

echo "=== Building ==="
idf.py build

echo "=== Flashing to $PORT at ${BAUD} baud ==="
idf.py -p "$PORT" -b "$BAUD" flash

echo ""
echo "=== Done! Device is running new firmware. ==="
