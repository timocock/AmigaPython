RCS_ID_C="$Id: strerror.c,v 4.1 1994/09/29 23:09:02 jraja Exp $";
/*
 *      strerror.c - network errno support for AmiTCP/IP
 *
 *      Copyright � 1994 AmiTCP/IP Group, 
 *                       Network Solutions Development Inc.
 *                       All rights reserved.
 */

#include <errno.h>
#include <bsdsocket.h>
#include <amitcp/socketbasetags.h>

/****** net.lib/strerror *****************************************************

	NAME
		strerror -- return the text for given error number

	SYNOPSIS
		string = strerror(error);

		char * strerror(int);

	FUNCTION
		This function returns pointer to the (English) string describing the
		error code given as argument. The error strings are defined for the
		error codes defined in <sys/errno.h>.

	NOTES
		The string pointed to by the return value should not be modified by
		the program, but may be overwritten by a subsequent call to this
		function.

	BUGS
		The strerror() prototype should be 
	const char *strerror(unsigned int); 
	However, the SAS C includes define it differently.

	SEE ALSO
		<netinclude:sys/errno.h>, perror(), PrintNetFault()
*****************************************************************************
*/

#include "libcheck.h"

extern char *__sys_errlist[];

#ifdef notyet
const char *
strerror(unsigned int error)
#else
char *
strerror(int error)
#endif
{
  ULONG taglist[3];

  if(!checksocketlib())
  {
	/* cannot use bsdsocket.lib's error strings, use those from SAS */
	PyErr_Clear();
	if(error>=0 && error<=34) return __sys_errlist[error];
	if(error==ELOOP) return "Too many levels of links"; /* link loop */
	else return __sys_errlist[0];
  }

  taglist[0] = SBTM_GETVAL(SBTC_ERRNOSTRPTR);
  taglist[1] = error;
  taglist[2] = TAG_END;

  SocketBaseTagList((struct TagItem *)taglist);
  return (char *)taglist[1];
}
