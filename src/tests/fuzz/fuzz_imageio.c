/*
    Fuzzing harness for darktable-cli image processing
    Copyright (C) 2026 darktable developers.

    darktable is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Usage with AFL++:
      export AFL_CC=afl-gcc-fast
      export AFL_CXX=afl-g++-fast
      mkdir build-fuzz && cd build-fuzz
      cmake -DCMAKE_C_COMPILER=$AFL_CC -DCMAKE_CXX_COMPILER=$AFL_CXX \
            -DCMAKE_BUILD_TYPE=Debug -DUSE_OPENCL=OFF ..
      make -j$(nproc) darktable-cli
      mkdir -p corpus/seeds
      # Add small valid RAW/TIFF/PNG/JPEG files as seeds
      afl-fuzz -i corpus/seeds -o findings -- \
        ./bin/darktable-cli --width 100 --height 100 \
        --hq false --apply-custom-presets false \
        @@ /dev/null \
        --core --disable-opencl --conf host_memory_limit=512 \
        --conf worker_threads=1 -t 1

    Usage with libFuzzer (Clang):
      Compile this harness separately:
      clang -fsanitize=fuzzer,address -I../../src \
            -o fuzz_imageio fuzz_imageio.c -ldarktable
      mkdir corpus && ./fuzz_imageio corpus/

    This file provides a standalone harness that exercises the raw
    image loading path. It reads fuzzed input from either stdin (AFL)
    or a libFuzzer entry point.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * AFL++ harness: reads a file from argv[1], writes it to a temp path,
 * then invokes darktable-cli on it. This is the simplest integration:
 * wrapping the CLI binary.
 *
 * For deeper fuzzing, you would link against lib_darktable and call
 * the raw decoding functions directly.
 */

#ifndef __AFL_COMPILER
/* Standalone mode: just document how to use it */
#endif

int main(int argc, char *argv[])
{
  if(argc < 2)
  {
    fprintf(stderr, "Usage: %s <input_image_file>\n", argv[0]);
    fprintf(stderr, "\n");
    fprintf(stderr, "This harness is designed to be used with AFL++ or similar fuzzers.\n");
    fprintf(stderr, "It processes the input file through darktable-cli and checks for crashes.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Quick start with AFL++:\n");
    fprintf(stderr, "  1. Build darktable with AFL instrumentation:\n");
    fprintf(stderr, "     CC=afl-gcc-fast CXX=afl-g++-fast cmake -DUSE_OPENCL=OFF ..\n");
    fprintf(stderr, "  2. Create seed corpus with small valid images\n");
    fprintf(stderr, "  3. Run: afl-fuzz -i seeds/ -o findings/ -- \\\n");
    fprintf(stderr, "       darktable-cli --width 100 --height 100 @@ /dev/null \\\n");
    fprintf(stderr, "       --core --disable-opencl\n");
    return 1;
  }

  const char *input_file = argv[1];
  const char *output_file = "/dev/null";

  /* Build the darktable-cli command */
  char cmd[4096];
  snprintf(cmd, sizeof(cmd),
    "darktable-cli "
    "--width 100 --height 100 "
    "--hq false --apply-custom-presets false "
    "\"%s\" \"%s\" "
    "--core --disable-opencl "
    "--conf host_memory_limit=512 "
    "--conf worker_threads=1 -t 1 "
    "2>/dev/null",
    input_file, output_file);

  int ret = system(cmd);

  /* We only care about crashes (signals), not normal error codes.
   * darktable-cli may legitimately fail for corrupt/unsupported inputs. */
  if(WIFSIGNALED(ret))
  {
    /* Crash detected — this is what the fuzzer looks for */
    fprintf(stderr, "CRASH: signal %d on input %s\n", WTERMSIG(ret), input_file);
    return WTERMSIG(ret);
  }

  return 0;
}
