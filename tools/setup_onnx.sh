#!/bin/bash
#
# setup_onnx.sh - Download and Setup ONNX Runtime for darktable AI Core
#
# This script downloads a pre-compiled ONNX Runtime binary (Linux x64)
# and extracts it to ./external/onnxruntime, so that CMake can find it.
#

set -e

ONNX_VERSION="1.17.1"
OS="linux"
ARCH="x64"
BASE_URL="https://github.com/microsoft/onnxruntime/releases/download/v${ONNX_VERSION}"
FILENAME="onnxruntime-${OS}-${ARCH}-${ONNX_VERSION}.tgz"
DEST_DIR="external/onnxruntime"

echo "=== darktable AI Core Setup ==="
echo "Downloading ONNX Runtime v${ONNX_VERSION}..."

if [ -d "$DEST_DIR" ]; then
    echo "Directory $DEST_DIR already exists. Skipping download."
    echo "To force redownload, remove the directory."
    exit 0
fi

# Create temp dir
TMP_DIR=$(mktemp -d)
trap "rm -rf $TMP_DIR" EXIT

cd "$TMP_DIR"
wget -q --show-progress "${BASE_URL}/${FILENAME}"

echo "Extracting..."
tar -xf "$FILENAME"

# Move to destination
cd - > /dev/null
mkdir -p $(dirname "$DEST_DIR")
mv "$TMP_DIR/onnxruntime-${OS}-${ARCH}-${ONNX_VERSION}" "$DEST_DIR"

echo "Done."
echo "ONNX Runtime installed to: $DEST_DIR"
echo ""
echo "To build with AI Core enabled:"
echo "  mkdir build && cd build"
echo "  cmake -DUSE_ONNX=ON -DONNXRuntime_ROOT=$PWD/../$DEST_DIR .."
echo "  make"
