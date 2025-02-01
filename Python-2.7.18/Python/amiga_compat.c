#include "Python.h"
#include "amiga_compat.h"

#ifdef _AMIGA
/* Include existing Amiga implementations */
#include "Amiga/_allocufb.c"
#include "Amiga/_chkufb.c"
#include "Amiga/_close.c"
#include "Amiga/_dup.c"
#include "Amiga/_dup2.c"
#include "Amiga/_fstat.c"
#include "Amiga/_lseek.c"
#include "Amiga/_open.c"
#include "Amiga/_read.c"
#include "Amiga/_write.c"
#include "Amiga/access.c"
#include "Amiga/chmod.c"
#include "Amiga/chown.c"
#include "Amiga/dosio_init.c"
#include "Amiga/dostat.c"
#include "Amiga/fhopen.c"

/* Initialize Amiga compatibility layer */
void init_amiga_compat(void)
{
    /* Initialize DOS I/O */
    __init_dosio();
}
#endif