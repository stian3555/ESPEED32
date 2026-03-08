#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ARDUINO_JSON="$ROOT_DIR/.vscode/arduino.json"
SPIFFS_SCRIPT="$ROOT_DIR/scripts/upload_spiffs.sh"

PORT=""
BAUD=""
FIRMWARE_ONLY=0
SPIFFS_ONLY=0
COMPILE_ONLY=0

usage() {
  cat <<USAGE
Usage: $(basename "$0") [--firmware-only] [--spiffs-only] [--compile-only] [-p PORT] [-u BAUD]

Default flow:
  1) Compile firmware
  2) Upload firmware
  3) Upload SPIFFS image (calls scripts/upload_spiffs.sh)

Options:
  --firmware-only    Compile + upload firmware, skip SPIFFS upload
  --spiffs-only      Only upload SPIFFS image (skip firmware compile/upload)
  --compile-only     Compile firmware only
  -p, --port PORT    Serial port override (default: read from .vscode/arduino.json)
  -u, --baud BAUD    SPIFFS upload baud override (default: UploadSpeed from .vscode/arduino.json)
  -h, --help         Show this help
USAGE
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --firmware-only)
      FIRMWARE_ONLY=1
      shift
      ;;
    --spiffs-only)
      SPIFFS_ONLY=1
      shift
      ;;
    --compile-only)
      COMPILE_ONLY=1
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

if [[ "$SPIFFS_ONLY" -eq 1 && ( "$FIRMWARE_ONLY" -eq 1 || "$COMPILE_ONLY" -eq 1 ) ]]; then
  echo "Cannot combine --spiffs-only with firmware-related modes." >&2
  exit 1
fi

if [[ "$COMPILE_ONLY" -eq 1 && "$FIRMWARE_ONLY" -eq 1 ]]; then
  echo "Cannot combine --compile-only and --firmware-only." >&2
  exit 1
fi

if [[ ! -f "$ARDUINO_JSON" ]]; then
  echo "Missing $ARDUINO_JSON" >&2
  exit 1
fi

if [[ ! -x "$SPIFFS_SCRIPT" ]]; then
  echo "Missing executable SPIFFS script: $SPIFFS_SCRIPT" >&2
  exit 1
fi

if ! command -v arduino-cli >/dev/null 2>&1; then
  echo "arduino-cli not found in PATH." >&2
  exit 1
fi

json_value() {
  local key="$1"
  sed -n "s/.*\"$key\"[[:space:]]*:[[:space:]]*\"\\([^\"]*\\)\".*/\\1/p" "$ARDUINO_JSON" | head -n 1
}

run_filtered_cmd() {
  local rc
  set +e
  "$@" 2>&1 | sed \
    -e '/esp_bt\.h:16,/d' \
    -e '/esp32-hal-bt\.c:30:/d' \
    -e '/esp32-hal-misc\.c:29:/d' \
    -e '/esp_bredr_cfg\.h:18:9: note:.*BT: forcing BR\/EDR max sync conn eff to 1/d' \
    -e '/#pragma message ("BT: forcing BR\/EDR max sync conn eff to 1 (Bluedroid HFP requires SCO\/eSCO)")/d' \
    -e '/^[[:space:]]*\|[[:space:]]\+\^~~~~~~$/d'
  rc=${PIPESTATUS[0]}
  set -e
  return "$rc"
}

BOARD="$(json_value board)"
BOARD_OPTIONS="$(json_value configuration)"
SKETCH_REL="$(json_value sketch)"
OUTPUT_REL="$(json_value output)"

if [[ -z "$BOARD" || -z "$SKETCH_REL" || -z "$OUTPUT_REL" ]]; then
  echo "Invalid $ARDUINO_JSON. Expected keys: board, sketch, output." >&2
  exit 1
fi

if [[ "$SKETCH_REL" = /* ]]; then
  SKETCH_PATH="$SKETCH_REL"
else
  SKETCH_PATH="$ROOT_DIR/$SKETCH_REL"
fi
SKETCH_DIR="$(cd "$(dirname "$SKETCH_PATH")" && pwd)"

if [[ "$OUTPUT_REL" = /* ]]; then
  BUILD_DIR="$OUTPUT_REL"
else
  BUILD_DIR="$ROOT_DIR/$OUTPUT_REL"
fi

if [[ ! -f "$SKETCH_PATH" ]]; then
  echo "Sketch not found: $SKETCH_PATH" >&2
  exit 1
fi

if [[ -z "$PORT" ]]; then
  PORT="$(json_value port)"
fi
if [[ -z "$PORT" ]]; then
  PORT="/dev/tty.usbserial-120"
fi

if [[ ! -e "$PORT" ]]; then
  ALT_PORT="$(ls -1 /dev/tty.usbserial* /dev/cu.usbserial* 2>/dev/null | head -n 1 || true)"
  if [[ -n "$ALT_PORT" ]]; then
    echo "[FLASH] Port $PORT not found, using $ALT_PORT"
    PORT="$ALT_PORT"
  fi
fi

if [[ -z "$BAUD" ]]; then
  BAUD="$(sed -n 's/.*UploadSpeed=\([0-9][0-9]*\).*/\1/p' "$ARDUINO_JSON" | head -n 1)"
fi
if [[ -z "$BAUD" ]]; then
  BAUD="115200"
fi

if [[ "$SPIFFS_ONLY" -eq 0 && "$COMPILE_ONLY" -eq 0 && ! -e "$PORT" ]]; then
  echo "Serial port not found: $PORT" >&2
  echo "Tip: run with -p /dev/tty.usbserial-XXXX or /dev/cu.usbserial-XXXX" >&2
  exit 1
fi

if [[ "$SPIFFS_ONLY" -eq 0 ]]; then
  mkdir -p "$BUILD_DIR"

  echo "[FLASH] Compiling firmware"
  compile_cmd=(arduino-cli compile --fqbn "$BOARD" --build-path "$BUILD_DIR")
  if [[ -n "$BOARD_OPTIONS" ]]; then
    compile_cmd+=(--board-options "$BOARD_OPTIONS")
  fi
  compile_cmd+=("$SKETCH_DIR")
  run_filtered_cmd "${compile_cmd[@]}"

  if [[ "$COMPILE_ONLY" -eq 1 ]]; then
    echo "[FLASH] Compile complete"
    exit 0
  fi

  echo "[FLASH] Uploading firmware to $PORT"
  upload_cmd=(arduino-cli upload --fqbn "$BOARD" --port "$PORT" --build-path "$BUILD_DIR")
  if [[ -n "$BOARD_OPTIONS" ]]; then
    upload_cmd+=(--board-options "$BOARD_OPTIONS")
  fi
  upload_cmd+=("$SKETCH_DIR")
  "${upload_cmd[@]}"

  echo "[FLASH] Firmware upload complete"
fi

if [[ "$FIRMWARE_ONLY" -eq 1 ]]; then
  exit 0
fi

echo "[FLASH] Uploading SPIFFS image"
"$SPIFFS_SCRIPT" -p "$PORT" -u "$BAUD"

echo "[FLASH] All done"
