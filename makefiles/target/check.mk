#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# target variables check.
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

$(call emb_need_var,TARGET_NAME)
$(call emb_need_var,TARGET_ARCH)
$(call emb_need_var,TARGET_MFR)
$(call emb_need_var,TARGET_FAMILY)
$(call emb_need_var,TARGET_DRIVERS)
$(call emb_need_var,TARGET_BSP)
$(call emb_need_var,TARGET_BSP_BOARD)

$(call emb_info,Using target '$(TARGET_NAME)')

$(call mk_need_dir,$(TARGET_ARCH),Arch)
$(call mk_need_dir,$(TARGET_MFR),Manufacturer)
$(call mk_need_dir,$(TARGET_FAMILY),Family)
$(call mk_need_dir,$(TARGET_DRIVERS),Drivers)
$(call mk_need_dir,$(TARGET_BSP),BSP)
$(call mk_need_dir,$(TARGET_BSP_BOARD),BSP board)

# Board BSP
CFLAGS += -I$(TARGET_BSP)
# Specific board code
CFLAGS += -I$(TARGET_BSP_BOARD)
