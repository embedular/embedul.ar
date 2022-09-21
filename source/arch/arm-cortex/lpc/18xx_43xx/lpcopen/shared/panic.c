/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  lpcopen board - panic (no recoverable error).

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

#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/chip_18xx_43xx/chip.h"
#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/shared/api.h"


// Weak definition in cr_startup_lpc43xx-m4.c, no prototype.
void HardFault_Handler (void);


void Board_Panic (const char *const ErrorMsg)
{
    if (ErrorMsg)
    {
        Board_UARTPutSTR ("BOARD PANIC.\r\n");
        Board_UARTPutSTR ("Error: ");
        Board_UARTPutSTR (ErrorMsg);
        Board_UARTPutSTR ("\r\n");
    }

    #ifdef DEBUG
    __BKPT (0);
    #endif

    HardFault_Handler ();
}
