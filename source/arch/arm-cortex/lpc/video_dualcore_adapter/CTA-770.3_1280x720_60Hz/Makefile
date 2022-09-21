PROJECT_NAME := $(notdir $(CURDIR))
PROJECT_ROOT ?= .
SDK_ROOT = $(PROJECT_ROOT)/../../..
VERBOSE ?= no
OLEVEL ?= 3
DEBUG ?= no

# Project include paths
CFLAGS += -I$(PROJECT_ROOT)

DDA_MODELINE := CTA-770.3_1280x720_60Hz
DDA_LINEOUT := fb256-14_13_14_14ipp
# Required hsync pulse frequency 45000 Hz, measured 44994 Hz
# Tune at -1 gives 45003 Hz
DDA_HSYNC_RIT_TUNE ?= -1
# Required 538.72 ns, measured 610 ns (72 ns, ~15 instructions at 204 Mhz).
DDA_HSYNC_IC_TUNE ?= -15
# Required 2962.96 ns, measured 3390 ns (427 ns, ~87 instructions at 204 Mhz).
DDA_BACK_PORCH_IC_TUNE ?= -87
# Required front porch 1481 ns, measured 1190 ns. The only way to reduce this
# time is to write less pixels in DDA_TargetLineOut to compensate for the
# higher pixel period.
# 1/74250000 Hz * 1000000000 = 13,468013468 ns pixel period.
# framebuffer pixel = 13,468013468 ns * 5 = 67,34006734 ns
# Instructions per pixel = 67,34006734 ns / (1/204000000) * 1000000000 =
# 13,737373737 instructions
# 13,73... x 256 = 3516,767676764 instructions
# = 14 x 256 = 3584 instructions
# 3584 - 3517 = 67 instructions
# 67 * (1/204000000) * 1000000000 = 328,431372549 ns
# 328,431372÷(4,901×14) = 4,7867 pixels

# Required vsync pulse frequency 60 Hz, easured: 60.0051 Hz

include $(SDK_ROOT)/makefiles/build.mk
