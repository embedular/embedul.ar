#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# smt32 target variables.
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
$(call emb_need_var,TARGET_NAME)
$(call emb_need_var,TARGET_BOARD)

TARGET_ARCH := $(LIB_EMBEDULAR_ROOT)/source/arch/arm-cortex
TARGET_MFR := $(TARGET_ARCH)/stm32
TARGET_FAMILY := $(TARGET_MFR)/$(CHIP_FAMILY)
TARGET_DRIVERS := $(TARGET_MFR)/drivers
TARGET_BSP := $(TARGET_FAMILY)/cube
TARGET_BSP_BOARD := $(TARGET_MFR)/$(CHIP_FAMILY)/boards/$(TARGET_NAME)

CFLAGS += -I$(TARGET_MFR)/boot

# Common target init and checks
$(call emb_include,target/check.mk)

# OpenOCD by default
FLASH_TOOL ?= openocd-elf

# OpenOCD configuration file
OPENOCD_CFG := $(TARGET_FAMILY)/openocd/$(TARGET_BOARD).cfg

OBJS += $(TARGET_ARCH)/syscalls.o
