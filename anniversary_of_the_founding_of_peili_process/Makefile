CC       = gcc
CFLAGS   = -Wall -Wextra -std=c11 -Isrc
LDFLAGS  = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lm -mwindows

SRC_DIR   = src
DIST_DIR  = dist
ASSET_DIR = assets

SRCS     = $(wildcard $(SRC_DIR)/*.c)
TARGET   = $(DIST_DIR)/game.exe

SDL_BIN  = /c/msys64/mingw64/bin

# Complete DLL set (resolved via full dependency chain from objdump)
DEPLOY_DLLS = \
	SDL2.dll \
	SDL2_image.dll \
	SDL2_ttf.dll \
	libfreetype-6.dll \
	libharfbuzz-0.dll \
	libbrotlidec.dll \
	libbrotlienc.dll \
	libbrotlicommon.dll \
	libbz2-1.dll \
	libpng16-16.dll \
	zlib1.dll \
	libgcc_s_seh-1.dll \
	libstdc++-6.dll \
	libwinpthread-1.dll \
	libglib-2.0-0.dll \
	libintl-8.dll \
	libpcre2-8-0.dll \
	libiconv-2.dll \
	libgraphite2.dll \
	libjpeg-8.dll \
	libavif-16.dll \
	libaom.dll \
	libdav1d-7.dll \
	librav1e.dll \
	libsharpyuv-0.dll \
	libSvtAv1Enc-4.dll \
	libyuv.dll \
	libwebp-7.dll \
	libwebpdemux-2.dll \
	libjxl.dll \
	libjxl_cms.dll \
	libhwy.dll \
	liblcms2-2.dll \
	libtiff-6.dll \
	libdeflate.dll \
	libjbig-0.dll \
	libLerc.dll \
	liblzma-5.dll \
	libzstd.dll

.PHONY: all clean deploy copy-dlls copy-assets dist

all: $(TARGET) deploy

deploy: copy-dlls copy-assets

$(TARGET): $(SRCS) $(SRC_DIR)/game.h $(SRC_DIR)/ui.h
	@mkdir -p "$(DIST_DIR)"
	/c/msys64/usr/bin/env.exe MSYSTEM=MINGW64 PATH="/mingw64/bin:/usr/bin" \
		/bin/bash -l -c "cd '$(CURDIR)' && $(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)"

copy-dlls:
	@for dll in $(DEPLOY_DLLS); do \
		if [ -f "$(SDL_BIN)/$$dll" ]; then \
			cp -f "$(SDL_BIN)/$$dll" "$(DIST_DIR)/"; \
		else \
			echo "WARNING: $$dll not found in $(SDL_BIN)"; \
		fi; \
	done

copy-assets:
	@mkdir -p "$(DIST_DIR)/$(ASSET_DIR)"
	@cp -f $(ASSET_DIR)/* "$(DIST_DIR)/$(ASSET_DIR)/"

# ── Windows-compatible distribution packaging ──
dist: $(TARGET)
	@echo "=== 正在构建发布版本... ==="
	-cmd /c "mkdir dist 2> nul"
	@cmd /c "xcopy /E /I /Y assets dist\\assets"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\SDL2.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\SDL2_image.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\SDL2_ttf.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libfreetype-6.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libharfbuzz-0.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libbrotlidec.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libbrotlienc.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libbrotlicommon.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libbz2-1.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libpng16-16.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\zlib1.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libgcc_s_seh-1.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libstdc++-6.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libwinpthread-1.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libglib-2.0-0.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libintl-8.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libpcre2-8-0.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libiconv-2.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libgraphite2.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libjpeg-8.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libavif-16.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libaom.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libdav1d-7.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\librav1e.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libsharpyuv-0.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libSvtAv1Enc-4.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libyuv.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libwebp-7.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libwebpdemux-2.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libjxl.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libjxl_cms.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libhwy.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\liblcms2-2.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libtiff-6.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libdeflate.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libjbig-0.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libLerc.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\liblzma-5.dll dist/"
	-cmd /c "copy /Y C:\\msys64\\mingw64\\bin\\libzstd.dll dist/"
	@echo "=== 发布版本已生成在 ./dist 目录 ==="

clean:
	rm -f $(TARGET)
	@for dll in $(DEPLOY_DLLS); do rm -f "$(DIST_DIR)/$$dll"; done
	rm -rf $(DIST_DIR)/$(ASSET_DIR)
