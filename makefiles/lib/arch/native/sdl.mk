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

SDL_CONFIG ?= sdl-config
SDL_CFLAGS ?= $(shell $(SDL_CONFIG) --cflags)
SDL_LDFLAGS ?= $(shell $(SDL_CONFIG) --static-libs)

$(call emb_info,SDL CFLAGS = '$(SDL_CFLAGS)')
$(call emb_info,SDL LDFLAGS = '$(SDL_LDFLAGS)')

CFLAGS += $(SDL_CFLAGS)

LDFLAGS += -lm
LDFLAGS += $(SDL_LDFLAGS)
LDFLAGS += -lSDL_mixer
