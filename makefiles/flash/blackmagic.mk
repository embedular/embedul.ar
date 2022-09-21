#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# blackmagic debug/flash adaptor.
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

$(call emb_must_precede,build.mk)
$(call emb_need_var,EXECUTABLE)

BLACKMAGIC_DEV ?= /dev/ttyBlackMagicGdb

flash: prog
    ifeq (, $(wildcard $(BLACKMAGIC_DEV)))
	$(call emb_error,Black magic probe device '$(BLACKMAGIC_DEV)' not found)
    endif

	$(GDB) --exec=$(EXECUTABLE) --batch \
	--eval-command="target extended-remote $(BLACKMAGIC_DEV)" \
	--eval-command="monitor swdp_scan" \
	--eval-command="attach 1" \
	--eval-command="load" \
	--eval-command="compare-sections" \
	--eval-command="detach" \
	--eval-command="kill"
