#include "Python.h"

#ifdef macintosh
#include "macbuildno.h"
#endif

#ifndef DONT_HAVE_STDIO_H
#include <stdio.h>
#endif

#ifndef DATE
#ifdef __DATE__
#define DATE __DATE__
#else
#define DATE "xx/xx/xx"
#endif
#endif

#ifndef TIME
#ifdef __TIME__
#define TIME __TIME__
#else
#define TIME "xx:xx:xx"
#endif
#endif

#ifndef BUILD
#define BUILD 0
#endif

const char *
Py_GetBuildInfo(void)
{
	static char buildinfo[50];
	PyOS_snprintf(buildinfo, sizeof(buildinfo),
		      "#%d, %.20s, %.9s", BUILD, DATE, TIME);
	return buildinfo;
}


#ifdef _AMIGA
#include "patchlevel.h"
/* AmigaDOS version string */
const char ver[] = "$VER: Python 2.3.3 " __AMIGADATE__ " AmiTCP";
/*
#ifdef AMITCP
 " AmiTCP";
#else
#ifdef INET225
 " I-Net225";
#else
 "";

#endif /* INET225
#endif /* AMITCP
*/
const char shellstack[] = "$STACK:100000\n";
#endif /* _AMIGA */
