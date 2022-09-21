#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# openocd debug/flash adaptor for elf executables.
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
$(call emb_need_var,OPENOCD_CFG)

OPENOCD_EXE ?= openocd

flash: prog
    ifeq (, $(shell which $(OPENOCD_EXE)))
	$(call emb_error,OpenOCD executable '$(OPENOCD_EXE)' not found)
    endif

	openocd -f $(OPENOCD_CFG) \
	        -c "init" \
			-c "reset" \
			-c "halt 200" \
	        -c "flash write_image erase $(EXECUTABLE) 0 elf" \
	        -c "flash verify_image $(EXECUTABLE) 0 elf" \
	        -c "reset run" \
	        -c "shutdown"
