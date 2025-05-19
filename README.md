# player-ffmpeg-wasm

make ffmpeg_configure
make ffmpeg_build
make ffmpeg_install

make sdl_configure
make sdl_build
make sdl_install

bear -- make tap_build # compile_commands.json


## crosscompile
IS_WASM=Y make ffmpeg_clean
IS_WASM=Y make ffmpeg_install
IS_WASM=Y bear -- make wasm_build # compile_commands.json

## last successfull variant
1. debug chrome: EMCC_DEBUG=1 IS_WASM=Y make wasm_build4
2. prod firefox: IS_WASM=Y make wasm_build4 && make wasm_run