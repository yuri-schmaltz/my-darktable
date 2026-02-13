# Fuzzing Infrastructure for darktable

This directory contains fuzzing harnesses for testing darktable's image
parsing and processing paths against malformed inputs.

## Harnesses

### `fuzz_imageio.c`
Wraps `darktable-cli` to exercise the full image loading pipeline (RawSpeed,
LibRaw, Exiv2, ImageIO). This catches crashes in the most security-sensitive
code paths.

## Quick Start with AFL++

```bash
# 1. Build darktable with AFL instrumentation
CC=afl-gcc-fast CXX=afl-g++-fast \
  cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_OPENCL=OFF \
  -DCMAKE_INSTALL_PREFIX=$PWD/install ..
make -j$(nproc) && make install

# 2. Create a seed corpus with small valid images
mkdir -p corpus/seeds
# Copy small RAW, TIFF, PNG, JPEG files (< 10KB each) into corpus/seeds/

# 3. Compile the harness
gcc -o fuzz_imageio src/tests/fuzz/fuzz_imageio.c

# 4. Run AFL++
afl-fuzz -i corpus/seeds -o findings -- \
  ./install/bin/darktable-cli \
  --width 100 --height 100 \
  --hq false --apply-custom-presets false \
  @@ /dev/null \
  --core --disable-opencl \
  --conf host_memory_limit=512 \
  --conf worker_threads=1 -t 1
```

## Quick Start with AddressSanitizer (manual)

```bash
# Build with ASan
cmake -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_FLAGS="-fsanitize=address -fno-omit-frame-pointer" \
  -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer" \
  -DUSE_OPENCL=OFF ..
make -j$(nproc) && make install

# Run on a corpus of suspicious files
for f in corpus/crashes/*; do
  ./install/bin/darktable-cli --width 100 --height 100 "$f" /dev/null \
    --core --disable-opencl 2>&1 | tee -a asan_report.txt
done
```

## Triage

Crashes found by the fuzzer will be saved in `findings/crashes/`. For each:

1. Reproduce: `darktable-cli @crash_file /dev/null --core --disable-opencl`
2. Get stack trace: rebuild with `-g -fsanitize=address` and reproduce
3. Minimize: `afl-tmin -i crash_file -o minimized -- darktable-cli @@ /dev/null`
4. Report: file a GitHub issue with the minimized input and stack trace
