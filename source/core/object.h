/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [CORE] object id by type.

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

#include <stdint.h>


#define OBJECT_TYPE_SEPARATOR   ":"


#define OBJECT_IsValid(_p) \
    _Generic((_p), \
        struct OSWRAP *         : 1, \
        struct IO *             : 1, \
        struct RAWSTOR *        : 1, \
        struct STREAM *         : 1, \
        struct RANDOM *         : 1, \
        struct SOUND *          : 1, \
        struct TICKS *          : 1, \
        struct VIDEO *          : 1, \
        struct BOARD *          : 1, \
        struct LOG *            : 1, \
        struct OUTPUT *         : 1, \
        struct INPUT *          : 1, \
        struct MIO *            : 1, \
        struct COMM *           : 1, \
        struct STORAGE *        : 1, \
        struct SCREEN *         : 1, \
        struct ANIM *           : 1, \
        struct ARRAY *          : 1, \
        struct BITFIELD *       : 1, \
        struct CYCLIC *         : 1, \
        struct FSM *            : 1, \
        struct MEMPOOL *        : 1, \
        struct QUEUE *          : 1, \
        struct QUEUE_TRV *      : 1, \
        struct SEQUENCE *       : 1, \
        struct VARIANT *        : 1, \
        struct INPUT_ACTION *   : 1, \
        struct SCREEN_DOTMAP *  : 1, \
        struct SCREEN_FADE *    : 1, \
        struct SCREEN_FONT *    : 1, \
        struct RGB332_Gradient *: 1, \
        struct SCREEN_SPRITE *  : 1, \
        struct SCREEN_TILEMAP * : 1, \
        struct TSKN *           : 1, \
        struct NOBJ *           : 1, \
        struct OBJECT_INFO *    : 1, \
        default                 : 0 \
    )


#define OBJECT_AssertValid(_p) \
    { _Static_assert(OBJECT_IsValid(_p), "invalid object"); }


#define OBJECT_Clear(_p) \
    OBJECT_AssertValid (_p); \
    if (_p) { memset (_p, 0, sizeof(*_p)); }


#define OBJECT_Type(_p) \
    _Generic((_p), \
        struct OSWRAP *         : "dev" OBJECT_TYPE_SEPARATOR "oswrap", \
        struct IO *             : "dev" OBJECT_TYPE_SEPARATOR "io", \
        struct RAWSTOR *        : "dev" OBJECT_TYPE_SEPARATOR "rawstor", \
        struct STREAM *         : "dev" OBJECT_TYPE_SEPARATOR "stream", \
        struct RANDOM *         : "dev" OBJECT_TYPE_SEPARATOR "random", \
        struct SOUND *          : "dev" OBJECT_TYPE_SEPARATOR "sound", \
        struct TICKS *          : "dev" OBJECT_TYPE_SEPARATOR "ticks", \
        struct VIDEO *          : "dev" OBJECT_TYPE_SEPARATOR "video", \
        struct BOARD *          : "dev" OBJECT_TYPE_SEPARATOR "board", \
        struct LOG *            : "manager", \
        struct OUTPUT *         : "manager", \
        struct INPUT *          : "manager", \
        struct MIO *            : "manager", \
        struct COMM *           : "manager", \
        struct STORAGE *        : "manager", \
        struct SCREEN *         : "manager", \
        struct ANIM *           : "base", \
        struct ARRAY *          : "base", \
        struct BITFIELD *       : "base", \
        struct CYCLIC *         : "base", \
        struct FSM *            : "base", \
        struct MEMPOOL *        : "base", \
        struct QUEUE *          : "base", \
        struct QUEUE_TRV *      : "base", \
        struct SEQUENCE *       : "base", \
        struct VARIANT *        : "base", \
        struct INPUT_ACTION *   : "base", \
        struct SCREEN_DOTMAP *  : "base" OBJECT_TYPE_SEPARATOR "screen", \
        struct SCREEN_FADE *    : "base" OBJECT_TYPE_SEPARATOR "screen", \
        struct SCREEN_FONT *    : "base" OBJECT_TYPE_SEPARATOR "screen", \
        struct RGB332_Gradient *: "base" OBJECT_TYPE_SEPARATOR "screen", \
        struct SCREEN_SPRITE *  : "base" OBJECT_TYPE_SEPARATOR "screen", \
        struct SCREEN_TILEMAP * : "base" OBJECT_TYPE_SEPARATOR "screen", \
        struct TSKN *           : "os-task", \
        struct NOBJ *           : "nobj", \
        struct OBJECT_INFO *    : OBJECT_INFO_Type( \
                                    (struct OBJECT_INFO *)(uintptr_t)_p) \
    )


#define OBJECT_Description(_p) \
    _Generic((_p), \
        struct OSWRAP *         : OSWRAP_Description(), \
        struct IO *             : IO_Description((struct IO *)_p), \
        struct RAWSTOR *        : RAWSTOR_Description((struct RAWSTOR *)_p), \
        struct STREAM *         : STREAM_Description((struct STREAM *)_p), \
        struct RANDOM *         : RANDOM_Description(), \
        struct SOUND *          : SOUND_Description(), \
        struct TICKS *          : TICKS_Description(), \
        struct VIDEO *          : VIDEO_Description((struct VIDEO *)_p), \
        struct BOARD *          : BOARD_Description(), \
        struct LOG *            : "log", \
        struct OUTPUT *         : "output", \
        struct INPUT *          : "input", \
        struct MIO *            : "io", \
        struct COMM *           : "communication", \
        struct STORAGE *        : "storage", \
        struct SCREEN *         : "screen", \
        struct ANIM *           : "animation", \
        struct ARRAY *          : "array", \
        struct BITFIELD *       : "bitfield", \
        struct CYCLIC *         : "cyclic", \
        struct FSM *            : "fsm", \
        struct MEMPOOL *        : "memory pool", \
        struct QUEUE *          : "queue", \
        struct QUEUE_TRV *      : "queue trv", \
        struct SEQUENCE *       : "sequence", \
        struct VARIANT *        : "variant", \
        struct INPUT_ACTION *   : "input action", \
        struct SCREEN_DOTMAP *  : "dotmap", \
        struct SCREEN_FADE *    : "fade", \
        struct SCREEN_FONT *    : "font", \
        struct RGB332_Gradient *: "rgb332 gradient", \
        struct SCREEN_SPRITE *  : "sprite", \
        struct SCREEN_TILEMAP * : "tilemap", \
        struct TSKN *           : OSWRAP_TaskName(), \
        struct NOBJ *           : "not an object", \
        struct OBJECT_INFO *    : OBJECT_INFO_Description( \
                                    (struct OBJECT_INFO *)(uintptr_t)_p) \
    )


#define OBJECT_Ptr(_p) \
    _Generic((_p), \
        struct OSWRAP *         : _p, \
        struct IO *             : _p, \
        struct RAWSTOR *        : _p, \
        struct STREAM *         : _p, \
        struct RANDOM *         : _p, \
        struct SOUND *          : _p, \
        struct TICKS *          : _p, \
        struct VIDEO *          : _p, \
        struct BOARD *          : _p, \
        struct LOG *            : _p, \
        struct OUTPUT *         : _p, \
        struct INPUT *          : _p, \
        struct MIO *            : _p, \
        struct COMM *           : _p, \
        struct STORAGE *        : _p, \
        struct SCREEN *         : _p, \
        struct ANIM *           : _p, \
        struct ARRAY *          : _p, \
        struct BITFIELD *       : _p, \
        struct CYCLIC *         : _p, \
        struct FSM *            : _p, \
        struct MEMPOOL *        : _p, \
        struct QUEUE *          : _p, \
        struct QUEUE_TRV *      : _p, \
        struct SEQUENCE *       : _p, \
        struct VARIANT *        : _p, \
        struct INPUT_ACTION *   : _p, \
        struct SCREEN_DOTMAP *  : _p, \
        struct SCREEN_FADE *    : _p, \
        struct SCREEN_FONT *    : _p, \
        struct RGB332_Gradient *: _p, \
        struct SCREEN_SPRITE *  : _p, \
        struct SCREEN_TILEMAP * : _p, \
        struct TSKN *           : _p, \
        struct NOBJ *           : NULL, \
        struct OBJECT_INFO *    : OBJECT_INFO_Ptr( \
                                    (struct OBJECT_INFO *)(uintptr_t)_p) \
    )


extern struct NOBJ * NOBJ;
extern struct TSKN * TSKN;


struct OBJECT_INFO
{
    const char *const Type;
    const char *const Description;
    const void *const Ptr;
};


#define OBJECT_INFO_Spawn(_p) \
    (struct OBJECT_INFO) { \
        .Type           = OBJECT_Type(_p), \
        .Description    = OBJECT_Description(_p), \
        .Ptr            = OBJECT_Ptr(_p) \
    }


const char * OBJECT_INFO_Type           (struct OBJECT_INFO *O);
const char * OBJECT_INFO_Description    (struct OBJECT_INFO *O);
const void * OBJECT_INFO_Ptr            (struct OBJECT_INFO *O);
