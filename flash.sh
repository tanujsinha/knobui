#!/bin/zsh
# Flash the ESP32-S3 Knob project to the connected device
set -e

PORT="${1:-/dev/cu.usbmodem1101}"
BAUD="${2:-460800}"

echo "=== Setting up ESP-IDF environment ==="
export PATH="/opt/homebrew/opt/python@3.11/libexec/bin:$PATH"
source ~/esp/esp-idf/export.sh > /dev/null 2>&1
export PATH="$HOME/.espressif/python_env/idf5.3_py3.9_env/bin:$PATH"

echo "=== Flashing to $PORT at ${BAUD} baud ==="
cd "$(dirname "$0")"
idf.py -p "$PORT" -b "$BAUD" flash

echo ""
echo "=== Flash complete! Device is rebooting. ==="
echo "Run: idf.py -p $PORT monitor   (to view serial output)"
