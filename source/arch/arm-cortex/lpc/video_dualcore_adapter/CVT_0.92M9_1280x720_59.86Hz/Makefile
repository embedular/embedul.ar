PROJECT_NAME := $(notdir $(CURDIR))
PROJECT_ROOT ?= .
SDK_ROOT = $(PROJECT_ROOT)/../../..
VERBOSE ?= no
OLEVEL ?= 3
DEBUG ?= no

# Project include paths
CFLAGS += -I$(PROJECT_ROOT)

DDA_MODELINE := CVT_0.92M9_1280x720_59.86Hz
DDA_LINEOUT := fb256-14ipp
DDA_HSYNC_RIT_TUNE ?= 0
DDA_HSYNC_IC_TUNE ?= 0
DDA_BACK_PORCH_IC_TUNE ?= 0

include $(SDK_ROOT)/makefiles/build.mk
