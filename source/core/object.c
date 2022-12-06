/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [CORE] object instance identification.

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

#include "embedul.ar/source/core/object.h"
#include "embedul.ar/source/core/device/board.h"


struct NOBJ {} * NOBJ = (struct NOBJ *) 0x0;
struct TSKN {} * TSKN = (struct TSKN *) 0x1;


const char * OBJECT_INFO_Type (struct OBJECT_INFO *O)
{
    BOARD_AssertParams (O);
    return O->Type;
}


const char * OBJECT_INFO_Description (struct OBJECT_INFO *O)
{
    BOARD_AssertParams (O);
    return O->Description;
}


const void * OBJECT_INFO_Ptr (struct OBJECT_INFO *O)
{
    BOARD_AssertParams (O);
    return O->Ptr;
}
