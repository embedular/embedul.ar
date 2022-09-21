#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# build rules.
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

$(call emb_must_precede,Makefile)

$(if $(strip $(BUILD_MAKEFILES)),,$(call emb_info,Build not configured yet) $\
    $(call emb_include,config.mk))

$(call emb_need_var,LIB_EMBEDULAR_ROOT)
$(call emb_need_var,APP_NAME)
$(call emb_need_var,APP_ROOT)
$(call emb_need_var,CPU_MODEL)
$(call emb_need_var,FLASH_TOOL)
$(call emb_need_var,COMPILE_CMD)
$(call emb_need_var,LINK_CMD)
$(call emb_need_var,TARGET_NAME)
$(call emb_need_var,TARGET_REQUIRE_BIN_IMAGE)


ifeq ($(DEBUG),yes)
    BUILD_ROOT ?= \
        $(APP_ROOT)/build/binary/$(TARGET_NAME)/$(CC)/debug_$(CPU_MODEL)_O$(OLEVEL)
else
    BUILD_ROOT ?= \
        $(APP_ROOT)/build/binary/$(TARGET_NAME)/$(CC)/$(CPU_MODEL)_O$(OLEVEL)
endif

$(call emb_info,Build root '$(BUILD_ROOT)')

EXECUTABLE := $(BUILD_ROOT)/$(APP_NAME).elf
BIN_IMAGE := $(BUILD_ROOT)/$(APP_NAME).bin
HEX_IMAGE := $(BUILD_ROOT)/$(APP_NAME).hex
EXE_LISTING := $(BUILD_ROOT)/$(APP_NAME).lst
EXE_MAP := $(BUILD_ROOT)/$(APP_NAME).map

ifneq (yes,$(VERBOSE))
    VN := @
endif

define compile =
    $(VN)mkdir -p $(3)
    $(if $(VN), $(info $(emb_term_blue)[$(4)]$(emb_term_sgr0) $(1)))
    $(VN)$(COMPILE_CMD)
endef

define link =
    $(if $(VN), $(info $(emb_term_cyan)[LD]$(emb_term_sgr0) $(1)))
    $(VN)$(LINK_CMD)
endef

define remove =
    $(if $(VN), $(info $(emb_term_magenta)[RM]$(emb_term_sgr0) $(1)))    
    $(if $(findstring $(BUILD_ROOT),$(1)),,$(error $(emb_term_red)SAFETY SWITCH:\
        File not in BUILD_ROOT$(emb_term_sgr0)))
    $(VN)rm -f $(1)
endef

define execute =
    $(if $(VN), $(info $(emb_term_cyan)[EX]$(emb_term_sgr0) $(1)))
    $(VN)stdbuf -o0 $(1)
endef

# Relative framework paths to relative current build
OBJS := $(foreach fp,$(OBJS),$(abspath $(fp)))
OBJS := $(patsubst $(abspath $(LIB_EMBEDULAR_ROOT))/%,$(BUILD_ROOT)/embedul.ar/%,$(OBJS))

APP_OBJS := $(foreach fp,$(APP_OBJS),$(abspath $(fp)))
APP_OBJS := $(patsubst $(abspath $(APP_ROOT))/%,$(BUILD_ROOT)/$(APP_NAME)/%,$(APP_OBJS))

ifeq ($(TARGET_REQUIRE_BIN_IMAGE),yes)
    TARGET_PROG := $(BIN_IMAGE)
    LDFLAGS += -Map=$(EXE_MAP)
else
    TARGET_PROG := $(EXECUTABLE)
endif

prog: clean
prog: $(OBJS)
prog: $(APP_OBJS)
prog: $(TARGET_PROG)

all: prog

# debajo del ultimo OBJCOPY: $(OBJDUMP) -h -S -D $(EXECUTABLE) > $(EXE_LISTING)
$(BIN_IMAGE): $(EXECUTABLE)
	$(OBJCOPY) -v -O binary $^ $@
	$(OBJCOPY) -O ihex $^ $(HEX_IMAGE)
	$(SIZE) $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
$(EXECUTABLE): $(APP_OBJS)
	$(call link,$@)

$(BUILD_ROOT)/embedul.ar/%.o: $(LIB_EMBEDULAR_ROOT)/%.c
	$(call compile,$<,$@,$(@D),CC)

$(BUILD_ROOT)/embedul.ar/%.o: $(LIB_EMBEDULAR_ROOT)/%.S
	$(call compile,$<,$@,$(@D),AS)

$(BUILD_ROOT)/embedul.ar/%.o: $(LIB_EMBEDULAR_ROOT)/%.s
	$(call compile,$<,$@,$(@D),AS)

$(BUILD_ROOT)/$(APP_NAME)/%.o: $(APP_ROOT)/%.c
	$(call compile,$<,$@,$(@D),CC)

$(BUILD_ROOT)/$(APP_NAME)/%.o: $(APP_ROOT)/%.S
	$(call compile,$<,$@,$(@D),AS)

$(BUILD_ROOT)/$(APP_NAME)/%.o: $(APP_ROOT)/%.s
	$(call compile,$<,$@,$(@D),AS)


clean:
	$(foreach fp,$(OBJS),$(call remove,$(fp)))
	$(foreach fp,$(APP_OBJS),$(call remove,$(fp)))
	$(call remove,$(EXECUTABLE))
	$(call remove,$(BIN_IMAGE))
	$(call remove,$(HEX_IMAGE))
	$(call remove,$(EXE_LISTING))
	$(call remove,$(EXE_MAP))

run:
	$(call execute,$(EXECUTABLE))


# include required flash build directive
$(call emb_info,Flash tool '$(FLASH_TOOL)')
$(call emb_include,flash/$(FLASH_TOOL).mk)

.PHONY: clean
