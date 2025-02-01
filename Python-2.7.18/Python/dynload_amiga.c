/* Add Amiga-specific dynamic loader stub */
#ifdef __amigaos__
#include "Python.h"

dl_funcptr _PyImport_GetDynLoadFunc(const char *shortname,
                                    const char *pathname, FILE *fp)
{
    PyErr_SetString(PyExc_NotImplementedError,
                    "Dynamic loading not supported on AmigaOS");
    return NULL;
}
#endif 