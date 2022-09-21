/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  lpcopen board - gpio handling.

  Copyright 2018-2022 Santiago Germino
  <sgermino@embedul.ar> https://www.linkedin.com/in/royconejo

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#pragma once


#define BOARD_GPIO_HIGH                     1
#define BOARD_GPIO_LOW                      0

#define BOARD_GPIO_ENABLED                  1
#define BOARD_GPIO_DISABLED                 0

#define BOARD_GPIO_FAST_ACCESS(name) \
                    *((volatile uint8_t *)(LPC_GPIO_PORT_BASE + \
                        (BOARD_GPIO_ ## name ## _PORT << 5) + \
                            BOARD_GPIO_ ## name ## _PIN))

#define BOARD_GPIO_FAST_WRITE(name,value) \
                    (BOARD_GPIO_FAST_ACCESS(name) = value)

// Returns BOARD_GPIO_ENABLED when state == _EN, BOARD_GPIO_DISABLED otherwise.
#define BOARD_GPIO_GET_STATE(name) \
                    (BOARD_GPIO_FAST_ACCESS(name) == BOARD_GPIO_ ## name ## _EN)

// Set _EN when ENABLED or !EN for DISABLED.
#define BOARD_GPIO_SET_STATE(name,en) \
                    BOARD_GPIO_FAST_WRITE(name, \
                        BOARD_GPIO_ ## en ? BOARD_GPIO_ ## name ## _EN : \
                            !(BOARD_GPIO_ ## name ## _EN))

#define BOARD_GPIO_TOGGLE_STATE(name) \
                    BOARD_GPIO_FAST_WRITE(name, !BOARD_GPIO_FAST_ACCESS(name))

// Convenience macros
#define BOARD_LED_ON(name) \
                    BOARD_GPIO_SET_STATE(OUT_LED_ ## name, ENABLED)

#define BOARD_LED_OFF(name) \
                    BOARD_GPIO_SET_STATE(OUT_LED_ ## name, DISABLED)

#define BOARD_LED_TOGGLE(name) \
                    BOARD_GPIO_TOGGLE_STATE(OUT_LED_ ## name)

#define BOARD_LED_GET_STATE(name) \
                    BOARD_GPIO_GET_STATE(OUT_LED_ ## name)

#define BOARD_SWITCH_GET_STATE(name) \
                    BOARD_GPIO_GET_STATE(IN_SWITCH_ ## name)

