#!/usr/bin/bash

set -euo pipefail

FFMPEG_CROSS_COMPILE_COMPONENTS='
    --disable-x86asm
    --disable-asm
	--target-os=none 
	--arch=x86_32 
	--enable-cross-compile 
	--nm=emnm
	--ar=emar 
	--ranlib=emranlib
	--cc=emcc 
	--cxx=em++ 
	--objcc=emcc 
	--dep-cc=emcc
'

FFMPEG_COMMON_COMPONENTS='
    --disable-doc
    --disable-debug
    --disable-runtime-cpudetect
    --disable-pthreads
    --disable-w32threads
    --disable-os2threads
    --disable-autodetect
    --disable-programs
    --enable-protocol=file
    --enable-demuxer=hevc 
    --enable-demuxer=h264 
    --enable-demuxer=pcm_s16le 
    --enable-demuxer=timestamp_inserter 
    --enable-demuxer=mpegts 
    --enable-parser=h264 
    --enable-parser=hevc 
    --enable-decoder=pcm_s16le 
    --enable-decoder=mp2 
    --enable-decoder=pcm_alaw 
    --enable-filter=aresample 
    --enable-bsf=extract_extradata
'

if [[ "$1" == "clean" ]]; then
    cd ./ffmpeg && emmake make clean
fi

if [[ "$1" == "configure" ]]; then
    cd ./ffmpeg && emconfigure ./configure $FFMPEG_COMMON_COMPONENTS $FFMPEG_CROSS_COMPILE_COMPONENTS
fi

function build_app () {
    cd app; em++ -c app.cpp; cd ..
}

if [[ "$1" == "app" ]]; then
    build_app
fi

if [[ "$1" == "build" ]]; then
    build_app
    cd ./ffmpeg && emmake make -j4
fi

function install() {
    mkdir -p ../out; cp *.wasm* ../out
}

if [[ "$1" == "link" ]]; then
    cd ./ffmpeg &&
        emcc -O3 -sWASM=2 --no-entry \
            -o libffmpeg.wasm \
            libavcodec/*.o libavformat/*.o libavutil/*.o libswscale/*.o ../app/app.o &&
        install
fi

if [[ "$1" == "link2" ]]; then # link some, need prefix _
    cd ./ffmpeg &&
        emcc -O3 -sWASM=2 --no-entry \
            -o libffmpeg.wasm \
            libavcodec/*.o libavformat/*.o libavutil/*.o libswscale/*.o ../app/app.o \
            -s EXPORTED_FUNCTIONS='["_web_test_function"]' &&
        install
fi

if [[ "$1" == "link3" ]]; then # link ALL
    cd ./ffmpeg &&
        emcc -O3 -sWASM=2 --no-entry \
            -o libffmpeg.wasm \
            libavcodec/*.o libavformat/*.o libavutil/*.o libswscale/*.o \
            -s LINKABLE=1 -s EXPORT_ALL=1 &&
        install
fi

exit 0

cd ./ffmpeg && \
    # emmake make clean && \
    # emconfigure ./configure $FFMPEG_COMMON_COMPONENTS $FFMPEG_CROSS_COMPILE_COMPONENTS && \
    emmake make -j && \
    emcc -O3 -sWASM=2 --no-entry -o libffmpeg.wasm libavcodec/*.o libavformat/*.o libavutil/*.o libswscale/*.o
