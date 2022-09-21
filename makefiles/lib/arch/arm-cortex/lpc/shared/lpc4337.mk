#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# lpc4337 cortex-m4/m0 common parameters.
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

$(call emb_must_precede,lpc4337-m4.mk lpc4337-m0.mk)
$(call emb_need_var,LIB_EMBEDULAR_ROOT)
$(call emb_need_var,TARGET_NAME)

# Needed by LD scripts on both M0 and M4
# How much $FLASH_BASE storage the M0 core takes for its binary executable
# May be zero, in which case it must be loaded from another storage medium
TARGET_LPC4337_M0_FLASH_STORAGE_SIZE ?= 8192

TARGET_REQUIRE_BIN_IMAGE := yes

TARGET_BOARD ?= $(TARGET_NAME)
TARGET_ARCH := $(LIB_EMBEDULAR_ROOT)/source/arch/arm-cortex
TARGET_MFR := $(TARGET_ARCH)/lpc
TARGET_FAMILY := $(TARGET_MFR)/18xx_43xx
TARGET_DRIVERS := $(TARGET_MFR)/drivers
TARGET_BSP := $(TARGET_FAMILY)/lpcopen
TARGET_BSP_BOARD := $(TARGET_BSP)/boards/$(TARGET_BOARD)

# Common target init and checks
$(call emb_include,target/check.mk)

# Internal, FTDI-based flash tool by default
FLASH_TOOL ?= openocd-elf

# OpenOCD configuration file
OPENOCD_CFG = $(TARGET_FAMILY)/openocd/$(TARGET_BOARD)-ftdi.cfg

# Startup and LD scripts
STARTUP_ROOT ?= $(TARGET_BSP)/startup
LD_ROOT ?= $(TARGET_FAMILY)/ld

# LPCOpen drivers needed for base board code
LIB_LPCOPEN_CHIP_DRIVERS += i2c ssp adc uart sysinit clock chip

LDFLAGS += --defsym __length_core_m0app_MFlash=$(TARGET_LPC4337_M0_FLASH_STORAGE_SIZE)

ifneq ($(filter video,$(TARGET_SUBSYSTEMS)),)
    # Reserves memory space for a video framebuffer
    LDFLAGS += -T $(LD_ROOT)/lpc4337jbd144_memory-video.ld
    # Framebuffer width & height according to reserved memory on linker script   
    CFLAGS += -D'VIDEO_FB_WIDTH=256'
    CFLAGS += -D'VIDEO_FB_HEIGHT=144'
else
    # No video framebuffer
    LDFLAGS += -T $(LD_ROOT)/lpc4337jbd144_memory.ld
endif
