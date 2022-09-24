#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# sdl library.
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

# ------------------------------------------------------------------------------
# Library config options (LIB_SDL_CONFIG_*)
# ------------------------------------------------------------------------------
# No options.
# ------------------------------------------------------------------------------

$(call emb_declare_lib,$\
    Simple DirectMedia Layer,$\
    LIB_SDL,$\
    ,$\
    )

LIB_SDL_CONFIG_EXE ?= sdl2-config
LIB_SDL_VERSION := $(shell $(LIB_SDL_CONFIG_EXE) --version)
LIB_SDL_CFLAGS ?= $(shell $(LIB_SDL_CONFIG_EXE) --cflags)
LIB_SDL_LDFLAGS ?= $(shell $(LIB_SDL_CONFIG_EXE) --libs)

$(call emb_info,Version '$(LIB_SDL_VERSION)')
$(call emb_info,Using CFLAGS '$(LIB_SDL_CFLAGS)')
$(call emb_info,Using LDFLAGS '$(LIB_SDL_LDFLAGS)')

CFLAGS += $(LIB_SDL_CFLAGS)
CFLAGS += -D'LIB_SDL_VERSION_STR="$(LIB_SDL_VERSION)"'

LDFLAGS += -lm
LDFLAGS += $(LIB_SDL_LDFLAGS)
LDFLAGS += -lSDL2_mixer
