#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# gcc arm thumb-2 cross-compiler options.
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

$(call emb_must_precede,gcc.mk)

CLIBS += nosys

# Toolchain configuration (GNU Arm Embedded Toolchain, bare-metal).
CROSS_COMPILE ?= arm-none-eabi-

# Need to call ld directly to avoid CRT objects added by gcc (collect2)
# over non hosted (nosys) embedded systems
LINK_CMD = $(LD) $(OBJS) $(APP_OBJS) --start-group $(LIBS) --end-group $(LDFLAGS) -o $(1)

$(call emb_info,TARGET '$(TARGET_NAME)' is cross-compiled)
$(call emb_info,Cross compiler toolchain prefix '$(CROSS_COMPILE)')
$(call emb_info,Freestanding (bare metal) target)

# Basic configurations
CFLAGS += -ggdb3
CFLAGS += --specs=nosys.specs
CFLAGS += -ffreestanding -nostartfiles -nodefaultlibs -nostdlib
CFLAGS += -mthumb -mcpu=$(CPU_MODEL)
CFLAGS += -fdata-sections -ffunction-sections

LDFLAGS += --gc-sections --print-memory-usage
