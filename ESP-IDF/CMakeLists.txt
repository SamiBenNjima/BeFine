cmake_minimum_required(VERSION 3.16)

# ── BeFine Firmware — ESP-IDF v5.2+ ──────────────────────────────────────────
# Target : ESP32-S3  (set via: idf.py set-target esp32s3)
# Build  : idf.py build
# Flash  : idf.py -p /dev/ttyUSB0 flash monitor
# Tests  : cd test && idf.py build flash monitor
# ─────────────────────────────────────────────────────────────────────────────

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(befine_firmware VERSION 1.0.0)
