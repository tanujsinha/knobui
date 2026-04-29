#!/bin/zsh
# Build the ESP32-S3 Knob project
set -e

echo "=== Setting up ESP-IDF environment ==="
export PATH="/opt/homebrew/opt/python@3.11/libexec/bin:$PATH"
source ~/esp/esp-idf/export.sh > /dev/null 2>&1
export PATH="$HOME/.espressif/python_env/idf5.3_py3.9_env/bin:$PATH"

echo "=== Building project ==="
cd "$(dirname "$0")"
idf.py build

echo ""
echo "=== Build complete! ==="
echo "Run ./flash.sh to flash to your device."
