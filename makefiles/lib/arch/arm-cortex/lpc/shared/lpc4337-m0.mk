#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# lpc4337 cortex-m0 common parameters.
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

$(call emb_must_precede,edu_ciaa-m0.mk retro_ciaa-m0.mk)
$(call emb_need_var,TARGET_SUBSYSTEMS)

# Include target core definitions (LPC4337JBD-144 M0 core)
$(call emb_include,cpu/arch/arm-cortex/lpc/lpc4337jbd144_m0.mk)

# Include shared platform definitions
$(call emb_include,lib/arch/arm-cortex/lpc/shared/lpc4337.mk)

# LPCOpen board code for M0 core
OBJS += \
    $(TARGET_BSP_BOARD)/board.o \
    $(TARGET_BSP)/shared/i2cm.o

FLASH_BASE ?= 0x1B000000

ifneq ($(filter video-adapter,$(TARGET_SUBSYSTEMS)),)
    # Selected modeline.
    $(call emb_need_var,VIDEO_ADAPTER_MODELINE)
    # VIDEO_TargetLineOut function that matches the selected modeline.
    $(call emb_need_var,VIDEO_ADAPTER_LINEOUT)
    # Video signal timing fine-tuning.
    $(call emb_need_var,VIDEO_ADAPTER_HSYNC_RIT_TUNE)
    $(call emb_need_var,VIDEO_ADAPTER_HSYNC_IC_TUNE)
    $(call emb_need_var,VIDEO_ADAPTER_BACK_PORCH_IC_TUNE)
    # GPIO pixel mask, mask address and masked pin address.
    $(call emb_need_var,VIDEO_ADAPTER_GPIO_MASK)
    $(call emb_need_var,VIDEO_ADAPTER_GPIO_MASK_ADDR)
    $(call emb_need_var,VIDEO_ADAPTER_GPIO_MPIN_ADDR)

    CFLAGS += -D'VIDEO_ADAPTER_GPIO_MASK=$(VIDEO_ADAPTER_GPIO_MASK)'
    CFLAGS += -D'VIDEO_ADAPTER_GPIO_MASK_ADDR=$(VIDEO_ADAPTER_GPIO_MASK_ADDR)'
    CFLAGS += -D'VIDEO_ADAPTER_GPIO_MPIN_ADDR=$(VIDEO_ADAPTER_GPIO_MPIN_ADDR)'

    CFLAGS += -D'VIDEO_ADAPTER_MODELINE=embedul.ar/source/core/misc/modeline/$(VIDEO_ADAPTER_MODELINE).h'
    CFLAGS += -D'VIDEO_ADAPTER_HSYNC_RIT_TUNE=$(VIDEO_ADAPTER_HSYNC_RIT_TUNE)'
    CFLAGS += -D'VIDEO_ADAPTER_HSYNC_IC_TUNE=$(VIDEO_ADAPTER_HSYNC_IC_TUNE)'
    CFLAGS += -D'VIDEO_ADAPTER_BACK_PORCH_IC_TUNE=$(VIDEO_ADAPTER_BACK_PORCH_IC_TUNE)'

    CFLAGS += -D'VIDEO_ADAPTER_LINEOUT_STR="$(VIDEO_ADAPTER_LINEOUT)"'

    # Video adapter
    OBJS += \
        $(TARGET_ARCH)/shared/m0_instdelay.o \
        $(TARGET_MFR)/video_dualcore_adapter/adapter.o \
        $(TARGET_DRIVERS)/board_$(TARGET_BOARD)/video_dualcore_adapter/$(VIDEO_ADAPTER_LINEOUT).o

    # Video adapter code executed from SRAM to achieve required performance
    TARGET_LPC4337_M0_EXECUTE_FROM_SRAM ?= yes
endif

# Startup and LD scripts for M0 core
LDFLAGS += -T $(LD_ROOT)/lpc4337jbd144_m0_memory.ld

ifeq ($(TARGET_LPC4337_M0_EXECUTE_FROM_SRAM),yes)
   $(call emb_info,Startup and linker script to execute from SRAM)
   OBJS += $(STARTUP_ROOT)/cr_startup_lpc43xx-m0app-sram.o
   LDFLAGS += -T $(LD_ROOT)/lpc4337jbd144_m0_sections-sram.ld
else
   $(call emb_info,Startup and linker script to execute from FLASH)
   OBJS += $(STARTUP_ROOT)/cr_startup_lpc43xx-m0app.o
   LDFLAGS += -T $(LD_ROOT)/lpc4337jbd144_m0_sections.ld
endif
