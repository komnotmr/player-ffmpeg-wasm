
FFMPEG_COMMON_COMPONENTS := --enable-protocol=file \
--enable-demuxer=hevc --enable-demuxer=h264 --enable-demuxer=pcm_s16le --enable-demuxer=timestamp_inserter --enable-demuxer=mpegts \
--enable-parser=h264 --enable-parser=hevc \
--enable-decoder=pcm_s16le --enable-decoder=mp2 --enable-decoder=pcm_alaw \
--enable-filter=aresample --enable-bsf=extract_extradata



.phony: configure
configure:
	cd ffmpeg && ./configure $(FFMPEG_COMMON_COMPONENTS)

.phony: build clean install
build clean install:
	$(MAKE) -C ffmpeg $@