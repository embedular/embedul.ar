#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# generic cortex-m7f/armv7e-m core definition.
# floating-point unit: single-precision only, ieee-754 compliant.
# vfpv5 extension.
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

$(call emb_include,cpu/arch/arm-cortex/m7.mk)

CFLAGS += -mfpu=fpv5-sp-d16
# -fsingle-precision-constant
CFLAGS += -Wdouble-promotion -Wfloat-conversion

M7F_FPU_MODE ?= hard

ifeq ($(M7F_FPU_MODE),soft)
    # Uses the FPU, but parameters are passed in core registers
    # (as in -mfloat-abi=soft)
    CFLAGS += -mfloat-abi=softfp
else
    ifeq ($(M7F_FPU_MODE),hard)
        # Uses the FPU, and passes the parameters in FPU registers, not core 
        # registers
        CFLAGS += -mfloat-abi=hard
    else
        $(call emb_error,Invalid M7F_FPU_MODE '$(M7F_FPU_MODE)')
    endif
endif
