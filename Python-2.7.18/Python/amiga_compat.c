#include "Python.h"
#include "amiga_compat.h"

#ifdef _AMIGA

void set_amiga_error(void)
{
    int error = IoErr();
    switch(error) {
        case ERROR_OBJECT_NOT_FOUND:
            errno = ENOENT;
            break;
        case ERROR_DISK_WRITE_PROTECTED:
            errno = EROFS;
            break;
        case ERROR_OBJECT_EXISTS:
            errno = EEXIST;
            break;
        case ERROR_DIR_NOT_FOUND:
            errno = ENOTDIR;
            break;
        default:
            errno = EIO;
    }
}

char *amiga_to_unix_path(const char *path)
{
    char *newpath = strdup(path);
    char *p;
    if (newpath == NULL)
        return NULL;
    for (p = newpath; *p; p++) {
        if (*p == '/') 
            *p = ':';
    }
    return newpath;
}

char *unix_to_amiga_path(const char *path)
{
    char *newpath = strdup(path);
    char *p;
    if (newpath == NULL)
        return NULL;
    for (p = newpath; *p; p++) {
        if (*p == ':') 
            *p = '/';
    }
    return newpath;
}

#endif /* _AMIGA */ 