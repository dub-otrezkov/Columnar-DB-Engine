#!/usr/bin/env bash
# Usage: run_query.sh <query_num> <columnar> <output_csv> <log_file>
set -e

if [ $# -lt 4 ]; then
    echo "usage: $0 <query_num> <columnar> <output_csv> <log_file>" >&2
    exit 1
fi

QUERY_NUM="$1"
COLUMNAR="$2"
OUTPUT="$3"
LOGS="$4"

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CLI="$REPO_DIR/build/cli/columnar_cli"

"$CLI" query "$QUERY_NUM" "$COLUMNAR" "$OUTPUT" 2>"$LOGS"
