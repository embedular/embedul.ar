#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# gcc common toolchain selection and options.
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

$(call emb_must_precede,config.mk)
$(call emb_need_var,CPU_MODEL)

$(call emb_info,GNU toolchain selected)

ifneq ($(CHIP),)
    $(call emb_info,CHIP is '$(CHIP)')
    # Specific CHIP based directives
    ifneq ($($(CHIP)_CFLAGS),)
        $(call emb_info,Appending $(words $($(CHIP)_CFLAGS)) CHIP-based CFLAGS directives)
        CFLAGS += $($(CHIP)_CFLAGS)
    endif
endif

$(call emb_info,CPU model is '$(CPU_MODEL)')

ifneq ($(findstring cortex-m,$(CPU_MODEL)),)
    $(call emb_include,toolchain/gcc/cross-cortex-m.mk)
endif

CLIBS += c gcc m

CROSS_COMPILE ?=
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
GDB := $(CROSS_COMPILE)gdb
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump
SIZE := $(CROSS_COMPILE)size

COMPILE_CMD ?= $(CC) -o $(2) -c $(1) $(CFLAGS)
## gcc (not ld) calls collect2 and automatically adds required CRT
## objects on hosted (non no-sys) platforms
LINK_CMD ?= $(CC) $(CFLAGS) $(OBJS) $(APP_OBJS) $(foreach flag,$(LDFLAGS),-Wl$(emb_comma)$(flag)) -o $(1)

# GCC version
$(call emb_info,Toolchain version '$(shell $(CC) -dumpversion)')

# Basic configurations
CFLAGS += -std=c17 -Wall -Wextra
CFLAGS += -O$(OLEVEL)

# Expose strnlen
CFLAGS += -D_POSIX_C_SOURCE=200809L

# Debug information
ifeq ($(DEBUG),yes)
    CFLAGS += -g$(GLEVEL) -DDEBUG
else
    # -flto
   # CFLAGS += -ffast-math
   # CFLAGS += -fno-common
endif

CFLAGS += -D'CC_NoOptimization=__attribute__((optimize("O0")))'
CFLAGS += -D'CC_Fallthrough=__attribute__((fallthrough))'
CFLAGS += -D'CC_Packed=__attribute__((packed))'
CFLAGS += -D'CC_Weak=__attribute__((weak))'
CFLAGS += -D'CC_Section(x)=__attribute__((section(x)))'
CFLAGS += -D'AS_Section(x)=.section x'

CFLAGS += -D'CC_CompilerStr="GCC "CC_ExpStr(__GNUC__)"."CC_ExpStr(__GNUC_MINOR__)"."CC_ExpStr(__GNUC_PATCHLEVEL__)'
CFLAGS += -D'CC_OptLevelStr="-O"CC_Str($(OLEVEL))'
CFLAGS += -D'CC_DateStr=CC_Str($(BUILD_DATE))'

$(call emb_info,DEBUG '$(DEBUG)'$(emb_comma) OLEVEL '$(OLEVEL)')

# Libraries
define get_library_path
    $(shell dirname $(shell $(CC) $(CFLAGS) -print-file-name=$(1)))
endef

$(foreach name,$(CLIBS), $(eval LDFLAGS += -L $(call get_library_path,lib$(name).a)))
$(foreach name,$(CLIBS), $(eval LIBS += -l$(name)))
