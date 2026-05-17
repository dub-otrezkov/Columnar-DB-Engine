#!/usr/bin/env bash
# Usage: convert.sh <input_csv> <output_columnar>
set -e

if [ $# -lt 2 ]; then
    echo "usage: $0 <input_csv> <output_columnar>" >&2
    exit 1
fi

INPUT_CSV="$1"
COLUMNAR="$2"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
SCHEMA="$SCRIPT_DIR/scheme.csv"
CLI="$REPO_DIR/build/cli/columnar_cli"

exec "$CLI" convert "$INPUT_CSV" "$SCHEMA" "$COLUMNAR"
