#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# build configuration.
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

$(call emb_must_precede,Makefile build.mk)

# Default values
VERBOSE ?= no
DEBUG ?= yes
OLEVEL ?= 0
GLEVEL ?= 

# Framework base directory, environment variable.
$(call emb_need_var,LIB_EMBEDULAR_PATH)
# Framework root directory, defined by system.mk.
$(call emb_need_var,LIB_EMBEDULAR_ROOT)
# Passed as parameter on the call to 'make -f Makefile'
$(call emb_need_var,BUILD_TARGET)

# Directory name where 'Makefile' resides.
APP_NAME := $(notdir $(CURDIR))

# APP_ROOT specifies the application source code root relative to the Makefile.
# This is where the build/ subdir will reside. Used to be freely configurable,
# but now it is fixed to the Makefile root directory since a moving build/
# subdirectory complicates default IDE Run & Debug configurations.
APP_ROOT := .
# Project include paths
CFLAGS += -I$(APP_ROOT)

# Application name
CFLAGS += -D'CC_AppNameStr=CC_Str($(APP_NAME))'

# Access to embedul.ar framework base from headers.
CFLAGS += -I$(LIB_EMBEDULAR_PATH)

# Initialization of properties later defined by the target.
TARGET_REQUIRE_BIN_IMAGE := no

# Remove duplicates and prepend proper relative path.
BUILD_LIBS := $(sort $(BUILD_LIBS))
BUILD_LIBS_RPATH := $(foreach name,$(BUILD_LIBS),lib/$(name))

# Default toolchain and build definitions.
BUILD_VCS ?= git
BUILD_TOOLCHAIN ?= gcc
BUILD_TARGET_RPATH := target/shortcut/$(BUILD_TARGET)

# Makefiles engaged in project building.
BUILD_MAKEFILES := $(BUILD_TARGET_RPATH) \
                   $(BUILD_LIBS_RPATH) \
                   vcs/$(BUILD_VCS) \
                   toolchain/$(BUILD_TOOLCHAIN)

# Initial project information.
$(call emb_info,App. name '$(APP_NAME)')
$(call emb_info,Requested target shortcut '$(BUILD_TARGET)')
$(call emb_info,Using makefiles '$(BUILD_MAKEFILES)')

# Recursive inclusion of build makefiles.
$(foreach name,$(BUILD_MAKEFILES),$(call emb_include,$(name).mk))

$(call emb_need_var,CHIP_MFR)
$(call emb_need_var,CHIP_FAMILY)
$(call emb_need_var,CHIP_MODEL)
$(call emb_need_var,CHIP_VARIANT)
$(call emb_need_var,CHIP)

CFLAGS += -D'EMBEDULAR_ARCH_CHIP_MFR=CC_Str($(CHIP_MFR))'
CFLAGS += -D'EMBEDULAR_ARCH_CHIP_FAMILY=CC_Str($(CHIP_FAMILY))'
CFLAGS += -D'EMBEDULAR_ARCH_CHIP_MODEL=CC_Str($(CHIP_MODEL))'
CFLAGS += -D'EMBEDULAR_ARCH_CHIP_VARIANT=CC_Str($(CHIP_VARIANT))'
CFLAGS += -D'EMBEDULAR_ARCH_CHIP=CC_Str($(CHIP))'

$(call emb_export_libs)
