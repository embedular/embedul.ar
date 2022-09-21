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


#define OBJECT_TYPE_SEPARATOR   ":"


extern struct NOBJ * NOBJ;


#define OBJECT_StaticCheck(_p) \
    _Generic( \
        (_p), \
        struct IO *             : (void)0, \
        struct RAWSTOR *        : (void)0, \
        struct STREAM *         : (void)0, \
        struct PACKET *         : (void)0, \
        struct RANDOM *         : (void)0, \
        struct SOUND *          : (void)0, \
        struct VIDEO *          : (void)0, \
        struct BOARD *          : (void)0, \
        struct LOG *            : (void)0, \
        struct OUTPUT *         : (void)0, \
        struct INPUT *          : (void)0, \
        struct COMM *           : (void)0, \
        struct STORAGE *        : (void)0, \
        struct ANIM *           : (void)0, \
        struct ARRAY *          : (void)0, \
        struct BITFIELD *       : (void)0, \
        struct CYCLIC *         : (void)0, \
        struct FSM *            : (void)0, \
        struct MEMPOOL *        : (void)0, \
        struct QUEUE *          : (void)0, \
        struct QUEUE_TRV *      : (void)0, \
        struct SEQUENCE *       : (void)0, \
        struct VARIANT *        : (void)0, \
        struct SWITCH_ACTION *  : (void)0, \
        struct VIDEO_DOTMAP *   : (void)0, \
        struct VIDEO_FADE *     : (void)0, \
        struct VIDEO_FONT *     : (void)0, \
        struct VIDEO_RGB332_Gradient * \
                                : (void)0, \
        struct VIDEO_SPRITE *   : (void)0, \
        struct VIDEO_TILEMAP *  : (void)0 \
    )


#define OBJECT_Clear(_p) \
    OBJECT_StaticCheck (_p); \
    if (_p) { memset (_p, 0, sizeof(*_p)); }


#define OBJECT_Type(_p) \
    _Generic( \
        (_p), \
        struct IO *             : "dev" OBJECT_TYPE_SEPARATOR "io", \
        struct RAWSTOR *        : "dev" OBJECT_TYPE_SEPARATOR "rawstor", \
        struct STREAM *         : "dev" OBJECT_TYPE_SEPARATOR "stream", \
        struct PACKET *         : "dev" OBJECT_TYPE_SEPARATOR "packet", \
        struct RANDOM *         : "dev" OBJECT_TYPE_SEPARATOR "random", \
        struct SOUND *          : "dev" OBJECT_TYPE_SEPARATOR "sound", \
        struct VIDEO *          : "dev" OBJECT_TYPE_SEPARATOR "video", \
        struct BOARD *          : "dev" OBJECT_TYPE_SEPARATOR "board", \
        struct LOG *            : "manager", \
        struct OUTPUT *         : "manager", \
        struct INPUT *          : "manager", \
        struct COMM *           : "manager", \
        struct STORAGE *        : "manager", \
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
        struct SWITCH_ACTION *  : "base", \
        struct VIDEO_DOTMAP *   : "base" OBJECT_TYPE_SEPARATOR "video", \
        struct VIDEO_FADE *     : "base" OBJECT_TYPE_SEPARATOR "video", \
        struct VIDEO_FONT *     : "base" OBJECT_TYPE_SEPARATOR "video", \
        struct VIDEO_RGB332_Gradient * \
                                : "base" OBJECT_TYPE_SEPARATOR "video", \
        struct VIDEO_SPRITE *   : "base" OBJECT_TYPE_SEPARATOR "video", \
        struct VIDEO_TILEMAP *  : "base" OBJECT_TYPE_SEPARATOR "video", \
        struct NOBJ *           : "nobj", \
        struct OBJECT_INFO *    : OBJECT_INFO_Type( \
                                    (struct OBJECT_INFO *)(uintptr_t)_p) \
    )


#define OBJECT_Description(_p) \
    _Generic( \
        (_p), \
        struct IO *             : IO_Description((struct IO *)_p), \
        struct RAWSTOR *        : RAWSTOR_Description((struct RAWSTOR *)_p), \
        struct STREAM *         : STREAM_Description((struct STREAM *)_p), \
        struct PACKET *         : PACKET_Description((struct PACKET *)_p), \
        struct RANDOM *         : RANDOM_Description(), \
        struct SOUND *          : SOUND_Description(), \
        struct VIDEO *          : VIDEO_Description(), \
        struct BOARD *          : BOARD_Description(), \
        struct LOG *            : "log", \
        struct OUTPUT *         : "output", \
        struct INPUT *          : "input", \
        struct COMM *           : "communication", \
        struct STORAGE *        : "storage", \
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
        struct SWITCH_ACTION *  : "switch action", \
        struct VIDEO_DOTMAP *   : "dotmap", \
        struct VIDEO_FADE *     : "fade", \
        struct VIDEO_FONT *     : "font", \
        struct VIDEO_RGB332_Gradient * \
                                : "rgb332 gradient", \
        struct VIDEO_SPRITE *   : "sprite", \
        struct VIDEO_TILEMAP *  : "tilemap", \
        struct NOBJ *           : "not an object", \
        struct OBJECT_INFO *    : OBJECT_INFO_Description( \
                                    (struct OBJECT_INFO *)(uintptr_t)_p) \
    )


#define OBJECT_Ptr(_p) \
    _Generic( \
        (_p), \
        struct IO *             : _p, \
        struct RAWSTOR *        : _p, \
        struct STREAM *         : _p, \
        struct PACKET *         : _p, \
        struct RANDOM *         : _p, \
        struct SOUND *          : _p, \
        struct VIDEO *          : _p, \
        struct BOARD *          : _p, \
        struct LOG *            : _p, \
        struct OUTPUT *         : _p, \
        struct INPUT *          : _p, \
        struct COMM *           : _p, \
        struct STORAGE *        : _p, \
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
        struct SWITCH_ACTION *  : _p, \
        struct VIDEO_DOTMAP *   : _p, \
        struct VIDEO_FADE *     : _p, \
        struct VIDEO_FONT *     : _p, \
        struct VIDEO_RGB332_Gradient * \
                                : _p, \
        struct VIDEO_SPRITE *   : _p, \
        struct VIDEO_TILEMAP *  : _p, \
        struct NOBJ *           : NULL, \
        struct OBJECT_INFO *    : OBJECT_INFO_Ptr( \
                                    (struct OBJECT_INFO *)(uintptr_t)_p) \
    )
