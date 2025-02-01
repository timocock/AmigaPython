RCS_ID_C="$Id: _lseek.c,v 4.1 1994/09/29 23:09:02 jraja Exp $";
/*
 *      _lseek.c - lseek() which knows sockets (SAS/C)
 *
 *      Copyright � 1994 AmiTCP/IP Group, 
 *                       Network Solutions Development Inc.
 *                       All rights reserved.
 */

#include <ios1.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dos.h>
#include <errno.h>
#include <dos/dos.h>
#include <proto/dos.h>

#ifdef HAVE_LARGEFILE_SUPPORT
off_t __lseek(int fd, off_t rpos, int mode)
#else
long __lseek(int fd, long rpos, int mode)
#endif
{
  struct UFB *ufb;
  long        apos;

  /*
   * Check for the break signals
   */
  __chkabort();
  /*
   * find the ufb *
   */
  if ((ufb = __chkufb(fd)) == NULL) {
    errno = EINVAL;
    return -1;
  }
  
  _OSERR = 0;

  if (ufb->ufbflg & UFB_SOCK) {
    errno = ESPIPE; /* illegal seek */
    return -1;
  }

  if ((apos = Seek(ufb->ufbfh, (LONG)rpos, mode - 1)) == -1) {
    _OSERR = IoErr();
    errno = EIO;
    return -1;
  }
  
  switch (mode) {
  case 0:
    return rpos;
  case 1:
    return apos + rpos;
  case 2:
    return Seek(ufb->ufbfh, 0, 0);
  }
}
