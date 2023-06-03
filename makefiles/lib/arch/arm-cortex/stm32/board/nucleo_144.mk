#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# project based on a generic st nucleo-144 board.
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

# Nucleo-144 has no video or audio capabilities by itself
TARGET_SUBSYSTEMS ?= null

TARGET_CHIP_TMP := $(call emb_split_get,$(TARGET_NAME),2)

# Include target core definitions
$(call emb_include,cpu/arch/arm-cortex/stm32/$(TARGET_CHIP_TMP).mk)

CHIP_FAMILY_UC := $(shell echo $(CHIP_FAMILY) | tr f F)

TARGET_REQUIRE_BIN_IMAGE := yes

TARGET_BOARD := nucleo_144

$(call emb_include,lib/arch/arm-cortex/stm32/shared/target.mk)

CFLAGS += -I$(TARGET_BSP_BOARD)/cubemx/Core/Inc

# Each mcu supported on NUCLEO-144 generates the same STM32CubeMX peripheral
# board filenames.
OBJS += \
	$(TARGET_MFR)/boot/board_nucleo_144.o \
	$(TARGET_DRIVERS)/io_board_nucleo_144.o \
	$(TARGET_DRIVERS)/stream_uart.o \
	$(TARGET_DRIVERS)/random_rng.o \
	$(TARGET_BSP)/Drivers/BSP/STM32$(CHIP_FAMILY_UC)xx_Nucleo_144/stm32$(CHIP_FAMILY)xx_nucleo_144.o \
    $(TARGET_BSP_BOARD)/cubemx/Core/Src/eth.o \
	$(TARGET_BSP_BOARD)/cubemx/Core/Src/gpio.o \
	$(TARGET_BSP_BOARD)/cubemx/Core/Src/main.o \
	$(TARGET_BSP_BOARD)/cubemx/Core/Src/rng.o \
	$(TARGET_BSP_BOARD)/cubemx/Core/Src/stm32$(CHIP_FAMILY)xx_hal_msp.o \
	$(TARGET_BSP_BOARD)/cubemx/Core/Src/usart.o \
	$(TARGET_BSP_BOARD)/cubemx/Core/Src/usb_otg.o

ifneq ($(findstring freertos,$(BUILD_LIBS)),)
    $(call emb_info,Using TICKS-OSWRAP)
	OBJS += $(LIB_EMBEDULAR_ROOT)/source/boot/ticks_oswrap.o
else
    $(call emb_info,Using TICKS-SYSTICK)
	OBJS += $(TARGET_ARCH)/boot/ticks_systick.o
endif

LDFLAGS += -T $(TARGET_FAMILY)/ld/$(TARGET_CHIP_TMP)_flash.ld

# STM32 HAL/LL drivers needed for base board code
LIB_STM32CUBE_HAL_DRIVERS += rcc rcc_ex flash flash_ex flash_ramfunc gpio \
                             dma_ex dma pwr pwr_ex cortex exti eth tim tim_ex \
						     uart pcd pcd_ex rng
LIB_STM32CUBE_LL_DRIVERS += usb

# Required libraries
$(call emb_include,lib/embedul.ar.mk)
$(call emb_include,lib/arch/arm-cortex/stm32/stm32cube.mk)
