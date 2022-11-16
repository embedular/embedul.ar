/*
  embedul.ar™ embedded systems framework - http://embedul.ar
  
  newlib freestanding system calls

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

#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>


char *__env[1] = { 0 };
char **environ = __env;


int _close (int file)
{
    (void) file;

	return -1;
}


int _fstat (int file, struct stat *st)
{
    (void) file;

	st->st_mode = S_IFCHR;

	return 0;
}


int _getpid (void)
{
	return 1;
}


int _isatty (int file)
{
    (void) file;

	return 1;
}


int _kill (int pid, int sig)
{
    (void) pid;
    (void) sig;

	errno = EINVAL;

	return -1;
}


int _lseek (int file, int ptr, int dir)
{
    (void) file;
    (void) ptr;
    (void) dir;

	return 0;
}


int _read (int file, char *ptr, int len)
{
    (void) file;
    (void) ptr;
    (void) len;

    return -1;
}


int _write (int file, char *ptr, int len)
{
    (void) file;
    (void) ptr;
    (void) len;

    return -1;
}
