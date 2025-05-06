
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

export ffmpeg_out:=$(shell pwd)/out
export ffmpeg_inc:=$(ffmpeg_out)/include
export ffmpeg_lib:=$(ffmpeg_out)/lib

.phony: configure
configure:
	cd ffmpeg && $(CONFIGURE) ./configure $(FFMPEG_COMMON_COMPONENTS) --prefix=$(ffmpeg_out)

tap:
	rm -rf ./test-app/*.o; rm -rf ./out/*.bin
	$(CXX) -I$(ffmpeg_inc) -o out/test.bin test-app/app.cpp test-app/test.cpp $(ffmpeg_lib)/libswscale.a $(ffmpeg_lib)/libavformat.a $(ffmpeg_lib)/libavcodec.a $(ffmpeg_lib)/libavutil.a $(ffmpeg_lib)/libswresample.a -lz -lm

.phony: libffmpeg
libffmpeg:
	cd ffmpeg && $(MAKE) -j4

# $(CC) -I$(ffmpeg_inc) -o a.bin test.c $(ffmpeg_lib)/libavformat.a $(ffmpeg_lib)/libavcodec.a $(ffmpeg_lib)/libavutil.a $(ffmpeg_lib)/libswscale.a -lz -lm

.phony: build clean install
build clean install:
	$(MAKE) -C ffmpeg $@