set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
OUT="$ROOT/build_tests"
mkdir -p "$OUT"

g++ -std=c++17 -Wall -Wextra -O1 \
    -I"$ROOT/app" \
    -I"$ROOT/library/borealis/library/include" \
    "$ROOT"/app/core/*.cpp \
    "$ROOT"/tests/CoreTests.cpp \
    -o "$OUT/core_tests"

"$OUT/core_tests" "$OUT/fixture" "${1:-$ROOT/tests/data/sample_sysmodule.nsp}"
