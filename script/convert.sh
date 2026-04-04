#!/usr/bin/env bash
# Usage: convert.sh <input_csv> <input_schema> <output_columnar>
set -e

if [ $# -lt 3 ]; then
    echo "usage: $0 <input_csv> <input_schema> <output_columnar>" >&2
    exit 1
fi

INPUT_CSV="$1"
INPUT_SCHEMA="$2"
COLUMNAR="$3"

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CLI="$REPO_DIR/build/cli/columnar_cli"

exec "$CLI" convert "$INPUT_CSV" "$INPUT_SCHEMA" "$COLUMNAR"
