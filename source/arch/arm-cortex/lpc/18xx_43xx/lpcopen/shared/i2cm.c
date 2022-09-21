/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  lpcopen board - i2c controller mode.

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

#include "embedul.ar/source/arch/arm-cortex/lpc/18xx_43xx/lpcopen/shared/i2cm.h"


uint8_t Board_I2CMTransfer (LPC_I2C_T* i2c, uint8_t slaveAddr,
                            const uint8_t* txData, uint32_t txSize,
                            uint8_t* rxData, uint32_t rxSize)
{
    I2CM_XFER_T xfer;

    xfer.slaveAddr  = slaveAddr;
    xfer.options    = 0;
    xfer.status     = 0;
    xfer.txBuff     = txData;
    xfer.txSz       = (uint16_t) txSize;
    xfer.rxBuff     = rxData;
    xfer.rxSz       = (uint16_t) rxSize;

    Chip_I2CM_XferBlocking (i2c, &xfer);

    // I2CM_STATUS_OK on success.
    return (uint8_t) xfer.status;
}
