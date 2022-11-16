#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# freertos library.
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

$(call emb_need_var,LIB_EMBEDULAR_ROOT)
$(call emb_need_var,CPU_MODEL)

# ------------------------------------------------------------------------------
# Library config options (LIB_FREERTOS_CONFIG_*)
# ------------------------------------------------------------------------------
# HEAP_SCHEME		: FreeRTOS heap allocation scheme number (see OS doc).
# INCLUDE_SOURCES	: Base FreeRTOS sources, ommiting file extension. Should
#					  match the enabled subsystems in FreeRTOS_Config.h
# ------------------------------------------------------------------------------

$(call emb_declare_lib,$\
	FreeRTOS,$\
	LIB_FREERTOS,$\
	$(LIB_EMBEDULAR_ROOT)/source/3rd_party/freertos-10.4.6,$\
	HEAP_SCHEME=4 \
	INCLUDE_SOURCES=tasks|list|queue|timers|event_groups|croutine|stream_buffer \
	)


$(call emb_split_enum_to_list,$(LIB_FREERTOS_CONFIG_INCLUDE_SOURCES),LIB_FREERTOS_SOURCES)

ifneq ($(findstring cortex-m,$(CPU_MODEL)),)
    # use ARM_CM4F on an r0p0 (CHIP_REV?)
    ifeq ($(CPU_MODEL),cortex-m7)
        LIB_FREERTOS_PORT := GCC/ARM_CM7/r0p1
    else
        ifeq ($(CPU_MODEL),cortex-m4)
            LIB_FREERTOS_PORT := GCC/ARM_CM4F
        else
            ifeq ($(CPU_MODEL),cortex-m0)
                LIB_FREERTOS_PORT := GCC/ARM_CM0
            else
                $(call emb_error,Unknown Cortex-M architecture for CPU_MODEL '$(CPU_MODEL)')
            endif
        endif
    endif
else
    ifeq ($(CPU_MODEL),generic)
        LIB_FREERTOS_PORT := ThirdParty/GCC/Posix
        CFLAGS += -I$(LIB_FREERTOS_PORT)/utils
        CFLAGS += -D'projCOVERAGE_TEST=0'
        OBJS += $(LIB_FREERTOS)/portable/$(LIB_FREERTOS_PORT)/utils/wait_for_event.o
        CFLAGS += -pthread
    else
        $(call emb_error,Unknown architecture for CPU_MODEL '$(CPU_MODEL)')
	endif
endif

$(call emb_info,Using heap scheme '$(LIB_FREERTOS_CONFIG_HEAP_SCHEME)')
$(call emb_info,Using port '$(LIB_FREERTOS_PORT)')
$(call emb_info,Using sources '$(LIB_FREERTOS_SOURCES)')

CFLAGS += -I$(LIB_FREERTOS)/include
CFLAGS += -I$(LIB_FREERTOS)/portable/$(LIB_FREERTOS_PORT)

OBJS += $(LIB_FREERTOS)/portable/$(LIB_FREERTOS_PORT)/port.o
#OBJS += $(LIB_FREERTOS)/portable/MemMang/heap_$(LIB_FREERTOS_CONFIG_HEAP_SCHEME).o

$(foreach name,$(LIB_FREERTOS_SOURCES),$(eval OBJS += $(LIB_FREERTOS)/$(name).o))
