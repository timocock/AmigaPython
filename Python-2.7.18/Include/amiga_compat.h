#ifndef Py_AMIGACOMPAT_H
#define Py_AMIGACOMPAT_H

#ifdef _AMIGA
#include <proto/dos.h>
#include <proto/exec.h>
#include <dos/dos.h>
#include <exec/memory.h>
#include <exec/types.h>

/* Amiga error translation */
static void set_amiga_error(void);
static char *amiga_to_unix_path(const char *path);
static char *unix_to_amiga_path(const char *path);

#endif /* _AMIGA */
#endif /* Py_AMIGACOMPAT_H */ 