#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DOCS_SRC_DIR="$ROOT_DIR/source/ESPEED32/docs_src"
DOCS_BUILD_SCRIPT="$ROOT_DIR/scripts/build_docs.py"

if [[ ! -d "$DOCS_SRC_DIR" ]]; then
  exit 0
fi

if [[ ! -f "$DOCS_BUILD_SCRIPT" ]]; then
  echo "[DOCS] Missing build script: $DOCS_BUILD_SCRIPT" >&2
  exit 1
fi

if ! command -v python3 >/dev/null 2>&1; then
  echo "[DOCS] python3 not found; cannot refresh generated docs." >&2
  exit 1
fi

echo "[DOCS] Checking generator syntax"
python3 -B -m py_compile "$DOCS_BUILD_SCRIPT"

echo "[DOCS] Refreshing generated docs"
python3 "$DOCS_BUILD_SCRIPT"
