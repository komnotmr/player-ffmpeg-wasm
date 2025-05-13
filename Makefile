### COMMON

export root_dir=$(shell pwd)

export COMMON_CFLAGS := -Os

ifeq ($(IS_WASM),Y)
	CONFIGURE := emconfigure
	MAKE := emmake make
	CC := emcc
	CXX := em++
	RANLIB := emranlib
endif

export target

.phony: dump
dump:
	echo "target=$(target)"

### FFMPEG

FFMPEG_COMMON_COMPONENTS := --disable-doc \
    --disable-debug\
    --disable-runtime-cpudetect\
    --disable-pthreads\
    --disable-w32threads\
    --disable-os2threads\
    --disable-autodetect\
    --disable-programs\
    --disable-encoder=libopus\
    --enable-protocol=file\
    --enable-demuxer=hevc \
    --enable-demuxer=h264 \
    --enable-demuxer=pcm_s16le \
    --enable-demuxer=timestamp_inserter \
    --enable-demuxer=mpegts \
    --enable-parser=h264 \
    --enable-parser=hevc \
    --enable-decoder=pcm_s16le \
    --enable-decoder=mp2 \
    --enable-decoder=pcm_alaw \
    --enable-bsf=extract_extradata

FFMPEG_CROSS_COMPILE_COMPONENTS :=

ifeq ($(IS_WASM),Y)
	FFMPEG_CROSS_COMPILE_COMPONENTS := --disable-x86asm \
    --disable-asm \
	--target-os=none  \
	--arch=x86_32  \
	--enable-cross-compile  \
	--nm=emnm \
	--ar=emar  \
	--ranlib=emranlib \
	--cc=emcc  \
	--cxx=em++  \
	--objcc=emcc  \
	--dep-cc=emcc
endif

export ffmpeg_dir:=$(root_dir)/ffmpeg

export ffmpeg_out:=$(root_dir)/out
export ffmpeg_inc:=$(ffmpeg_out)/include
export ffmpeg_lib:=$(ffmpeg_out)/lib

.phony: ffmpeg_configure
ffmpeg_configure: $(ffmpeg_dir) ffmpeg_clean
	cd $(ffmpeg_dir) && $(CONFIGURE) ./configure --extra-cflags=$(COMMON_CFLAGS) $(FFMPEG_COMMON_COMPONENTS) $(FFMPEG_CROSS_COMPILE_COMPONENTS) --prefix=$(ffmpeg_out)

.phony: ffmpeg_build
ffmpeg_build: ffmpeg_configure
	cd $(ffmpeg_dir) && $(MAKE) -j4

.phony: ffmpeg_install 
ffmpeg_install: ffmpeg_build
	$(MAKE) -C $(ffmpeg_dir) install

.phony: ffmpeg_clean
ffmpeg_clean:
	$(MAKE) -C $(ffmpeg_dir) clean

.phony: ffmpeg_all
ffmpeg_all: ffmpeg_configure ffmpeg_build ffmpeg_install

### SDL

sdl_COMMON_COMPONENTS:=

export sdl_dir:=$(root_dir)/SDL

export sdl_out:=$(root_dir)/out
export sdl_inc:=$(sdl_out)/include
export sdl_lib:=$(sdl_out)/lib

.phony: sdl_configure
sdl_configure: $(sdl_dir)
	cd $(sdl_dir) && $(CONFIGURE) ./configure $(sdl_COMMON_COMPONENTS) --prefix=$(sdl_out)


.phony: sdl_build
sdl_build: sdl_configure
	cd $(sdl_dir) && $(MAKE) -j4

.phony: sdl_install 
sdl_install: sdl_build
	$(MAKE) -C $(sdl_dir) install
	cp $(sdl_dir)/include/* $(sdl_inc)/

.phony: sdl_clean
sdl_clean:
	$(MAKE) -C $(sdl_dir) clean

.phony: sdl_all
sdl_all: sdl_configure sdl_build sdl_install

### TEST APP

export tap_dir:=$(root_dir)/test-app

.phony: tap_build
tap_build:
	rm -rf ./$(tap_dir)/*.o; rm -rf ./out/*.bin
	$(CXX) -I$(ffmpeg_inc) -I$(sdl_inc) -o out/test.bin $(tap_dir)/*.cpp $(ffmpeg_lib)/libswscale.a $(ffmpeg_lib)/libavformat.a $(ffmpeg_lib)/libavcodec.a $(ffmpeg_lib)/libavutil.a $(ffmpeg_lib)/libswresample.a $(sdl_lib)/libSDL2.a -lz -lm


### WASM library

export wasm_dir:=$(root_dir)/wasm_src
export wasm_out:=$(root_dir)/out/wasm

.phony:
wasm_clean:
	rm -rf $(wasm_dir)/*.o; rm -rf $(wasm_out)/*

.phony: wasm_build
wasm_build: wasm_clean
	mkdir -p $(wasm_out)
	$(CXX) -I$(tap_dir) -I$(ffmpeg_inc) $(COMMON_CFLAGS) \
		$(wasm_dir)/wasm.cpp $(tap_dir)/player.cpp \
		$(ffmpeg_lib)/libswscale.a $(ffmpeg_lib)/libavformat.a $(ffmpeg_lib)/libavcodec.a $(ffmpeg_lib)/libavutil.a $(ffmpeg_lib)/libswresample.a \
		-o eweb_player.js \
		-sMODULARIZE \
		-sEXPORTED_RUNTIME_METHODS=ccall
	mv ./eweb_player* $(wasm_out)/
