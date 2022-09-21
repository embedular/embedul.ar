#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# project based on an edu-ciaa-nxp board, cortex-m0 core.
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
    TARGET_SUBSYSTEMS ?= video video-adapter
endif

ifneq ($(filter video-adapter,$(TARGET_SUBSYSTEMS)),)
    # Pixel video output bit arrangement for video adapter
    #
    #                   Format          Arrangement
    # BIT                                            8  7  6  5  4  3  2  1  0
    # PIXEL DATA        (RGB332)        [7-0]        x  R2 R1 R0 G2 G1 G0 B1 B0
    # GPIO2             (RGB332)        [8,6-0]      R2 x  R1 R0 G2 G1 G0 B1 B0
    #
    # MASK: A low state indicates the pin is settable and readable via the
    #       masked write and read functions.
    VIDEO_ADAPTER_GPIO_MASK := 0xFFFFFE80
    VIDEO_ADAPTER_GPIO_MASK_ADDR := 0x400F6088
    VIDEO_ADAPTER_GPIO_MPIN_ADDR := 0x400F6188
endif

# Include shared target definitions
$(call emb_include,lib/arch/arm-cortex/lpc/shared/lpc4337-m0.mk)

ifneq ($(filter video-adapter,$(TARGET_SUBSYSTEMS)),)
    # Short adapter description
    CFLAGS += -D'VIDEO_ADAPTER_STR="ANALOG NES"'

    # Target video adapter specialization
    OBJS += $(TARGET_DRIVERS)/board_$(TARGET_BOARD)/video_dualcore_adapter/adapter_analog_nes.o

    LIB_LPCOPEN_CHIP_DRIVERS += ritimer
endif

# Additional LPCOpen drivers needed for target code
LIB_LPCOPEN_CHIP_DRIVERS += gpio clock i2cm

# M0 required libraries
$(call emb_include,lib/arch/arm-cortex/lpc/lpcopen.mk)
