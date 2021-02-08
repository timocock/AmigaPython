/* Generated automatically from ./Modules/config.c.in by makesetup. */
/* -*- C -*- ***********************************************
Copyright (c) 2000, BeOpen.com.
Copyright (c) 1995-2000, Corporation for National Research Initiatives.
Copyright (c) 1990-1995, Stichting Mathematisch Centrum.
All rights reserved.

See the file "Misc/COPYRIGHT" for information on usage and
redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
******************************************************************/

/* Module configuration */

/* !!! !!! !!! This file is edited by the makesetup script !!! !!! !!! */

/* This file contains the table of built-in modules.
   See init_builtin() in import.c. */

#include "Python.h"
#include "protos.h"

#if defined(AMITCP) || defined(INET225)

/* special 'safe' init functions - they check library availabilities */

static void initpwd_check(void)
{
	if(!checkusergrouplib()) return;
	initpwd();
}

static void initgrp_check(void)
{
	if(!checkusergrouplib()) return;
	initgrp();
}

static void initcrypt_check(void)
{
	if(!checkusergrouplib()) return;
	initcrypt();
}

static void initsyslog_check(void)
{
	if(!checksocketlib()) return;
	initsyslog();
}

static void initsocket_check(void)
{
	if(!checksocketlib()) return;
	init_socket();
}

static void initselect_check(void)
{
	if(!checksocketlib()) return;
	initselect();
}
#endif /* AmiTCP or Inet225 */

#ifndef _AMIGA
extern void initsignal(void);
extern void initposix(void);
#endif

extern void initamiga(void);
extern void initARexx(void);
extern void initDoslib(void);
extern void initenvironment(void);
extern void init_locale(void);
extern void init_csv(void);
extern void initxxsubtype(void);
// extern void initunicodedata(void);
extern void init_testcapi(void);
extern void init_symtable(void);
extern void initerrno(void);
extern void init_sre(void);
extern void init_codecs(void);
extern void initzipimport(void);
extern void initarray(void);
extern void initcmath(void);
extern void initmath(void);
extern void initstruct(void);
extern void inittime(void);
extern void initoperator(void);
extern void init_weakref(void);
extern void initpcre(void);
extern void init_random(void);
extern void initstrop(void);
extern void initmd5(void);
extern void initrotor(void);
extern void initregex(void);
extern void initcPickle(void);
extern void initcStringIO(void);
extern void initsha(void);
extern void initzlib(void);
extern void initbinascii(void);
extern void initdatetime(void);
extern void initpyexpat(void);
extern void initbz2(void);

extern void initpwd();
extern void initgrp();
extern void initcrypt();
extern void initselect();
extern void init_socket();
extern void initsyslog();


/* -- ADDMODULE MARKER 1 -- */

extern void PyMarshal_Init(void);
extern void initimp(void);
extern void initgc(void);

struct _inittab _PyImport_Inittab[] = {

//	{"signal", initsignal},
//	{"posix", initposix},
	{"amiga", initamiga},
	{"ARexxll", initARexx},
	{"Doslib", initDoslib},
	{"errno", initerrno},
	{"environment", initenvironment},
	{"_sre", init_sre},
	{"_codecs", init_codecs},
	{"zipimport", initzipimport},
	{"_symtable", init_symtable},
	{"array", initarray},
	{"cmath", initcmath},
	{"math", initmath},
	{"struct", initstruct},
	{"time", inittime},
	{"operator", initoperator},
	{"_weakref", init_weakref},
	{"_testcapi", init_testcapi},
//	{"unicodedata", initunicodedata},
	{"_locale", init_locale},
//	{"pwd", initpwd},
//	{"grp", initgrp},
//	{"select", initselect},
	{"_csv", init_csv},
//	{"_socket", init_socket},
	{"xxsubtype", initxxsubtype},
	{"pcre", initpcre},
	{"_random", init_random},
	{"strop", initstrop},
	{"md5",initmd5},
	{"rotor",initrotor},
	{"regex", initregex},
	{"cPickle", initcPickle},
	{"cStringIO", initcStringIO},
	{"sha",initsha},
	{"zlib",initzlib},
        {"binascii", initbinascii},
	{"datetime", initdatetime},
	{"pyexpat", initpyexpat},
	{"bz2", initbz2},

#if defined(AMITCP) || defined(INET225)
	/* Use the lib-checking init functions defined above */
	{"pwd",initpwd_check},
	{"grp",initgrp_check},
	{"crypt",initcrypt_check},
	{"select",initselect_check},
	{"_socket",initsocket_check},
	{"syslog",initsyslog_check},
#endif

/* -- ADDMODULE MARKER 2 -- */

	/* This module lives in marshal.c */
	{"marshal", PyMarshal_Init},

	/* This lives in import.c */
	{"imp", initimp},

	/* These entries are here for sys.builtin_module_names */
	{"__main__", NULL},
	{"__builtin__", NULL},
	{"sys", NULL},
	{"exceptions", NULL},

	/* This lives in gcmodule.c */
	{"gc", initgc},

	/* Sentinel */
	{0, 0}
};
