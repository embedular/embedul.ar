#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# simd-oriented fast mersenne twister library.
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

# ------------------------------------------------------------------------------
# Library config options (LIB_SFMT_CONFIG_*)
# ------------------------------------------------------------------------------
# MEXP: defaults to the smallest available, suitable for memory
#		constrained embedded systems.
# ------------------------------------------------------------------------------

$(call emb_declare_lib,$\
	Fast Mersenne Twister,$\
	LIB_SFMT,$\
	$(LIB_EMBEDULAR_ROOT)/source/3rd_party/sfmt-1.5.1,$\
	MEXP=607 \
	)


$(call emb_info,Using Fast Mersenne Twister with period '$(LIB_SFMT_CONFIG_MEXP)')

CFLAGS += -I$(LIB_SFMT)
CFLAGS += -DSFMT_MEXP=$(LIB_SFMT_CONFIG_MEXP)

OBJS += $(LIB_SFMT)/SFMT.o
