#!/bin/zsh
# Build and flash the ESP32-S3 Knob project in one step
set -e

PORT="${1:-/dev/cu.usbmodem1101}"

echo "=== Setting up ESP-IDF environment ==="
export PATH="/opt/homebrew/opt/python@3.11/libexec/bin:$PATH"
source ~/esp/esp-idf/export.sh > /dev/null 2>&1
export PATH="$HOME/.espressif/python_env/idf5.3_py3.9_env/bin:$PATH"

cd "$(dirname "$0")"

echo "=== Building project ==="
idf.py build

echo ""
echo "=== Flashing to $PORT ==="
idf.py -p "$PORT" flash

echo ""
echo "=== Done! Device is rebooting. ==="
echo "Run: idf.py -p $PORT monitor   (to view serial output)"
