#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# lpc4337 cortex-m4 common parameters.
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

$(call emb_must_precede,edu_ciaa-m4.mk retro_ciaa-m4.mk)
$(call emb_need_var,TARGET_SUBSYSTEMS)

# Include target core definitions (LPC4337JBD-144 M4F core)
$(call emb_include,cpu/arch/arm-cortex/lpc/lpc4337jbd144_m4.mk)

# Include shared platform definitions
$(call emb_include,lib/arch/arm-cortex/lpc/shared/lpc4337.mk)

# LPCOpen board code for M4 core
OBJS += \
    $(TARGET_ARCH)/shared/systick.o \
    $(TARGET_BSP_BOARD)/board.o \
    $(TARGET_BSP_BOARD)/board_sysinit.o \
    $(TARGET_BSP)/shared/i2cm.o \
    $(TARGET_BSP)/shared/panic.o \
    $(TARGET_BSP)/shared/seed.o \
    $(TARGET_BSP)/shared/sysinit_util.o


FLASH_BASE ?= 0x1A000000

# Startup and LD scripts for M4 core
OBJS += $(STARTUP_ROOT)/cr_startup_lpc43xx-m4.o

ifneq ($(filter video,$(TARGET_SUBSYSTEMS)),)
    LDFLAGS += -T $(LD_ROOT)/lpc4337jbd144_m4_memory-video.ld
else
    LDFLAGS += -T $(LD_ROOT)/lpc4337jbd144_m4_memory.ld
endif

LDFLAGS += -T $(LD_ROOT)/lpc4337jbd144_m4_sections.ld
