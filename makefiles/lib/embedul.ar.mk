#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# embedul.ar library.
#
# Copyright 2018-2022 Santiago Germino
# <sgermino@embedul.ar> https://www.linkedin.com/in/royconejo
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#

$(call emb_need_var,LIB_EMBEDULAR_ROOT)

# ------------------------------------------------------------------------------
# Library config options (LIB_EMBEDULAR_CONFIG_*)
# ------------------------------------------------------------------------------
# NEED_VIDEO			: the application requires board video support,
#                         will not compile otherwise.
# NEED_SOUND			: the application requires board sound support,
#                         will not compile otherwise.
# SPLASH_SCREENS		: enables(1) or disables(0) showing up to three visual
#                         splash screens right after board init. will be 
#                         disabled automatically if the board has no video
#                         support.
# SPLASH_THEME_L1,L2,L3	: first, second and third splash screens. Names must
#                         exactly match available source files (without 
#                         extension) in source/core/misc/splash_themes/.
# INPUT_SWITCH_ACTION	: enables(1) or disables(0) the processing capability of
#                         detecting and storing clicks, double clicks, holds and
#                         releases on INPUT manager digital inputs.
# ------------------------------------------------------------------------------

$(call emb_declare_lib,$\
	embedul.ar,$\
	LIB_EMBEDULAR,$\
	$(LIB_EMBEDULAR_ROOT)/source/core,$\
	NEED_VIDEO=0 \
	NEED_SOUND=0 \
	SPLASH_SCREENS=0 \
	SPLASH_THEME_L1=lock_glyph \
	SPLASH_THEME_L2=none \
	SPLASH_THEME_L3=c64 \
	INPUT_SWITCH_ACTION=1 \
	INPUT_MAX_DEVICES=10U \
	INPUT_MAX_LIGHTING_DEVICES=2U \
	OUTPUT_MAX_DEVICES=10U \
	OUTPUT_MAX_LIGHT_CHANNELS=48U \
	)


# Miscelaneous system files
OBJS += $(LIB_EMBEDULAR)/device/board.o \
        $(LIB_EMBEDULAR)/device/stream.o \
		$(LIB_EMBEDULAR)/device/packet.o \
		$(LIB_EMBEDULAR)/device/packet/error_log.o \
        $(LIB_EMBEDULAR)/device/rawstor.o \
        $(LIB_EMBEDULAR)/device/random.o \
        $(LIB_EMBEDULAR)/device/io.o \
	    $(LIB_EMBEDULAR)/device/board/init.o \
		$(LIB_EMBEDULAR)/manager/log.o \
        $(LIB_EMBEDULAR)/manager/input.o \
	    $(LIB_EMBEDULAR)/manager/input/switch_action.o \
        $(LIB_EMBEDULAR)/manager/output.o \
        $(LIB_EMBEDULAR)/manager/storage.o \
        $(LIB_EMBEDULAR)/manager/storage/cache.o \
		$(LIB_EMBEDULAR)/manager/comm.o \
        $(LIB_EMBEDULAR)/misc/hsl.o


# Remove optional subsystem duplicates
LIB_EMBEDULAR_SUBSYSTEMS := $(sort $(LIB_EMBEDULAR_SUBSYSTEMS))


# Include subsystems (may modify required base modules)
# RetrOS (Real-time Preemtive Multitasking Operating System)
ifneq ($(filter retros,$(LIB_EMBEDULAR_SUBSYSTEMS)),)
    # Required base modules
    LIB_EMBEDULAR_BASE += queue mempool
    OBJS += $(LIB_EMBEDULAR)/retros/api.o \
            $(LIB_EMBEDULAR)/retros/semaphore.o \
            $(LIB_EMBEDULAR)/retros/mutex.o \
            $(LIB_EMBEDULAR)/retros/private/handlers.o \
            $(LIB_EMBEDULAR)/retros/private/opaque.o \
            $(LIB_EMBEDULAR)/retros/private/runtime.o \
            $(LIB_EMBEDULAR)/retros/private/syscall.o \
            $(LIB_EMBEDULAR)/retros/private/metrics.o \
            $(LIB_EMBEDULAR)/retros/private/scheduler.o
#            $(LIB_EMBEDULAR)/retros/private/driver/storage.o
endif

# Video subsystem
ifneq ($(filter video,$(LIB_EMBEDULAR_SUBSYSTEMS)),)
    CFLAGS += -DLIB_EMBEDULAR_HAS_VIDEO
    # Required base modules
    LIB_EMBEDULAR_BASE += anim
    OBJS += $(LIB_EMBEDULAR)/device/video.o \
	        $(LIB_EMBEDULAR)/device/video/rgb332.o \
            $(LIB_EMBEDULAR)/device/video/dotmap.o \
            $(LIB_EMBEDULAR)/device/video/font.o \
            $(LIB_EMBEDULAR)/device/video/font_std.o \
            $(LIB_EMBEDULAR)/device/video/tile.o \
            $(LIB_EMBEDULAR)/device/video/tilemap.o \
            $(LIB_EMBEDULAR)/device/video/pixel.o \
            $(LIB_EMBEDULAR)/device/video/line.o \
            $(LIB_EMBEDULAR)/device/video/rect.o \
            $(LIB_EMBEDULAR)/device/video/collide.o \
            $(LIB_EMBEDULAR)/device/video/sprite.o \
            $(LIB_EMBEDULAR)/device/video/fade.o
else
    $(call emb_warning,No video subsystem. Splash screens disabled)
    LIB_EMBEDULAR_CONFIG_SPLASH_SCREENS := 0

    ifeq ($(LIB_EMBEDULAR_CONFIG_NEED_VIDEO),1)
		$(call emb_error,The application requires a board with video support.)
	endif
endif

ifeq ($(LIB_EMBEDULAR_CONFIG_SPLASH_SCREENS),1)
    $(call emb_info,Splash screens enabled)
	
    $(foreach theme,$(sort $(filter-out $\
			SPLASH_THEME_L1 SPLASH_THEME_L2 SPLASH_THEME_L3,$\
			$(subst =,$(emb_space),$(LIB_EMBEDULAR_CONFIG_SPLASH_THEME_L1) $\
								   $(LIB_EMBEDULAR_CONFIG_SPLASH_THEME_L2) $\
								   $(LIB_EMBEDULAR_CONFIG_SPLASH_THEME_L3)))),$\
                      $(eval OBJS += $(LIB_EMBEDULAR)/device/video/splash_themes/$(theme).o))
endif

# Sound subsystem
ifneq ($(filter sound,$(LIB_EMBEDULAR_SUBSYSTEMS)),)
    CFLAGS += -DLIB_EMBEDULAR_HAS_SOUND
    OBJS += $(LIB_EMBEDULAR)/device/sound.o \
            $(LIB_EMBEDULAR)/device/sound/mixer.o \
            $(LIB_EMBEDULAR)/device/sound/opl/fmopl.o
else
    ifeq ($(LIB_EMBEDULAR_CONFIG_NEED_SOUND),1)
		$(call emb_error,The application requires a board with sound support.)
	endif
endif


# Required core modules
LIB_EMBEDULAR_BASE += \
    main \
    object \
    device \
	bitfield \
    cyclic \
	variant \
    array \
    utf8 \
    sequence

# Remove base duplicates
LIB_EMBEDULAR_BASE := $(sort $(LIB_EMBEDULAR_BASE))

$(foreach name,$(LIB_EMBEDULAR_BASE),$(eval OBJS += $(LIB_EMBEDULAR)/$(name).o))

$(call emb_info,Using CORE modules '$(LIB_EMBEDULAR_BASE)')
$(call emb_info,Using SUBSYSTEMS '$(LIB_EMBEDULAR_SUBSYSTEMS)')

# Dependencies with 3rd_party libraries
ifneq ($(findstring fatfs,$(BUILD_LIBS)),)
    $(call emb_info,Using FatFs custom disks and config)
    CFLAGS += -DLIB_EMBEDULAR_HAS_FILESYSTEM

    LIB_EMBEDULAR_STORAGE_CACHE_LOCAL_PATH := fs-cache/EMBEDUL.AR/CACHE/APP/
    # Any application that uses the filesystem should also have the filesystem
    # cache directory structure, even if it doesn't use the cache.
    $(call emb_need_dir,$(LIB_EMBEDULAR_STORAGE_CACHE_LOCAL_PATH),Cache directory)

    $(call emb_info,Project cache directory '$(LIB_EMBEDULAR_STORAGE_CACHE_LOCAL_PATH)')

    # Find out the application 8.3 name by looking at the filesystem cache files
    LIB_EMBEDULAR_STORAGE_CACHE_APP_NAME := $(word 1,$(notdir $\
	                    $(wildcard $(LIB_EMBEDULAR_STORAGE_CACHE_LOCAL_PATH)*)))

    ifeq ($(LIB_EMBEDULAR_STORAGE_CACHE_APP_NAME),)
        $(call emb_error,No 8.3 application name found in cache directory)
	endif

    $(call emb_info,Application name in project cache dir. $\
                                    '$(LIB_EMBEDULAR_STORAGE_CACHE_APP_NAME)')

    LIB_EMBEDULAR_STORAGE_CACHE_FS_DRIVE := 0:
    LIB_EMBEDULAR_STORAGE_CACHE_FS_ROOT_DIR := $\
                $(LIB_EMBEDULAR_STORAGE_CACHE_FS_DRIVE)/EMBEDUL.AR/
    LIB_EMBEDULAR_STORAGE_CACHE_FS_FRAMEWORK_DIR := $\
                $(LIB_EMBEDULAR_STORAGE_CACHE_FS_ROOT_DIR)CACHE/FWK/
    LIB_EMBEDULAR_STORAGE_CACHE_FS_APPLICATION_DIR := $\
                $(LIB_EMBEDULAR_STORAGE_CACHE_FS_ROOT_DIR)CACHE/APP/$(LIB_EMBEDULAR_STORAGE_CACHE_APP_NAME)/

    # Cache directories on FAT 8.3 filenames
    CFLAGS += -D'LIB_EMBEDULAR_STORAGE_CACHE_FS_DRIVE="$(LIB_EMBEDULAR_STORAGE_CACHE_FS_DRIVE)"'
    CFLAGS += -D'LIB_EMBEDULAR_STORAGE_CACHE_FS_ROOT_DIR="$(LIB_EMBEDULAR_STORAGE_CACHE_FS_ROOT_DIR)"'
    CFLAGS += -D'LIB_EMBEDULAR_STORAGE_CACHE_FS_FRAMEWORK_DIR="$(LIB_EMBEDULAR_STORAGE_CACHE_FS_FRAMEWORK_DIR)"'
    CFLAGS += -D'LIB_EMBEDULAR_STORAGE_CACHE_FS_APPLICATION_DIR="$(LIB_EMBEDULAR_STORAGE_CACHE_FS_APPLICATION_DIR)"'

    # For FatFs code to find ffconf.h
    CFLAGS += -I$(LIB_EMBEDULAR)/fatfs
    OBJS += $(LIB_EMBEDULAR)/fatfs/diskio.o
endif
