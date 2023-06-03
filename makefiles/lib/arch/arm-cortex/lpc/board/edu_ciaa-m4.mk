#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# project based on an edu-ciaa-nxp board, cortex-m4f core.
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

ifeq ($(TARGET_NAME), edu_ciaa_retro_poncho)
    $(call emb_info,RETRO-CIAA poncho support enabled)
    CFLAGS += -DBOARD_EDU_CIAA_WITH_RETRO_PONCHO
    TARGET_SUBSYSTEMS ?= video sound
else
# Define the variable even if edu_ciaa has no video or sound
    TARGET_SUBSYSTEMS ?= null
endif

# Include shared platform definitions
$(call emb_include,lib/arch/arm-cortex/lpc/shared/lpc4337-m4.mk)

# System implementation
OBJS += \
    $(LIB_EMBEDULAR_ROOT)/source/drivers/random_sfmt.o \
    $(LIB_EMBEDULAR_ROOT)/source/drivers/rawstor_sd_1bit.o \
    $(TARGET_MFR)/boot/board_edu_ciaa.o \
    $(TARGET_MFR)/boot/shared_iface.o \
    $(TARGET_DRIVERS)/stream_usart.o \
    $(TARGET_DRIVERS)/packet_ssp.o \
    $(TARGET_DRIVERS)/io_board_edu_ciaa.o

ifneq ($(findstring freertos,$(BUILD_LIBS)),)
    $(call emb_info,Using TICKS-OSWRAP)
	OBJS += $(LIB_EMBEDULAR_ROOT)/source/boot/ticks_oswrap.o
else
    $(call emb_info,Using TICKS-SYSTICK)
	OBJS += $(TARGET_ARCH)/boot/ticks_systick.o
endif

ifeq ($(TARGET_NAME), edu_ciaa_retro_poncho)
	OBJS += $(LIB_EMBEDULAR_ROOT)/source/drivers/stream_esp32at_tcp_server.o
    OBJS += $(TARGET_DRIVERS)/io_dual_nes_videoex.o
endif

ifneq ($(filter video,$(TARGET_SUBSYSTEMS)),)
    OBJS += $(TARGET_DRIVERS)/video_dualcore.o
endif

ifneq ($(filter sound,$(TARGET_SUBSYSTEMS)),)
    OBJS += $(TARGET_DRIVERS)/sound_pcm5100.o
    LIB_LPCOPEN_CHIP_DRIVERS += i2s
endif

LIB_EMBEDULAR_SUBSYSTEMS += $(TARGET_SUBSYSTEMS)

# Additional LPCOpen drivers needed for target code
LIB_LPCOPEN_CHIP_DRIVERS += gpio clock rtc i2cm gpdma

# M4 required libraries
$(call emb_include,lib/embedul.ar.mk)
$(call emb_include,lib/arch/arm-cortex/lpc/lpcopen.mk)
$(call emb_include,lib/3rd_party/sfmt.mk)
