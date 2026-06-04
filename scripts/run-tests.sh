#!/usr/bin/env bash
#
# Build the amnezia-client against this repo's test suite and run ctest.
#
# Strategy (mirrors upstream PR #2550's "get sources" model): clone the client,
# drop our tests/ into client/client/tests/, hook them into the client build,
# configure with the client's normal conan+cmake flow, build, then ctest.
#
# Env knobs (all optional):
#   CLIENT_REPO   git URL of the client          (default: coffeegrind123/amnezia-client)
#   CLIENT_REF    branch/tag/sha to test against  (default: dev)
#   BUILD_TYPE    CMake build type                (default: Debug)
#   GENERATOR     CMake generator                 (default: Ninja)
#   WORK          scratch dir                     (default: ./.work)
#   QT_TOOLCHAIN  path to Qt's qt.toolchain.cmake (optional; else CMAKE_PREFIX_PATH/auto)
#
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CLIENT_REPO="${CLIENT_REPO:-https://github.com/coffeegrind123/amnezia-client.git}"
CLIENT_REF="${CLIENT_REF:-dev}"
BUILD_TYPE="${BUILD_TYPE:-Debug}"
GENERATOR="${GENERATOR:-Ninja}"
WORK="${WORK:-$REPO_ROOT/.work}"
CLIENT_DIR="$WORK/amnezia-client"
BUILD_DIR="$WORK/build"

echo "==> cleaning $WORK"
rm -rf "$WORK"; mkdir -p "$WORK"

echo "==> cloning $CLIENT_REPO @ $CLIENT_REF"
git clone --depth 1 --branch "$CLIENT_REF" --recurse-submodules --shallow-submodules \
    "$CLIENT_REPO" "$CLIENT_DIR"

echo "==> injecting test suite into client/client/tests"
rm -rf "$CLIENT_DIR/client/tests"
cp -R "$REPO_ROOT/tests" "$CLIENT_DIR/client/tests"
python3 "$REPO_ROOT/scripts/inject_tests.py" "$CLIENT_DIR/client/CMakeLists.txt"

echo "==> configuring ($GENERATOR, $BUILD_TYPE)"
cmake_args=(-S "$CLIENT_DIR" -B "$BUILD_DIR" -G "$GENERATOR" "-DCMAKE_BUILD_TYPE=$BUILD_TYPE")
if [[ -n "${QT_TOOLCHAIN:-}" ]]; then
    cmake_args+=("-DCMAKE_TOOLCHAIN_FILE=$QT_TOOLCHAIN")
elif [[ -n "${CMAKE_PREFIX_PATH:-}" ]]; then
    cmake_args+=("-DCMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH")
fi
cmake "${cmake_args[@]}"

echo "==> building test targets"
# Build only the test executables (+ their test_common object lib), not the
# full app/installer. ctest discovers what was registered via add_test().
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" \
    --target test_common 2>/dev/null || true
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE"

echo "==> running ctest"
ctest --test-dir "$BUILD_DIR/client/tests" --build-config "$BUILD_TYPE" --output-on-failure
