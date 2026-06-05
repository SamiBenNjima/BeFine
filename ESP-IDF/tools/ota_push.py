#!/usr/bin/env python3
"""
BeFine OTA Push Tool
Usage: python ota_push.py --port /dev/ttyUSB0 --firmware build/befine.bin
Sends a firmware binary to the device over serial (factory/production line use).
In production, OTA is triggered via BLE write to a dedicated OTA control service.
"""

import argparse
import sys
import os

def parse_args():
    p = argparse.ArgumentParser(description="BeFine OTA firmware pusher")
    p.add_argument("--port",     required=True, help="Serial port (e.g. /dev/ttyUSB0)")
    p.add_argument("--firmware", required=True, help="Path to .bin firmware file")
    p.add_argument("--baud",     default=921600, type=int, help="Baud rate (default 921600)")
    return p.parse_args()

def main():
    args = parse_args()

    if not os.path.isfile(args.firmware):
        print(f"[ERROR] Firmware file not found: {args.firmware}", file=sys.stderr)
        sys.exit(1)

    size = os.path.getsize(args.firmware)
    print(f"[INFO] Firmware: {args.firmware} ({size:,} bytes)")
    print(f"[INFO] Target:   {args.port} @ {args.baud} baud")

    # Delegate to esptool for the actual flashing
    cmd = (
        f"esptool.py --port {args.port} --baud {args.baud} "
        f"--chip esp32s3 "
        f"write_flash 0x210000 {args.firmware}"   # OTA slot app1
    )
    print(f"[CMD] {cmd}")
    ret = os.system(cmd)
    if ret != 0:
        print("[ERROR] esptool failed", file=sys.stderr)
        sys.exit(1)

    print("[OK] OTA firmware written to app1 partition")
    print("[INFO] Reboot the device to boot into the new firmware")

if __name__ == "__main__":
    main()
