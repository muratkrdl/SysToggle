set -euo pipefail

export DEVKITPRO=/opt/devkitpro
export DEVKITA64=/opt/devkitpro/devkitA64
export PATH=/opt/devkitpro/portlibs/switch/bin:/opt/devkitpro/tools/bin:/opt/devkitpro/devkitA64/bin:$PATH

SOURCE_DIR="${1:-.}"
BUILD_DIR="${2:-build_switch}"
TARGET="${3:-SysToggle.nro}"

cd "$SOURCE_DIR"
cmake -B "$BUILD_DIR" -G Ninja -DPLATFORM_SWITCH=ON -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" --target "$TARGET"
