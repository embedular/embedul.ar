#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# git version-control system.
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

BUILD_VCS_EXE := git
BUILD_VCS_BRANCH_CMD := rev-parse --abbrev-ref HEAD
BUILD_VCS_TAG_CMD := describe --tags --always

# $(1) Description
# $(2) Variable Code
define vcs_get_failed =
    $(call emb_info,$(1) does not use GIT)
	$(eval CFLAGS += -D'CC_Vcs$(2)VersionStr="No GIT"')
endef

# $(1) Description
# $(2) Variable Code
define vcs_get_ok =
	$(eval BUILD_VCS_VERSION := $(BUILD_VCS_BRANCH)/$(BUILD_VCS_TAG))
    $(call emb_info,$(1) version: '$(BUILD_VCS_VERSION)')
    $(eval CFLAGS += -D'CC_Vcs$(2)VersionStr=CC_Str($(BUILD_VCS_VERSION))')
endef

# $(1) Description
# $(2) Variable Code
# $(3) Path
define vcs_get_version =
    $(eval BUILD_VCS_BRANCH := $(shell cd $(3) && $(BUILD_VCS_EXE) \
                                       $(BUILD_VCS_BRANCH_CMD) 2>/dev/null))
    $(eval BUILD_VCS_TAG := $(shell cd $(3) && $(BUILD_VCS_EXE) \
	                                      $(BUILD_VCS_TAG_CMD) 2>/dev/null))
    $(if $(filter $(.SHELLSTATUS),0),\
              $(call vcs_get_ok,$(1),$(2)),$(call vcs_get_failed,$(1),$(2)))
endef

$(call vcs_get_version,Application,App,.)
$(call vcs_get_version,Framework,Fwk,$(LIB_EMBEDULAR_ROOT))
