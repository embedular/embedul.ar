#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# project based on a retro-ciaa board, cortex-m4f core.
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

TARGET_SUBSYSTEMS ?= video sound

# Include shared platform definitions
$(call emb_include,lib/arch/arm-cortex/lpc/shared/lpc4337-m4.mk)

# System implementation
OBJS += \
    $(LIB_EMBEDULAR_ROOT)/source/drivers/random_sfmt.o \
    $(LIB_EMBEDULAR_ROOT)/source/drivers/io_dual_genesis_pca9673.o \
    $(LIB_EMBEDULAR_ROOT)/source/drivers/io_lp5036.o \
	$(LIB_EMBEDULAR_ROOT)/source/drivers/io_huewheel.o \
	$(LIB_EMBEDULAR_ROOT)/source/drivers/io_pca9956b.o \
	$(LIB_EMBEDULAR_ROOT)/source/drivers/packet_esp32at_tcp_server.o \
    $(TARGET_DRIVERS)/board_retro_ciaa.o \
    $(TARGET_DRIVERS)/board_shared/iface_methods.o \
    $(TARGET_DRIVERS)/stream_usart.o \
    $(TARGET_DRIVERS)/packet_i2c_controller.o \
    $(TARGET_DRIVERS)/rawstor_sd_sdmmc.o \
    $(TARGET_DRIVERS)/board_retro_ciaa/io_board.o

ifneq ($(filter video,$(TARGET_SUBSYSTEMS)),)
    OBJS += $(TARGET_DRIVERS)/video_dualcore.o
endif

ifneq ($(filter sound,$(TARGET_SUBSYSTEMS)),)
    OBJS += $(TARGET_DRIVERS)/sound_pcm5100.o
    LIB_LPCOPEN_CHIP_DRIVERS += i2s
endif

LIB_EMBEDULAR_SUBSYSTEMS += $(TARGET_SUBSYSTEMS)

# Additional LPCOpen drivers needed for target code
LIB_LPCOPEN_CHIP_DRIVERS += gpio clock rtc i2cm gpdma sdif sdmmc evrt

# M4 required libraries
$(call emb_include,lib/embedul.ar.mk)
$(call emb_include,lib/arch/arm-cortex/lpc/lpcopen.mk)
$(call emb_include,lib/3rd_party/sfmt.mk)
