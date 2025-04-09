.phony: configure
configure:
	cd ffmpeg && ./configure \
		--disable-everything \
		--disable-x86asm \
		--enable-avcodec \
		--enable-avformat \
		--enable-avfilter \
		--enable-swresample

.phony: build
build:
	make -C ffmpeg $@