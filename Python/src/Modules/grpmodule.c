
/* UNIX group file access module */

#include "Python.h"
#include "structseq.h"

#include <sys/types.h>
#include <grp.h>

#ifdef AMITCP
#include <proto/usergroup.h>
#endif
#ifdef INET225
#include <proto/socket.h>
#endif


static PyStructSequence_Field struct_group_type_fields[] = {
   {"gr_name", "group name"},
   {"gr_passwd", "password"},
   {"gr_gid", "group id"},
   {"gr_mem", "group memebers"},
   {0}
};

PyDoc_STRVAR(struct_group__doc__,
"grp.struct_group: Results from getgr*() routines.\n\n\
This object may be accessed either as a tuple of\n\
  (gr_name,gr_passwd,gr_gid,gr_mem)\n\
or via the object attributes as named in the above tuple.\n");

static PyStructSequence_Desc struct_group_type_desc = {
   "grp.struct_group",
   struct_group__doc__,
   struct_group_type_fields,
   4,
};


static PyTypeObject StructGrpType;

static PyObject *
mkgrent(struct group *p)
{
    int setIndex = 0;
    PyObject *v = PyStructSequence_New(&StructGrpType), *w;
    char **member;

    if (v == NULL)
        return NULL;

    if ((w = PyList_New(0)) == NULL) {
        Py_DECREF(v);
        return NULL;
    }
    for (member = p->gr_mem; *member != NULL; member++) {
        PyObject *x = PyString_FromString(*member);
        if (x == NULL || PyList_Append(w, x) != 0) {
            Py_XDECREF(x);
            Py_DECREF(w);
            Py_DECREF(v);
            return NULL;
        }
        Py_DECREF(x);
    }

#define SET(i,val) PyStructSequence_SET_ITEM(v, i, val)
    SET(setIndex++, PyString_FromString(p->gr_name));
#ifdef __VMS
    SET(setIndex++, Py_None);
    Py_INCREF(Py_None);
#else
    if (p->gr_passwd)
	    SET(setIndex++, PyString_FromString(p->gr_passwd));
    else {
	    SET(setIndex++, Py_None);
	    Py_INCREF(Py_None);
    }
#endif
    SET(setIndex++, PyInt_FromLong((long) p->gr_gid));
    SET(setIndex++, w);
#undef SET

    if (PyErr_Occurred()) {
        Py_DECREF(v);
        Py_DECREF(w);
        return NULL;
    }

    return v;
}

static PyObject *
grp_getgrgid(PyObject *self, PyObject *args)
{
    int gid;
    struct group *p;
    if (!PyArg_ParseTuple(args, "i:getgrgid", &gid))
        return NULL;
    if ((p = getgrgid(gid)) == NULL) {
        PyErr_SetString(PyExc_KeyError, "getgrgid(): gid not found");
        return NULL;
    }
    return mkgrent(p);
}

static PyObject *
grp_getgrnam(PyObject *self, PyObject *args)
{
    char *name;
    struct group *p;
    if (!PyArg_ParseTuple(args, "s:getgrnam", &name))
        return NULL;
    if ((p = getgrnam(name)) == NULL) {
        PyErr_SetString(PyExc_KeyError, "getgrnam(): name not found");
        return NULL;
    }
    return mkgrent(p);
}

static PyObject *
grp_getgrall(PyObject *self, PyObject *args)
{
    PyObject *d;
    struct group *p;

    if (!PyArg_ParseTuple(args, ":getgrall"))
        return NULL;
    if ((d = PyList_New(0)) == NULL)
        return NULL;
#if !defined(AMITCP) && !defined(INET225)
    setgrent();
    while ((p = getgrent()) != NULL) {
        PyObject *v = mkgrent(p);
        if (v == NULL || PyList_Append(d, v) != 0) {
            Py_XDECREF(v);
            Py_DECREF(d);
            return NULL;
        }
        Py_DECREF(v);
    }
    return d;
#else
 #ifdef AMITCP
	setgrent();
 #else
    setgrent(1); /* INET225 wants argument XXX correct? */
 #endif
	while ((p = getgrent()) != NULL) {
		PyObject *v = mkgrent(p);
		if (v == NULL || PyList_Append(d, v) != 0) {
			Py_XDECREF(v);
			Py_DECREF(d);
			endgrent();
			return NULL;
		}
		Py_DECREF(v);
	}
	endgrent();
	return d;
#endif /* AMITCP or INET225 */

}

static PyMethodDef grp_methods[] = {
    {"getgrgid",	grp_getgrgid,	METH_VARARGS,
     "getgrgid(id) -> tuple\n\
Return the group database entry for the given numeric group ID.  If\n\
id is not valid, raise KeyError."},
    {"getgrnam",	grp_getgrnam,	METH_VARARGS,
     "getgrnam(name) -> tuple\n\
Return the group database entry for the given group name.  If\n\
name is not valid, raise KeyError."},
    {"getgrall",	grp_getgrall,	METH_VARARGS,
     "getgrall() -> list of tuples\n\
Return a list of all available group entries, in arbitrary order."},
    {NULL,		NULL}		/* sentinel */
};

PyDoc_STRVAR(grp__doc__,
"Access to the Unix group database.\n\
\n\
Group entries are reported as 4-tuples containing the following fields\n\
from the group database, in order:\n\
\n\
  name   - name of the group\n\
  passwd - group password (encrypted); often empty\n\
  gid    - numeric ID of the group\n\
  mem    - list of members\n\
\n\
The gid is an integer, name and password are strings.  (Note that most\n\
users are not explicitly listed as members of the groups they are in\n\
according to the password database.  Check both databases to get\n\
complete membership information.)");


PyMODINIT_FUNC
initgrp(void)
{
    PyObject *m, *d;
    m = Py_InitModule3("grp", grp_methods, grp__doc__);
    d = PyModule_GetDict(m);
    PyStructSequence_InitType(&StructGrpType, &struct_group_type_desc);
    PyDict_SetItemString(d, "struct_group", (PyObject *) &StructGrpType);
}
