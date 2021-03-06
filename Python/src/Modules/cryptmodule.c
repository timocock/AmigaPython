/* cryptmodule.c - by Steve Majewski
 */

#include "Python.h"

#include <sys/types.h>

#ifdef AMITCP
#include <proto/usergroup.h>
#endif
#ifdef INET225
#include <proto/socket.h>
static __inline STRPTR crypt(STRPTR pw, STRPTR un)
{
	static char buf[32];
	return s_crypt(buf,pw,un);
}
#endif


/* Module crypt */


static PyObject *crypt_crypt(PyObject *self, PyObject *args)
{
	char *word, *salt;
	extern char * crypt(const char *, const char *);

	if (!PyArg_ParseTuple(args, "ss:crypt", &word, &salt)) {
		return NULL;
	}
	/* On some platforms (AtheOS) crypt returns NULL for an invalid
	   salt. Return None in that case. XXX Maybe raise an exception?  */
	return Py_BuildValue("s", crypt(word, salt));

}

PyDoc_STRVAR(crypt_crypt__doc__,
"crypt(word, salt) -> string\n\
word will usually be a user's password. salt is a 2-character string\n\
which will be used to select one of 4096 variations of DES. The characters\n\
in salt must be either \".\", \"/\", or an alphanumeric character. Returns\n\
the hashed password as a string, which will be composed of characters from\n\
the same alphabet as the salt.");


static PyMethodDef crypt_methods[] = {
	{"crypt",	crypt_crypt, METH_VARARGS, crypt_crypt__doc__},
	{NULL,		NULL}		/* sentinel */
};

PyMODINIT_FUNC
initcrypt(void)
{
	Py_InitModule("crypt", crypt_methods);
}
