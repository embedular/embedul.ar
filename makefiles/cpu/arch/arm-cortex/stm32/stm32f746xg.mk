#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# stm32f746xg cortex-m7f core definition.
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

$(call emb_declare_chip,stm32,f7,46,xg)
$(call emb_include,cpu/arch/arm-cortex/m7f-sp.mk)

FLASH_BASE ?= 0x08000000

# Required by STM32Cube
CFLAGS += -DSTM32F746xx
