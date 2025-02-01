#ifndef Py_POSIX_AMIGA_H
#define Py_POSIX_AMIGA_H

#ifdef _AMIGA
PyObject *amiga_posix_open(PyObject *self, PyObject *args);
PyObject *amiga_posix_read(PyObject *self, PyObject *args);
PyObject *amiga_posix_write(PyObject *self, PyObject *args);
PyObject *amiga_posix_close(PyObject *self, PyObject *args);
PyObject *amiga_posix_mkdir(PyObject *self, PyObject *args);
PyObject *amiga_posix_rmdir(PyObject *self, PyObject *args);
PyObject *amiga_posix_stat(PyObject *self, PyObject *args);
PyObject *amiga_posix_chmod(PyObject *self, PyObject *args);
PyObject *amiga_posix_getcwd(PyObject *self, PyObject *args);
/* Add other function declarations */
#endif

#endif /* Py_POSIX_AMIGA_H */ 