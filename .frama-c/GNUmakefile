# Makefile template for Frama-C/Eva case studies.
# For details and usage information, see the Frama-C User Manual.

### Prologue. Do not modify this block. #######################################
-include path.mk # path.mk contains variables specific to each user
                 # (e.g. FRAMAC, FRAMAC_GUI) and should not be versioned. It is
                 # an optional include, unnecessary if frama-c is in the PATH.
FRAMAC ?= frama-c # FRAMAC is defined in path.mk when it is included, but the
                  # user can override it in the command-line.
include $(shell $(FRAMAC)-config -print-share-path)/analysis-scripts/prologue.mk
###############################################################################

# Edit below as needed. Suggested flags are optional.

MACHDEP = x86_64

## Preprocessing flags (for -cpp-extra-args)
CPPFLAGS    += \

## General flags
FCFLAGS     += \
  -json-compilation-database ../compile_commands.json \
  -add-symbolic-path=.:.. \
  -kernel-warn-key annot:missing-spec=abort \
  -kernel-warn-key typing:implicit-function-declaration=abort \
  -c11

## Eva-specific flags
EVAFLAGS    += \
  -eva-warn-key builtins:missing-spec=abort \

## GUI-only flags
FCGUIFLAGS += \
  -add-symbolic-path=.:.. \

## Analysis targets (suffixed with .eva)
TARGETS = native_hosted.eva

### Each target <t>.eva needs a rule <t>.parse with source files as prerequisites
native_hosted.parse: \
../examples/log_hello_world/main.c \
../source/3rd_party/fatfs-0.14b/ff.c \
../source/3rd_party/sfmt-1.5.1/SFMT.c \
../source/arch/native/sdl/drivers/board_hosted_sdl.c \
../source/arch/native/sdl/drivers/board_hosted_sdl/io_keyboard_sdl.c \
../source/arch/native/sdl/drivers/board_hosted_sdl/rawstor_file.c \
../source/arch/native/sdl/drivers/board_hosted_sdl/sound_sdl.c \
../source/arch/native/sdl/drivers/board_hosted_sdl/stream_file.c \
../source/arch/native/sdl/drivers/board_hosted_sdl/video_sdl.c \
../source/core/anim.c \
../source/core/array.c \
../source/core/bitfield.c \
../source/core/cyclic.c \
../source/core/device.c \
../source/core/device/board.c \
../source/core/device/board/init.c \
../source/core/device/io.c \
../source/core/device/packet.c \
../source/core/device/packet/error_log.c \
../source/core/device/random.c \
../source/core/device/rawstor.c \
../source/core/device/sound.c \
../source/core/device/sound/mixer.c \
../source/core/device/sound/opl/fmopl.c \
../source/core/device/stream.c \
../source/core/device/video.c \
../source/core/device/video/collide.c \
../source/core/device/video/dotmap.c \
../source/core/device/video/fade.c \
../source/core/device/video/font.c \
../source/core/device/video/font_std.c \
../source/core/device/video/line.c \
../source/core/device/video/pixel.c \
../source/core/device/video/rect.c \
../source/core/device/video/rgb332.c \
../source/core/device/video/sprite.c \
../source/core/device/video/tile.c \
../source/core/device/video/tilemap.c \
../source/core/fatfs/diskio.c \
../source/core/main.c \
../source/core/manager/comm.c \
../source/core/manager/input.c \
../source/core/manager/input/action.c \
../source/core/manager/input/profile.c \
../source/core/manager/log.c \
../source/core/manager/output.c \
../source/core/manager/storage.c \
../source/core/manager/storage/cache.c \
../source/core/misc/hsl.c \
../source/core/object.c \
../source/core/sequence.c \
../source/core/utf8.c \
../source/core/variant.c \
../source/drivers/random_sfmt.c \
../source/core/main.c

### Epilogue. Do not modify this block. #######################################
include $(shell $(FRAMAC)-config -print-share-path)/analysis-scripts/epilogue.mk
###############################################################################
