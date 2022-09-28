/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  input switch action detection.

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

#include "embedul.ar/source/core/manager/input/action.h"
#include "embedul.ar/source/core/device/board.h"


void INPUT_ACTION_Reset (struct INPUT_ACTION *const S)
{
    BOARD_AssertParams (S);
    OBJECT_Clear (S);
}


enum INPUT_ACTION_Type INPUT_ACTION_Update (struct INPUT_ACTION *const S,
                                              const bool NewStatus)
{
    BOARD_AssertParams (S);

	const TIMER_Ticks   Now         = BOARD_TicksNow ();
	const bool          LastStatus  = S->currentStatus;

	S->currentStatus = NewStatus;

	// Sigue suelto (1)
	if (!LastStatus && !NewStatus)
	{
		// Timeout para detectar DoubleClicked
		if (S->lastClicked && Now >= S->lastClicked + 300)
		{
			S->lastClicked = 0;
			++ S->clickedCount;
			return (S->lastAction = INPUT_ACTION_Type_Clicked);
		}

		//return INPUT_ACTION_Type_None;
        return (S->lastAction = INPUT_ACTION_Type_None);
	}

	// Estaba suelto y se presiono (2)
	if (!LastStatus && NewStatus)
	{
		S->holdStarted  = Now;
		S->lastHold     = Now;
		return (S->lastAction = INPUT_ACTION_Type_None);
	}

	// Estaba presionado y se solto (3)
	if (LastStatus && !NewStatus)
	{
		// Debounce
		if (S->lastAction == INPUT_ACTION_Type_None)
		{
			return INPUT_ACTION_Type_None;
		}

		// Timeout para detectar Clicked
		if (Now < S->holdStarted + 400)
		{
			// El ultimo Click debio haberse hecho hace menos de 800 ms
			if (Now < S->lastClicked + 800)
			{
				S->lastClicked = 0;
				++ S->doubleClickedCount;
				return (S->lastAction = INPUT_ACTION_Type_DoubleClicked);
			}
			else {
				S->lastClicked = Now;
			}
		}
		// Se solto un Hold
		else if (S->lastAction == INPUT_ACTION_Type_OnHold)
		{
			++ S->releasedCount;
			return (S->lastAction = INPUT_ACTION_Type_Released);
		}

        return INPUT_ACTION_Type_None;
        // return (b->lastAction = INPUT_ACTION_Type_None);
	}

	// Sigue presionado (4) (LastStatus && NewStatus)
	// Pressed con debounce
	if (S->lastAction == INPUT_ACTION_Type_None
                && Now > S->holdStarted + 50)
	{
		++ S->pressedCount;
		return (S->lastAction = INPUT_ACTION_Type_Pressed);
	}
	else if (S->lastAction == INPUT_ACTION_Type_Pressed
                && Now > S->holdStarted + 1000)
	{
		++ S->onHoldCount;
		return (S->lastAction = INPUT_ACTION_Type_OnHold);
	}
	else if (S->lastAction == INPUT_ACTION_Type_OnHold)
	{
		// Devuelve OnHold cada vez que se actualiza lastHold
		S->lastHold = Now;
		return INPUT_ACTION_Type_OnHold;
	}

	return INPUT_ACTION_Type_None;
   //return (b->lastAction = INPUT_ACTION_Type_None);
}


enum INPUT_ACTION_Type INPUT_ACTION_Last (struct INPUT_ACTION *const S)
{
    BOARD_AssertParams (S);
    return S->lastAction;
}
