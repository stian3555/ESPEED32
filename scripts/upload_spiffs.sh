#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DATA_DIR="$ROOT_DIR/source/ESPEED32/data"
BUILD_DIR="$ROOT_DIR/build"
IMAGE_PATH="$BUILD_DIR/espeed32_spiffs.bin"
ARDUINO_JSON="$ROOT_DIR/.vscode/arduino.json"

PARTITION_OFFSET="0x290000"
PARTITION_SIZE="0x160000"
BLOCK_SIZE="4096"
PAGE_SIZE="256"

PORT=""
BAUD=""
BUILD_ONLY=0
PURGE=0
ASSUME_YES=0

usage() {
  cat <<USAGE
Usage: $(basename "$0") [--build-only] [--purge] [--yes] [-p PORT] [-u BAUD]

Options:
  --build-only      Build SPIFFS image only (no upload)
  --purge           Erase SPIFFS partition only (no build, no upload)
  -y, --yes         Skip interactive confirmation for --purge
  -p, --port PORT   Serial port override (default: read from .vscode/arduino.json)
  -u, --baud BAUD   Upload baud override (default: UploadSpeed from .vscode/arduino.json)
  -h, --help        Show this help
USAGE
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-only)
      BUILD_ONLY=1
      shift
      ;;
    --purge)
      PURGE=1
      shift
      ;;
    -y|--yes)
      ASSUME_YES=1
      shift
      ;;
    -p|--port)
      [[ $# -ge 2 ]] || { echo "Missing value for $1" >&2; exit 1; }
      PORT="$2"
      shift 2
      ;;
    -u|--baud)
      [[ $# -ge 2 ]] || { echo "Missing value for $1" >&2; exit 1; }
      BAUD="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown argument: $1" >&2
      usage
      exit 1
      ;;
  esac
done

if [[ "$PURGE" -eq 1 && "$BUILD_ONLY" -eq 1 ]]; then
  echo "Cannot combine --purge and --build-only." >&2
  exit 1
fi

if [[ "$PURGE" -eq 0 && ! -d "$DATA_DIR" ]]; then
  echo "Data directory not found: $DATA_DIR" >&2
  exit 1
fi

if [[ -z "$PORT" && -f "$ARDUINO_JSON" ]]; then
  PORT="$(sed -n 's/.*"port"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p' "$ARDUINO_JSON" | head -n 1)"
fi

if [[ -z "$PORT" ]]; then
  PORT="/dev/tty.usbserial-120"
fi

if [[ ! -e "$PORT" ]]; then
  ALT_PORT="$(ls -1 /dev/tty.usbserial* /dev/cu.usbserial* 2>/dev/null | head -n 1 || true)"
  if [[ -n "$ALT_PORT" ]]; then
    echo "[SPIFFS] Port $PORT not found, using $ALT_PORT"
    PORT="$ALT_PORT"
  fi
fi

if [[ -z "$BAUD" && -f "$ARDUINO_JSON" ]]; then
  BAUD="$(sed -n 's/.*UploadSpeed=\([0-9][0-9]*\).*/\1/p' "$ARDUINO_JSON" | head -n 1)"
fi

if [[ -z "$BAUD" ]]; then
  BAUD="115200"
fi

MKSPiffs_DIR="$(ls -d "$HOME/Library/Arduino15/packages/esp32/tools/mkspiffs/"* 2>/dev/null | sort -V | tail -n 1 || true)"
ESPTOOL_DIR="$(ls -d "$HOME/Library/Arduino15/packages/esp32/tools/esptool_py/"* 2>/dev/null | sort -V | tail -n 1 || true)"

MKSPiffs_BIN="$MKSPiffs_DIR/mkspiffs"
ESPTOOL_BIN="$ESPTOOL_DIR/esptool"

if [[ "$PURGE" -eq 0 && ! -x "$MKSPiffs_BIN" ]]; then
  echo "mkspiffs not found. Install ESP32 platform tools first." >&2
  exit 1
fi

if [[ ! -x "$ESPTOOL_BIN" && ( "$BUILD_ONLY" -eq 0 || "$PURGE" -eq 1 ) ]]; then
  echo "esptool not found. Install ESP32 platform tools first." >&2
  exit 1
fi

if [[ ( "$BUILD_ONLY" -eq 0 || "$PURGE" -eq 1 ) && ! -e "$PORT" ]]; then
  echo "Serial port not found: $PORT" >&2
  echo "Tip: run with -p /dev/tty.usbserial-XXXX or /dev/cu.usbserial-XXXX" >&2
  exit 1
fi

if [[ "$PURGE" -eq 1 ]]; then
  if [[ "$ASSUME_YES" -eq 0 ]]; then
    echo "[SPIFFS] WARNING: This will erase the SPIFFS partition."
    echo "[SPIFFS] Partition: offset $PARTITION_OFFSET, size $PARTITION_SIZE"
    echo "[SPIFFS] Port: $PORT (baud $BAUD)"
    read -r -p "[SPIFFS] Type PURGE to continue: " confirm
    if [[ "$confirm" != "PURGE" ]]; then
      echo "[SPIFFS] Purge aborted."
      exit 1
    fi
  fi

  echo "[SPIFFS] Erasing partition at $PARTITION_OFFSET (size $PARTITION_SIZE)"
  "$ESPTOOL_BIN" --chip esp32 --port "$PORT" --baud "$BAUD" --before default-reset --after hard-reset erase-region "$PARTITION_OFFSET" "$PARTITION_SIZE"
  echo "[SPIFFS] Purge complete"
  exit 0
fi

mkdir -p "$BUILD_DIR"

echo "[SPIFFS] Building image from $DATA_DIR"
"$MKSPiffs_BIN" -c "$DATA_DIR" -b "$BLOCK_SIZE" -p "$PAGE_SIZE" -s "$PARTITION_SIZE" "$IMAGE_PATH"

echo "[SPIFFS] Image ready: $IMAGE_PATH"

if [[ "$BUILD_ONLY" -eq 1 ]]; then
  exit 0
fi

echo "[SPIFFS] Uploading to $PORT at offset $PARTITION_OFFSET (baud $BAUD)"
"$ESPTOOL_BIN" --chip esp32 --port "$PORT" --baud "$BAUD" --before default-reset --after hard-reset write-flash "$PARTITION_OFFSET" "$IMAGE_PATH"

echo "[SPIFFS] Upload complete"
