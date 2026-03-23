#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VERSION_HEADER="$ROOT_DIR/source/ESPEED32/slot_ESC.h"
MANIFEST_PATH="$ROOT_DIR/source/ESPEED32/data/ui/release.json"

if [[ ! -f "$VERSION_HEADER" ]]; then
  echo "[SPIFFS] Missing version header: $VERSION_HEADER" >&2
  exit 1
fi

major="$(awk '/^#define[[:space:]]+SW_MAJOR_VERSION[[:space:]]+[0-9]+/{print $3; exit}' "$VERSION_HEADER")"
minor="$(awk '/^#define[[:space:]]+SW_MINOR_VERSION[[:space:]]+[0-9]+/{print $3; exit}' "$VERSION_HEADER")"

if [[ -z "$major" || -z "$minor" ]]; then
  echo "[SPIFFS] Could not parse firmware version from $VERSION_HEADER" >&2
  exit 1
fi

release="$major.$minor"
mkdir -p "$(dirname "$MANIFEST_PATH")"

tmp_file="$(mktemp)"
printf '{\n  "spiffsRelease": "%s"\n}\n' "$release" > "$tmp_file"

if [[ ! -f "$MANIFEST_PATH" ]] || ! cmp -s "$tmp_file" "$MANIFEST_PATH"; then
  mv "$tmp_file" "$MANIFEST_PATH"
  echo "[SPIFFS] Updated release manifest: $release"
else
  rm -f "$tmp_file"
  echo "[SPIFFS] Release manifest already current: $release"
fi
