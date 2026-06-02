#!/bin/bash
emcc main.c raytracing.c raycasting.c input.c -o index.html \
  --shell-file shell.html \
  -s USE_SDL=2 \
  -s WASM=1 \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s EXPORTED_FUNCTIONS='["_main","_set_touch_rotation"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall"]'
