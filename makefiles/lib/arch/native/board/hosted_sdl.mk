#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# project based on a hosted environment using the sdl library.
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

TARGET_SUBSYSTEMS ?= video sound

# Include target cpu definitions
$(call emb_include,cpu/arch/native/generic.mk)

TARGET_ARCH := $(LIB_EMBEDULAR_ROOT)/source/arch/native
TARGET_MFR := $(TARGET_ARCH)/sdl
TARGET_FAMILY := $(TARGET_MFR)
TARGET_BSP := $(TARGET_MFR)
TARGET_BSP_BOARD := $(TARGET_MFR)/drivers
TARGET_DRIVERS := $(TARGET_BSP_BOARD)/board_hosted_sdl

# Common target init and checks
$(call emb_include,target/check.mk)

# No flash tool present when target is native ("make flash" will fail)
FLASH_TOOL ?= no

# System code
OBJS += \
    $(LIB_EMBEDULAR_ROOT)/source/drivers/random_sfmt.o \
    $(TARGET_BSP_BOARD)/board_hosted_sdl.o \
    $(TARGET_DRIVERS)/video_sdl.o \
    $(TARGET_DRIVERS)/sound_sdl.o \
    $(TARGET_DRIVERS)/io_keyboard_sdl.o \
    $(TARGET_DRIVERS)/stream_file.o \
    $(TARGET_DRIVERS)/rawstor_file.o

LIB_EMBEDULAR_SUBSYSTEMS += $(TARGET_SUBSYSTEMS)

# Include required libraries
$(call emb_include,lib/embedul.ar.mk)
$(call emb_include,lib/arch/native/sdl.mk)
$(call emb_include,lib/3rd_party/sfmt.mk)
