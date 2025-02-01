#include "Python.h"
#include "amiga_compat.h"

#ifdef _AMIGA

/* Amiga implementations of POSIX functions */

static PyMethodDef amiga_posix_methods[] = {
    {"open", amiga_posix_open, METH_VARARGS, "Open a file."},
    {"read", amiga_posix_read, METH_VARARGS, "Read from a file."},
    {"write", amiga_posix_write, METH_VARARGS, "Write to a file."},
    {"close", amiga_posix_close, METH_VARARGS, "Close a file."},
    {"mkdir", amiga_posix_mkdir, METH_VARARGS, "Create a directory."},
    {"rmdir", amiga_posix_rmdir, METH_VARARGS, "Remove a directory."},
    {"stat", amiga_posix_stat, METH_VARARGS, "Get file status."},
    {"chmod", amiga_posix_chmod, METH_VARARGS, "Change file mode."},
    {"getcwd", amiga_posix_getcwd, METH_VARARGS, "Get current working directory."},
    {NULL, NULL, 0, NULL}
};

PyObject *
amiga_posix_open(PyObject *self, PyObject *args)
{
    char *file;
    int flag;
    int mode = 0777;
    BPTR fh;
    int fd;
    char *apath;

    if (!PyArg_ParseTuple(args, "si|i:open", &file, &flag, &mode))
        return NULL;

    apath = amiga_to_unix_path(file);
    if (apath == NULL)
        return PyErr_NoMemory();

    Py_BEGIN_ALLOW_THREADS
    fh = Open((STRPTR)apath, MODE_OLDFILE);
    Py_END_ALLOW_THREADS

    free(apath);
    if (fh == 0) {
        set_amiga_error();
        return NULL;
    }

    fd = AmigaFH_FromFH(fh);
    if (fd < 0) {
        Close(fh);
        return NULL;
    }

    return PyInt_FromLong((long)fd);
}

PyObject *
amiga_posix_read(PyObject *self, PyObject *args)
{
    int fd;
    Py_ssize_t size;
    BPTR fh;
    PyObject *buffer;
    LONG bytes_read;

    if (!PyArg_ParseTuple(args, "in:read", &fd, &size))
        return NULL;

    fh = AmigaFH_ToFH(fd);
    if (!fh) {
        set_amiga_error();
        return NULL;
    }

    buffer = PyString_FromStringAndSize((char *)NULL, size);
    if (buffer == NULL)
        return NULL;

    Py_BEGIN_ALLOW_THREADS
    bytes_read = Read(fh, PyString_AS_STRING(buffer), size);
    Py_END_ALLOW_THREADS

    if (bytes_read < 0) {
        Py_DECREF(buffer);
        set_amiga_error();
        return NULL;
    }

    if (bytes_read != size)
        _PyString_Resize(&buffer, bytes_read);

    return buffer;
}

PyObject *
amiga_posix_write(PyObject *self, PyObject *args)
{
    int fd;
    Py_buffer pbuf;
    BPTR fh;
    LONG bytes_written;

    if (!PyArg_ParseTuple(args, "is*:write", &fd, &pbuf))
        return NULL;

    fh = AmigaFH_ToFH(fd);
    if (!fh) {
        PyBuffer_Release(&pbuf);
        set_amiga_error();
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    bytes_written = Write(fh, pbuf.buf, pbuf.len);
    Py_END_ALLOW_THREADS

    PyBuffer_Release(&pbuf);

    if (bytes_written < 0) {
        set_amiga_error();
        return NULL;
    }

    return PyInt_FromLong(bytes_written);
}

PyObject *
amiga_posix_close(PyObject *self, PyObject *args)
{
    int fd;
    BPTR fh;

    if (!PyArg_ParseTuple(args, "i:close", &fd))
        return NULL;

    fh = AmigaFH_ToFH(fd);
    if (!fh) {
        set_amiga_error();
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    Close(fh);
    Py_END_ALLOW_THREADS

    AmigaFH_RemoveFH(fd);
    Py_RETURN_NONE;
}

PyObject *
amiga_posix_mkdir(PyObject *self, PyObject *args)
{
    char *path;
    int mode = 0777;
    char *apath;
    BPTR lock;

    if (!PyArg_ParseTuple(args, "s|i:mkdir", &path, &mode))
        return NULL;

    apath = amiga_to_unix_path(path);
    if (apath == NULL)
        return PyErr_NoMemory();

    Py_BEGIN_ALLOW_THREADS
    lock = CreateDir((STRPTR)apath);
    Py_END_ALLOW_THREADS

    free(apath);
    
    if (!lock) {
        set_amiga_error();
        return NULL;
    }

    UnLock(lock);
    Py_RETURN_NONE;
}

PyObject *
amiga_posix_rmdir(PyObject *self, PyObject *args)
{
    char *path;
    char *apath;
    BOOL success;

    if (!PyArg_ParseTuple(args, "s:rmdir", &path))
        return NULL;

    apath = amiga_to_unix_path(path);
    if (apath == NULL)
        return PyErr_NoMemory();

    Py_BEGIN_ALLOW_THREADS
    success = DeleteFile((STRPTR)apath);
    Py_END_ALLOW_THREADS

    free(apath);

    if (!success) {
        set_amiga_error();
        return NULL;
    }

    Py_RETURN_NONE;
}

PyObject *
amiga_posix_stat(PyObject *self, PyObject *args)
{
    char *path;
    char *apath;
    struct stat st;
    BPTR lock;
    struct FileInfoBlock *fib;
    PyObject *result;

    if (!PyArg_ParseTuple(args, "s:stat", &path))
        return NULL;

    apath = amiga_to_unix_path(path);
    if (apath == NULL)
        return PyErr_NoMemory();

    Py_BEGIN_ALLOW_THREADS
    lock = Lock((STRPTR)apath, ACCESS_READ);
    Py_END_ALLOW_THREADS

    free(apath);

    if (!lock) {
        set_amiga_error();
        return NULL;
    }

    fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, NULL);
    if (!fib) {
        UnLock(lock);
        return PyErr_NoMemory();
    }

    if (!Examine(lock, fib)) {
        UnLock(lock);
        FreeDosObject(DOS_FIB, fib);
        set_amiga_error();
        return NULL;
    }

    /* Convert Amiga file info to Unix stat structure */
    memset(&st, 0, sizeof(st));
    st.st_mode = (fib->fib_DirEntryType > 0) ? S_IFDIR | 0755 : S_IFREG | 0644;
    st.st_size = fib->fib_Size;
    st.st_mtime = fib->fib_Date.ds_Days * 86400 + 
                  fib->fib_Date.ds_Minute * 60 +
                  fib->fib_Date.ds_Tick / TICKS_PER_SECOND;
    st.st_atime = st.st_mtime;
    st.st_ctime = st.st_mtime;
    st.st_nlink = 1;
    st.st_uid = 0;
    st.st_gid = 0;

    UnLock(lock);
    FreeDosObject(DOS_FIB, fib);

    result = Py_BuildValue("(llllllllll)",
        st.st_mode,
        st.st_ino,
        st.st_dev,
        st.st_nlink,
        st.st_uid,
        st.st_gid,
        st.st_size,
        st.st_atime,
        st.st_mtime,
        st.st_ctime);

    return result;
}

PyObject *
amiga_posix_chmod(PyObject *self, PyObject *args)
{
    char *path;
    int mode;
    char *apath;
    BPTR lock;
    BOOL success = TRUE;

    if (!PyArg_ParseTuple(args, "si:chmod", &path, &mode))
        return NULL;

    apath = amiga_to_unix_path(path);
    if (apath == NULL)
        return PyErr_NoMemory();

    Py_BEGIN_ALLOW_THREADS
    lock = Lock((STRPTR)apath, ACCESS_READ);
    Py_END_ALLOW_THREADS

    free(apath);

    if (!lock) {
        set_amiga_error();
        return NULL;
    }

    /* Amiga doesn't really support Unix-style permissions,
       but we can set protection bits */
    if (success) {
        ULONG protection = FIBF_READ|FIBF_WRITE|FIBF_EXECUTE;
        if (!(mode & 0222))  /* Write protection */
            protection &= ~FIBF_WRITE;
        if (!(mode & 0444))  /* Read protection */
            protection &= ~FIBF_READ;
        if (!(mode & 0111))  /* Execute protection */
            protection &= ~FIBF_EXECUTE;

        success = SetProtection((STRPTR)path, protection);
    }

    UnLock(lock);

    if (!success) {
        set_amiga_error();
        return NULL;
    }

    Py_RETURN_NONE;
}

PyObject *
amiga_posix_getcwd(PyObject *self, PyObject *args)
{
    char pathname[1024];
    BPTR lock;
    BOOL success;

    lock = Lock("", ACCESS_READ);
    if (!lock) {
        set_amiga_error();
        return NULL;
    }

    success = NameFromLock(lock, (STRPTR)pathname, sizeof(pathname));
    UnLock(lock);

    if (!success) {
        set_amiga_error();
        return NULL;
    }

    return PyString_FromString(pathname);
}

/* Add other POSIX function implementations here */

static int amiga_posix_initialized = 0;

void init_amiga_posix(void)
{
    if (amiga_posix_initialized) return;
    
    /* Override POSIX functions */
    PyOS_ReadlineFunctionPointer = amiga_posix_read;
    PyOS_InputHook = amiga_posix_inputhook;
    
    /* Replace module methods */
    PyModule_AddObject(PyImport_AddModule("posix"),
                      "open", PyCFunction_New(&amiga_posix_open_def, NULL));
    
    amiga_posix_initialized = 1;
}

#endif /* _AMIGA */ 