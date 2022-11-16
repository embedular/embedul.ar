#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# lpcopen library.
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

$(call emb_need_var,CPU_MODEL)
$(call emb_need_var,TARGET_BSP)

$(call emb_declare_lib,$\
    NXP LPCOpen,$\
	LIB_LPCOPEN,$\
	$(TARGET_BSP),$\
	)

CFLAGS += -DCHIP_LPC43XX -D__CODE_RED
CFLAGS += -D__USE_LPCOPEN

# Chip support library (M0 & M4)
LIB_LPCOPEN_CHIP := $(LIB_LPCOPEN)/chip_18xx_43xx

# Path to CMSIS (required)
CFLAGS += -I$(LIB_LPCOPEN)/CMSIS/Include

# Path to LPCOpen chip files and usbd_rom
CFLAGS += -I$(LIB_LPCOPEN_CHIP)
CFLAGS += -I$(LIB_LPCOPEN)/usbd_rom

# Standard sysinit (with or without board)
OBJS += $(LIB_LPCOPEN)/startup/sysinit.o

# No CRP
OBJS += $(LIB_LPCOPEN)/startup/no_crp.o

# Core dependant options
ifeq ($(CPU_MODEL), cortex-m4)
    CFLAGS += -DCORE_M4
    CFLAGS += -DLPC43XX_CORE_M4
    # fpuInit() called from SystemInit() in sysint.c
    OBJS += $(LIB_LPCOPEN_CHIP)/fpu_init.o
else
    ifeq ($(CPU_MODEL), cortex-m0)
	CFLAGS += -DCORE_M0
        CFLAGS += -DLPC43XX_CORE_M0APP
    else
        $(error Unexpected CPU_MODEL '$(CPU_MODEL)')
    endif
endif

# Selectable chip drivers
define include_driver
    OBJS += $(LIB_LPCOPEN_CHIP)/$(1)_18xx_43xx.o
endef

# Remove duplicates
LIB_LPCOPEN_CHIP_DRIVERS := $(sort $(LIB_LPCOPEN_CHIP_DRIVERS))

ifeq ($(LIB_LPCOPEN_CHIP_DRIVERS),)
    $(call emb_info,Using CMSIS only)
else
    $(call emb_info,Using chip drivers '$(LIB_LPCOPEN_CHIP_DRIVERS)')

    # uart chip code requires ring_buffer functions
    ifneq ($(filter uart,$(LIB_LPCOPEN_CHIP_DRIVERS)),)
        OBJS += $(LIB_LPCOPEN_CHIP)/ring_buffer.o
    endif

    $(foreach name,$(LIB_LPCOPEN_CHIP_DRIVERS),$(eval $(call include_driver,$(name))))
endif

# USB Mass storage enabled
ifeq ($(TARGET_STORAGE_USBHMS),yes)
    LIB_LPCOPEN_USB_HOST_CLASSES += MassStorage
endif

# Remove duplicates
LIB_LPCOPEN_USB_HOST_CLASSES := $(sort $(LIB_LPCOPEN_USB_HOST_CLASSES))

# LPCUSBLib
ifneq ($(LIB_LPCOPEN_USB_HOST_CLASSES),)
    $(call emb_info,Using USB host library)
    # LPCUSBLib
    $(call emb_declare_lib,NXP LPCUSBLib,LIB_LPCOPEN_USBLIB,$(LIB_LPCOPEN)/LPCUSBLib,)
    $(call emb_info,Using LCPCUSBLib host classes '$(LIB_LPCOPEN_USB_HOST_CLASSES)')

    CFLAGS += -D__LPC43XX__
    
    # Host only
    CFLAGS += -DUSB_HOST_ONLY    
    CFLAGS += -I$(LIB_LPCOPEN_USBLIB)
    CFLAGS += -I$(LIB_LPCOPEN_USBLIB)/Common
    CFLAGS += -I$(LIB_LPCOPEN_USBLIB)/Drivers/USB
    CFLAGS += -I$(LIB_LPCOPEN_USBLIB)/Drivers/USB/Class
    CFLAGS += -I$(LIB_LPCOPEN_USBLIB)/Drivers/USB/Class/Common
    CFLAGS += -I$(LIB_LPCOPEN_USBLIB)/Drivers/USB/Class/Host
    CFLAGS += -I$(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core
    CFLAGS += -I$(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/HAL
    CFLAGS += -I$(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/HAL/LPC18XX
    CFLAGS += -I$(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/HCD
    CFLAGS += -I$(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/HCD/EHCI

    # HAL: 18xx == 43xx
    OBJS += $(LIB_LPCOPEN_USBLIB)/Drivers/USB/Class/Common/HIDParser.o \
            $(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/ConfigDescriptor.o \
            $(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/Device.o \
            $(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/DeviceStandardReq.o \
            $(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/Endpoint.o \
            $(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/EndpointStream.o \
            $(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/Events.o \
            $(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/Host.o \
            $(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/HostStandardReq.o \
            $(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/Pipe.o \
            $(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/PipeStream.o \
            $(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/USBController.o \
            $(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/USBMemory.o \
            $(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/USBTask.o \
            $(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/HAL/LPC18XX/HAL_LPC18xx.o \
            $(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/HCD/HCD.o \
            $(LIB_LPCOPEN_USBLIB)/Drivers/USB/Core/HCD/EHCI/EHCI.o

    LIB_LPCOPEN_USB_HOST := $(LIB_LPCOPEN)/lpcusblib_host
    CFLAGS += -I$(LIB_LPCOPEN_USB_HOST)

    # One or more of Audio, CDC, HID, MassStorage, MIDI, Printer, RNDIS, StillImage (see path contents)
    include_hostusbclass = OBJS += $(LIB_LPCOPEN_USBLIB)/Drivers/USB/Class/Host/$(1)ClassHost.o
    $(foreach name,$(LIB_LPCOPEN_USB_HOST_CLASSES),$(eval $(call include_hostusbclass,$(name))))
endif
