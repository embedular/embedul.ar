#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# build system definitions.
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

emb_term_sgr0 := $(shell tput sgr0)
emb_term_red := $(shell tput setaf 1)
emb_term_green := $(shell tput setaf 2)
emb_term_yellow := $(shell tput setaf 3)
emb_term_blue := $(shell tput setaf 4)
emb_term_magenta := $(shell tput setaf 5)
emb_term_cyan := $(shell tput setaf 6)
emb_term_bold := $(shell tput bold)
emb_parb := (
emb_pare := )
emb_level_sep := |
emb_indent_mark := _ ____ _______ __________ _____________ ________________\
                   ___________________ ______________________\
                   _________________________ ____________________________\
                   _______________________________
emb_null :=
emb_space := $(emb_null) $(emb_null)
emb_comma := ,

emb_nth_file = $(notdir $(word $(1), $(MAKEFILE_LIST)))
emb_self_file = $(notdir $(lastword $(MAKEFILE_LIST)))
emb_self_name = $(basename $(call emb_self_file))
emb_indent = $(subst _,$(emb_space),$(word $(words $(emb_file_stack)),$\
             $(emb_indent_mark)))

# Returns the nth word divided by a single '-' character in the string.
# $(1) String.
# $(2) Get nth word.
emb_split_get = $(word $(2),$(subst -,$(emb_space),$(1)))

# Begin of newline (do not modify, leave spaces as-is)
define emb_nl


endef
# End of newline (do not modify, leave spaces as-is)

define emb_info
    $(info $(emb_term_green)[MK]$(emb_term_sgr0)$\
        $(call emb_indent)$(emb_parb)$(emb_current_file)$(emb_pare) $(1))
endef

define emb_warning
    $(info $(emb_term_yellow)[MK]$(emb_term_sgr0)$\
        $(call emb_indent)$(emb_parb)$(emb_current_file)$(emb_pare) $\
		    $(emb_term_yellow)[WARNING] $(1)$(emb_term_sgr0))
endef

define emb_error
    $(error $(emb_nl)$(emb_term_red)[MK]$(emb_term_sgr0)$\
        $(call emb_indent)$(emb_parb)$(emb_current_file)$(emb_pare) $\
		    $(emb_term_red)[ERROR] $(1)$(emb_term_sgr0))
endef

# Error when variable $(1) is undefined or empty.
define emb_need_var
    $(if $(value $(1)),,$(call emb_error,Undefined variable '$(1)'))
endef

# $(1): Directory path to test.
# $(2): Contents description.
define emb_need_dir
    $(if $(wildcard $(1)),,$(call emb_error,$(2) not found in '$(1)'))
endef

# A top-level (L1) .mk must call this function to initialize the calling stack
define emb_top
    $(eval emb_last_file := )
    $(eval emb_current_file := 0$(emb_level_sep)$(emb_term_bold)$\
        $(call emb_nth_file,1)$(emb_term_sgr0))
    $(eval emb_file_stack := $(emb_current_file))
    $(call emb_info,)
    $(eval emb_current_file := 1$(emb_level_sep)$(emb_term_bold)$\
        $(call emb_nth_file,2)$(emb_term_sgr0))
    $(eval emb_last_file := 0$(emb_level_sep)$(emb_term_bold)$\
        $(call emb_nth_file,1)$(emb_term_sgr0))
    $(eval emb_file_stack := $(emb_last_file) $(emb_current_file))
endef

# A top-level (L1) .mk must call this function to remove itself from the calling
# stack
define emb_end
    $(call emb_info,EOF)
    $(eval emb_file_stack = $(filter-out $(emb_current_file),$(emb_file_stack)))
    $(eval emb_last_file = $(emb_current_file))
    $(eval emb_current_file = $(lastword $(emb_file_stack)))
endef

# $(1): file or list of files from where it is valid to include the callee.
define emb_must_precede
    $(eval emb_tmp := )
    $(foreach file,$(1),$(eval emb_tmp += %$(emb_level_sep)$(emb_term_bold)$\
        $(file)$(emb_term_sgr0)))
    $(if $(filter $(emb_tmp),$(emb_last_file)),,$(call emb_error,$\
        Invalid precedence '$(emb_last_file)'))
endef

# $(1): library path.
# $(2): library human readable name.
define emb_need_lib
    $(call emb_info,Locate '$(2)' in '$(1)')
    $(call emb_need_dir,$(1),$(2) library)
endef

# $(1): library variable name.
# $(2): list name.
# $(3): config option, not prefixed.
define emb_create_lib_list
	$(eval tmp := $(subst =,$(emb_space),$(3)))
	$(eval $(1)_$(2)_CONFIG += $(word 1, $(tmp)))
	$(eval $(1)_CONFIG_$(word 1, $(tmp)) := $(word 2, $(tmp)))
	$(call emb_need_var,$(1)_CONFIG_$(word 1, $(tmp)))
endef

# $(1): library human readable name.
# $(2): library variable name.
# $(3): library path.
# $(4): library default config params.
define emb_declare_lib
	$(call emb_info,Declaring library '$(1)')
    $(eval $(2) ?= $(3))
    $(if $(3),$(call emb_need_lib,$($(2)),$(1)),$(call emb_info,'$(1)' is a system-wide library))
    $(eval $(foreach var,$(4),$(call emb_create_lib_list,$(2),DEFAULT,$(var))))
    $(eval $(foreach var,$($(2)_CONFIG),$(call emb_create_lib_list,$(2),USER,$(var))))	
	$(eval $(2)_INVALID_CONFIG := $(filter-out $($(2)_DEFAULT_CONFIG),$($(2)_USER_CONFIG)))
	$(if $($(2)_INVALID_CONFIG),$(call emb_error,$\
        Invalid config params '$($(2)_INVALID_CONFIG)'),)
	$(eval $(2)_NO_USER_CONFIG := $(filter-out $($(2)_USER_CONFIG),$($(2)_DEFAULT_CONFIG)))
	$(eval emb_declared_libs += $(2))
endef

# $(1): enum: one|two|three|four.
# $(2): splitted list name.
define emb_split_enum_to_list
	$(eval $(2) := $(subst |,$(emb_space),$(1)))
endef

# $(1): chip manufacturer (stm32, lpc, etc).
# $(2): chip family.
# $(3): chip model (or sub-family).
# $(4): chip variant (memory/footprint code).
define emb_declare_chip
	$(call emb_info,Declaring chip manufacturer '$(1)'$(emb_comma) family '$(2)'$(emb_comma) model '$(3)'$(emb_comma) variant '$(4)')
    $(eval CHIP_MFR := $(1))
    $(eval CHIP_FAMILY := $(2))
    $(eval CHIP_MODEL := $(3))
    $(eval CHIP_VARIANT := $(4))
    $(eval CHIP := $(1)$(2)$(3)$(4))
endef

# $(1): library variable name.
# $(2): config option, not prefixed (no {LIB_libname_CONFIG_}
define emb_export_lib_cflags
	$(eval tmp := $(1)_CONFIG_$(2)=$($(1)_CONFIG_$(2)))
    $(call emb_info,Config param '$(2)=$($(1)_CONFIG_$(2))')
	$(eval CFLAGS += -D'$(tmp)')
endef

# $(1): library variable name.
define emb_export_lib
	$(call emb_info,$(emb_term_bold)$(1)$(emb_term_sgr0))
	$(call emb_info,Exporting '$(words $($(1)_USER_CONFIG))' user defined$(emb_comma) '$(words $($(1)_NO_USER_CONFIG))' default config params)
    $(eval $(foreach var,$($(1)_USER_CONFIG),$(call emb_export_lib_cflags,$(1),$(var))))
    $(eval $(foreach var,$($(1)_NO_USER_CONFIG),$(call emb_export_lib_cflags,$(1),$(var))))
endef

define emb_export_libs
	$(eval $(foreach var,$(emb_declared_libs),$(call emb_export_lib,$(var))))
endef

define emb_include
    $(if $(wildcard $(LIB_EMBEDULAR_ROOT)/makefiles/$(1)),,$(call emb_error,$\
        Makefile not found '$(1)'))
    $(call emb_info,Including '$(1)')
    $(eval emb_last_file = $(emb_current_file))
    $(eval emb_current_file = $(words $(emb_file_stack))$(emb_level_sep)$\
        $(emb_term_bold)$(notdir $(1))$(emb_term_sgr0))
    $(eval emb_file_stack = $(emb_file_stack) $(emb_current_file))
    $(eval include $(LIB_EMBEDULAR_ROOT)/makefiles/$(1))
    $(call emb_end)
endef

define BUILD
    $(call emb_include,build.mk)
    $(call emb_end)
endef

define CONFIG
    $(call emb_include,config.mk)
endef


$(call emb_top)
$(call emb_must_precede,Makefile)

$(call emb_info,                  _________)
$(call emb_info,                 /\       /\)
$(call emb_info,                /  \_____/  \)
$(call emb_info,               /   /\   '\   \)
$(call emb_info,              /___/..\_'__\___\)
$(call emb_info,              \   \  / '  /   /)
$(call emb_info,               \   \/___'/   /)
$(call emb_info,                \  /     \  /)
$(call emb_info,                 \/_______\/)
$(call emb_info,)
$(call emb_info,          |             |     |)
$(call emb_info,$(emb_comma)---.$(emb_comma)-.-.|---.$(emb_comma)---.$(emb_comma)---|.   .|     $(emb_comma)---.$(emb_comma)---.)
$(call emb_info,|---'| | ||   ||---'|   ||   ||     $(emb_comma)---||)
$(call emb_info,`---'` ' '`---'`---'`---'`---'`---'o`---^`)
$(call emb_info,)
$(call emb_info,$(emb_term_bold)Welcome to http://embedul.ar build system$(emb_term_sgr0))

BUILD_DATE := $(shell date --iso=minutes)
$(call emb_info,Date/time '$(BUILD_DATE)')

# Framework base directory (where ./embedul.ar resides), environment variable.
#
# Calling system.mk with an undefined LIB_EMBEDULAR_PATH implies the caller
# already knows where to include system.mk. We assume that can only happen
# inside the framework, and the goal is to build framework examples.
#
# Applications reside outside the framework and must be independent of the
# framework installation path. Therefore, an application relies on a correctly
# set LIB_EMBEDULAR_PATH environment variable.
ifeq ($(LIB_EMBEDULAR_PATH),)
    $(call emb_warning,Undefined LIB_EMBEDULAR_PATH)
    $(call emb_warning,Assuming the build of "in-tree" framework examples)
    LIB_EMBEDULAR_PATH := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))../../)
endif

# Check framework base directory (where ./embedul.ar resides)
$(call emb_need_dir,$(LIB_EMBEDULAR_PATH),LIB_EMBEDULAR_PATH '$(LIB_EMBEDULAR_PATH)')

# Actual framework root
LIB_EMBEDULAR_ROOT := $(LIB_EMBEDULAR_PATH)/embedul.ar

# Check actual framework root
$(call emb_need_dir,$(LIB_EMBEDULAR_ROOT),LIB_EMBEDULAR_ROOT '$(LIB_EMBEDULAR_ROOT)')

$(call emb_info,Framework root '$(LIB_EMBEDULAR_ROOT)')
$(call emb_info,Build system ready)
$(call emb_end)
