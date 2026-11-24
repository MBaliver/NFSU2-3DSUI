#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PROJECT_DIR="$ROOT/3DS Files"
BUILD_MOUNT="$ROOT/build_3ds"
TOOLS_BIN="$ROOT/tools/3ds-bin"

export DEVKITPRO="${DEVKITPRO:-$HOME/.local/share/devkitpro/opt/devkitpro}"
export DEVKITARM="${DEVKITARM:-$DEVKITPRO/devkitARM}"
export PATH="$DEVKITPRO/tools/bin:$DEVKITARM/bin:$PATH"

PYTHON="$ROOT/.venv/bin/python"
if [[ ! -x "$PYTHON" ]]; then
    PYTHON="$(command -v python3 || true)"
fi

if [[ -n "$PYTHON" ]]; then
    "$PYTHON" "$ROOT/tools/pica_output_check.py"
fi

touch "$PROJECT_DIR/source/buildstamp.c"
mkdir -p "$BUILD_MOUNT"

unshare -rm bash -c "
    mount --bind \"$PROJECT_DIR\" \"$BUILD_MOUNT\"
    cd \"$BUILD_MOUNT\"
    make -j\$(nproc) cia MAKEROM=\"$TOOLS_BIN/makerom\" BANNERTOOL=\"$TOOLS_BIN/bannertool\"
"

ls -lh "$PROJECT_DIR/NFSU2-3DS.3dsx" "$PROJECT_DIR/NFSU2-3DS.cia"

