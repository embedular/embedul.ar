/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  [STREAM driver] hosted environment file.

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

#include "embedul.ar/source/arch/native/sdl/drivers/stream_file.h"
#include "embedul.ar/source/core/device/board.h"
#include <errno.h>
#include <stdlib.h>


#define FILENAME_OPEN_ERROR_STR         "error opening file"
#define FILENAME_ITEM_STR               "filename"
#define ERRNO_ITEM_STR                  "errno"


extern FILE * stdout;
extern FILE * stderr;


// Common IO interface
static void         hardwareInit    (struct STREAM *const S);
static uint32_t     dataIn          (struct STREAM *const S,
                                     const uint8_t* Data,
                                     const uint32_t Octets);
static uint32_t     dataOut         (struct STREAM *const S,
                                     uint8_t *const Buffer,
                                     const uint32_t Octets);


static const struct STREAM_IFACE STREAM_FILE_IFACE =
{
    .Description    = "os-hosted file",
    .HardwareInit   = hardwareInit,
    .DataIn         = dataIn,
    .DataOut        = dataOut
};


void STREAM_FILE_Init (struct STREAM_FILE *c, const char *filename)
{
    BOARD_AssertParams (c && filename);

    DEVICE_IMPLEMENTATION_Clear (c);

    c->filename = filename;

    STREAM_Init ((struct STREAM *)c, &STREAM_FILE_IFACE);
}


static void hardwareInit (struct STREAM *const S)
{
    struct STREAM_FILE *const C = (struct STREAM_FILE *) S;

    if (!strcmp (C->filename, "stdout"))
    {
        C->fd = stdout;
    }
    else if (!strcmp (C->filename, "stderr"))
    {
        C->fd = stderr;
    }
    else
    {
        C->fd = fopen (C->filename, "w+b");
    }

    if (!C->fd)
    {
        LOG_Warn (S, FILENAME_OPEN_ERROR_STR);
        LOG_Items (2,
                FILENAME_ITEM_STR,      C->filename,
                ERRNO_ITEM_STR,         errno);

        BOARD_AssertInitialized (false);
    }
}


static uint32_t dataIn (struct STREAM *const S, const uint8_t *const Data,
                        const uint32_t Octets)
{
    struct STREAM_FILE *const F = (struct STREAM_FILE *) S;

    const uint32_t WrittenOctets = (uint32_t) fwrite (Data, 1, Octets, F->fd);

    fflush (F->fd);

    return WrittenOctets;
}


static uint32_t dataOut (struct STREAM *const S, uint8_t *const Buffer,
                         const uint32_t Octets)
{
    struct STREAM_FILE *const F = (struct STREAM_FILE *) S;

    const uint32_t ReadOctets = (uint32_t) fread (Buffer, 1, Octets, F->fd);

    return ReadOctets;
}
