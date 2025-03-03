/* Python interpreter main program */

#include "Python.h"
#include "osdefs.h"
#include "code.h" /* For CO_FUTURE_DIVISION */
#include "import.h"

#ifdef __VMS
#include <unixlib.h>
#endif

#if defined(MS_WINDOWS) || defined(__CYGWIN__)
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#endif

#if (defined(PYOS_OS2) && !defined(PYCC_GCC)) || defined(MS_WINDOWS)
#define PYTHONHOMEHELP "<prefix>\\lib"
#else
#if defined(PYOS_OS2) && defined(PYCC_GCC)
#define PYTHONHOMEHELP "<prefix>/Lib"
#else
#define PYTHONHOMEHELP "<prefix>/pythonX.X"
#endif
#endif

#include "pygetopt.h"

#define COPYRIGHT \
    "Type \"help\", \"copyright\", \"credits\" or \"license\" " \
    "for more information."

#ifdef __cplusplus
extern "C" {
#endif

/* For Py_GetArgcArgv(); set by main() */
static char **orig_argv;
static int  orig_argc;

/* command line options */
#define BASE_OPTS "3bBc:dEhiJm:OQ:RsStuUvVW:xX?"

#ifndef RISCOS
#define PROGRAM_OPTS BASE_OPTS
#else /*RISCOS*/
/* extra option saying that we are running under a special task window
   frontend; especially my_readline will behave different */
#define PROGRAM_OPTS BASE_OPTS "w"
/* corresponding flag */
extern int Py_RISCOSWimpFlag;
#endif /*RISCOS*/

/* Short usage message (with %s for argv0) */
static char *usage_line =
"usage: %s [option] ... [-c cmd | -m mod | file | -] [arg] ...\n";

/* Long usage message, split into parts < 512 bytes */
static char *usage_1 = "\
Options and arguments (and corresponding environment variables):\n\
-b     : issue warnings about comparing bytearray with unicode\n\
         (-bb: issue errors)\n\
-B     : don't write .py[co] files on import; also PYTHONDONTWRITEBYTECODE=x\n\
-c cmd : program passed in as string (terminates option list)\n\
-d     : debug output from parser; also PYTHONDEBUG=x\n\
-E     : ignore PYTHON* environment variables (such as PYTHONPATH)\n\
-h     : print this help message and exit (also --help)\n\
-i     : inspect interactively after running script; forces a prompt even\n\
";
static char *usage_2 = "\
         if stdin does not appear to be a terminal; also PYTHONINSPECT=x\n\
-m mod : run library module as a script (terminates option list)\n\
-O     : optimize generated bytecode slightly; also PYTHONOPTIMIZE=x\n\
-OO    : remove doc-strings in addition to the -O optimizations\n\
-R     : use a pseudo-random salt to make hash() values of various types be\n\
         unpredictable between separate invocations of the interpreter, as\n\
         a defense against denial-of-service attacks\n\
-Q arg : division options: -Qold (default), -Qwarn, -Qwarnall, -Qnew\n\
-s     : don't add user site directory to sys.path; also PYTHONNOUSERSITE\n\
-S     : don't imply 'import site' on initialization\n\
-t     : issue warnings about inconsistent tab usage (-tt: issue errors)\n\
";
static char *usage_3 = "\
-u     : unbuffered binary stdout and stderr; also PYTHONUNBUFFERED=x\n\
         see man page for details on internal buffering relating to '-u'\n\
-v     : verbose (trace import statements); also PYTHONVERBOSE=x\n\
         can be supplied multiple times to increase verbosity\n\
-V     : print the Python version number and exit (also --version)\n\
-W arg : warning control; arg is action:message:category:module:lineno\n\
         also PYTHONWARNINGS=arg\n\
-x     : skip first line of source, allowing use of non-Unix forms of #!cmd\n\
";
static char *usage_4 = "\
-3     : warn about Python 3.x incompatibilities that 2to3 cannot trivially fix\n\
file   : program read from script file\n\
-      : program read from stdin (default; interactive mode if a tty)\n\
arg ...: arguments passed to program in sys.argv[1:]\n\n\
Other environment variables:\n\
PYTHONSTARTUP: file executed on interactive startup (no default)\n\
PYTHONPATH   : '%c'-separated list of directories prefixed to the\n\
               default module search path.  The result is sys.path.\n\
";
static char *usage_5 = "\
PYTHONHOME   : alternate <prefix> directory (or <prefix>%c<exec_prefix>).\n\
               The default module search path uses %s.\n\
PYTHONCASEOK : ignore case in 'import' statements (Windows).\n\
PYTHONIOENCODING: Encoding[:errors] used for stdin/stdout/stderr.\n\
";
static char *usage_6 = "\
PYTHONHASHSEED: if this variable is set to 'random', the effect is the same\n\
   as specifying the -R option: a random value is used to seed the hashes of\n\
   str, bytes and datetime objects.  It can also be set to an integer\n\
   in the range [0,4294967295] to get hash values with a predictable seed.\n\
";


static int
usage(int exitcode, char* program)
{
    FILE *f = exitcode ? stderr : stdout;

    fprintf(f, usage_line, program);
    if (exitcode)
        fprintf(f, "Try `python -h' for more information.\n");
    else {
        fputs(usage_1, f);
        fputs(usage_2, f);
        fputs(usage_3, f);
        fprintf(f, usage_4, DELIM);
        fprintf(f, usage_5, DELIM, PYTHONHOMEHELP);
        fputs(usage_6, f);
    }
#if defined(__VMS)
    if (exitcode == 0) {
        /* suppress 'error' message */
        return 1;
    }
    else {
        /* STS$M_INHIB_MSG + SS$_ABORT */
        return 0x1000002c;
    }
#else
    return exitcode;
#endif
    /*NOTREACHED*/
}

static void RunStartupFile(PyCompilerFlags *cf)
{
    char *startup = Py_GETENV("PYTHONSTARTUP");
    if (startup != NULL && startup[0] != '\0') {
        FILE *fp = fopen(startup, "r");
        if (fp != NULL) {
            (void) PyRun_SimpleFileExFlags(fp, startup, 0, cf);
            PyErr_Clear();
            fclose(fp);
        } else {
            int save_errno;
            save_errno = errno;
            PySys_WriteStderr("Could not open PYTHONSTARTUP\n");
            errno = save_errno;
            PyErr_SetFromErrnoWithFilename(PyExc_IOError,
                                           startup);
            PyErr_Print();
            PyErr_Clear();
        }
    }
}


static int RunModule(char *module, int set_argv0)
{
    PyObject *runpy, *runmodule, *runargs, *result;
    runpy = PyImport_ImportModule("runpy");
    if (runpy == NULL) {
        fprintf(stderr, "Could not import runpy module\n");
        return -1;
    }
    runmodule = PyObject_GetAttrString(runpy, "_run_module_as_main");
    if (runmodule == NULL) {
        fprintf(stderr, "Could not access runpy._run_module_as_main\n");
        Py_DECREF(runpy);
        return -1;
    }
    runargs = Py_BuildValue("(si)", module, set_argv0);
    if (runargs == NULL) {
        fprintf(stderr,
            "Could not create arguments for runpy._run_module_as_main\n");
        Py_DECREF(runpy);
        Py_DECREF(runmodule);
        return -1;
    }
    result = PyObject_Call(runmodule, runargs, NULL);
    if (result == NULL) {
        PyErr_Print();
    }
    Py_DECREF(runpy);
    Py_DECREF(runmodule);
    Py_DECREF(runargs);
    if (result == NULL) {
        return -1;
    }
    Py_DECREF(result);
    return 0;
}

static int RunMainFromImporter(char *filename)
{
    PyObject *argv0 = NULL, *importer = NULL;

    if ((argv0 = PyString_FromString(filename)) &&
        (importer = PyImport_GetImporter(argv0)) &&
        (importer->ob_type != &PyNullImporter_Type))
    {
             /* argv0 is usable as an import source, so
                    put it in sys.path[0] and import __main__ */
        PyObject *sys_path = NULL;
        if ((sys_path = PySys_GetObject("path")) &&
            !PyList_SetItem(sys_path, 0, argv0))
        {
            Py_INCREF(argv0);
            Py_DECREF(importer);
            sys_path = NULL;
            return RunModule("__main__", 0) != 0;
        }
    }
    Py_XDECREF(argv0);
    Py_XDECREF(importer);
    if (PyErr_Occurred()) {
        PyErr_Print();
        return 1;
    }
    return -1;
}


/* Main program */

int
Py_Main(int argc, char **argv)
{
    int c;
    int sts;
    char *command = NULL;
    char *filename = NULL;
    char *module = NULL;
    FILE *fp = stdin;
    char *p;
    int unbuffered = 0;
    int skipfirstline = 0;
    int stdin_is_interactive = 0;
    int help = 0;
    int version = 0;
    int saw_unbuffered_flag = 0;
    PyCompilerFlags cf;

    cf.cf_flags = 0;

    orig_argc = argc;           /* For Py_GetArgcArgv() */
    orig_argv = argv;

#ifdef RISCOS
    Py_RISCOSWimpFlag = 0;
#endif

    /* Hash randomization needed early for all string operations
       (including -W and -X options). */
    _PyOS_opterr = 0;  /* prevent printing the error in 1st pass */
    while ((c = _PyOS_GetOpt(argc, argv, PROGRAM_OPTS)) != EOF) {
        if (c == 'm' || c == 'c') {
            /* -c / -m is the last option: following arguments are
               not interpreter options. */
            break;
        }
        switch (c) {
        case 'E':
            Py_IgnoreEnvironmentFlag++;
            break;
        case 'R':
            Py_HashRandomizationFlag++;
            break;
        }
    }
    /* The variable is only tested for existence here; _PyRandom_Init will
       check its value further. */
    if (!Py_HashRandomizationFlag &&
        (p = Py_GETENV("PYTHONHASHSEED")) && *p != '\0')
        Py_HashRandomizationFlag = 1;

    _PyRandom_Init();

    PySys_ResetWarnOptions();
    _PyOS_ResetGetOpt();

    while ((c = _PyOS_GetOpt(argc, argv, PROGRAM_OPTS)) != EOF) {
        if (c == 'c') {
            /* -c is the last option; following arguments
               that look like options are left for the
               command to interpret. */
            command = (char *)malloc(strlen(_PyOS_optarg) + 2);
            if (command == NULL)
                Py_FatalError(
                   "not enough memory to copy -c argument");
            strcpy(command, _PyOS_optarg);
            strcat(command, "\n");
            break;
        }

        if (c == 'm') {
            /* -m is the last option; following arguments
               that look like options are left for the
               module to interpret. */
            module = (char *)malloc(strlen(_PyOS_optarg) + 2);
            if (module == NULL)
                Py_FatalError(
                   "not enough memory to copy -m argument");
            strcpy(module, _PyOS_optarg);
            break;
        }

        switch (c) {
        case 'b':
            Py_BytesWarningFlag++;
            break;

        case 'd':
            Py_DebugFlag++;
            break;

        case '3':
            Py_Py3kWarningFlag++;
            if (!Py_DivisionWarningFlag)
                Py_DivisionWarningFlag = 1;
            break;

        case 'Q':
            if (strcmp(_PyOS_optarg, "old") == 0) {
                Py_DivisionWarningFlag = 0;
                break;
            }
            if (strcmp(_PyOS_optarg, "warn") == 0) {
                Py_DivisionWarningFlag = 1;
                break;
            }
            if (strcmp(_PyOS_optarg, "warnall") == 0) {
                Py_DivisionWarningFlag = 2;
                break;
            }
            if (strcmp(_PyOS_optarg, "new") == 0) {
                /* This only affects __main__ */
                cf.cf_flags |= CO_FUTURE_DIVISION;
                /* And this tells the eval loop to treat
                   BINARY_DIVIDE as BINARY_TRUE_DIVIDE */
                _Py_QnewFlag = 1;
                break;
            }
            fprintf(stderr,
                "-Q option should be `-Qold', "
                "`-Qwarn', `-Qwarnall', or `-Qnew' only\n");
            return usage(2, argv[0]);
            /* NOTREACHED */

        case 'i':
            Py_InspectFlag++;
            Py_InteractiveFlag++;
            break;

        /* case 'J': reserved for Jython */

        case 'O':
            Py_OptimizeFlag++;
            break;

        case 'B':
            Py_DontWriteBytecodeFlag++;
            break;

        case 's':
            Py_NoUserSiteDirectory++;
            break;

        case 'S':
            Py_NoSiteFlag++;
            break;

        case 'E':
            /* Already handled above */
            break;

        case 't':
            Py_TabcheckFlag++;
            break;

        case 'u':
            unbuffered++;
            saw_unbuffered_flag = 1;
            break;

        case 'v':
            Py_VerboseFlag++;
            break;

#ifdef RISCOS
        case 'w':
            Py_RISCOSWimpFlag = 1;
            break;
#endif

        case 'x':
            skipfirstline = 1;
            break;

        /* case 'X': reserved for implementation-specific arguments */

        case 'U':
            Py_UnicodeFlag++;
            break;
        case 'h':
        case '?':
            help++;
            break;
        case 'V':
            version++;
            break;

        case 'W':
            PySys_AddWarnOption(_PyOS_optarg);
            break;

        case 'R':
            /* Already handled above */
            break;

        /* This space reserved for other options */

        default:
            return usage(2, argv[0]);
            /*NOTREACHED*/

        }
    }

    if (help)
        return usage(0, argv[0]);

    if (version) {
        fprintf(stderr, "Python %s\n", PY_VERSION);
        return 0;
    }

    if (Py_Py3kWarningFlag && !Py_TabcheckFlag)
        /* -3 implies -t (but not -tt) */
        Py_TabcheckFlag = 1;

    if (!Py_InspectFlag &&
        (p = Py_GETENV("PYTHONINSPECT")) && *p != '\0')
        Py_InspectFlag = 1;
    if (!saw_unbuffered_flag &&
        (p = Py_GETENV("PYTHONUNBUFFERED")) && *p != '\0')
        unbuffered = 1;

    if (!Py_NoUserSiteDirectory &&
        (p = Py_GETENV("PYTHONNOUSERSITE")) && *p != '\0')
        Py_NoUserSiteDirectory = 1;

    if ((p = Py_GETENV("PYTHONWARNINGS")) && *p != '\0') {
        char *buf, *warning;

        buf = (char *)malloc(strlen(p) + 1);
        if (buf == NULL)
            Py_FatalError(
               "not enough memory to copy PYTHONWARNINGS");
        strcpy(buf, p);
        for (warning = strtok(buf, ",");
             warning != NULL;
             warning = strtok(NULL, ","))
            PySys_AddWarnOption(warning);
        free(buf);
    }

    if (command == NULL && module == NULL && _PyOS_optind < argc &&
        strcmp(argv[_PyOS_optind], "-") != 0)
    {
#ifdef __VMS
        filename = decc$translate_vms(argv[_PyOS_optind]);
        if (filename == (char *)0 || filename == (char *)-1)
            filename = argv[_PyOS_optind];

#else
        filename = argv[_PyOS_optind];
#endif
    }

    stdin_is_interactive = Py_FdIsInteractive(stdin, (char *)0);

    if (unbuffered) {
#if defined(MS_WINDOWS) || defined(__CYGWIN__)
        _setmode(fileno(stdin), O_BINARY);
        _setmode(fileno(stdout), O_BINARY);
#endif
#ifdef HAVE_SETVBUF
        setvbuf(stdin,  (char *)NULL, _IONBF, BUFSIZ);
        setvbuf(stdout, (char *)NULL, _IONBF, BUFSIZ);
        setvbuf(stderr, (char *)NULL, _IONBF, BUFSIZ);
#else /* !HAVE_SETVBUF */
        setbuf(stdin,  (char *)NULL);
        setbuf(stdout, (char *)NULL);
        setbuf(stderr, (char *)NULL);
#endif /* !HAVE_SETVBUF */
    }
    else if (Py_InteractiveFlag) {
#ifdef MS_WINDOWS
        /* Doesn't have to have line-buffered -- use unbuffered */
        /* Any set[v]buf(stdin, ...) screws up Tkinter :-( */
        setvbuf(stdout, (char *)NULL, _IONBF, BUFSIZ);
#else /* !MS_WINDOWS */
#ifdef HAVE_SETVBUF
        setvbuf(stdin,  (char *)NULL, _IOLBF, BUFSIZ);
        setvbuf(stdout, (char *)NULL, _IOLBF, BUFSIZ);
#endif /* HAVE_SETVBUF */
#endif /* !MS_WINDOWS */
        /* Leave stderr alone - it should be unbuffered anyway. */
    }
#ifdef __VMS
    else {
        setvbuf (stdout, (char *)NULL, _IOLBF, BUFSIZ);
    }
#endif /* __VMS */

#ifdef __APPLE__
    /* On MacOS X, when the Python interpreter is embedded in an
       application bundle, it gets executed by a bootstrapping script
       that does os.execve() with an argv[0] that's different from the
       actual Python executable. This is needed to keep the Finder happy,
       or rather, to work around Apple's overly strict requirements of
       the process name. However, we still need a usable sys.executable,
       so the actual executable path is passed in an environment variable.
       See Lib/plat-mac/bundlebuiler.py for details about the bootstrap
       script. */
    if ((p = Py_GETENV("PYTHONEXECUTABLE")) && *p != '\0')
        Py_SetProgramName(p);
    else
        Py_SetProgramName(argv[0]);
#else
    Py_SetProgramName(argv[0]);
#endif
    Py_Initialize();

    if (Py_VerboseFlag ||
        (command == NULL && filename == NULL && module == NULL && stdin_is_interactive)) {
        fprintf(stderr, "Python %s on %s\n",
            Py_GetVersion(), Py_GetPlatform());
        if (!Py_NoSiteFlag)
            fprintf(stderr, "%s\n", COPYRIGHT);
    }

    if (command != NULL) {
        /* Backup _PyOS_optind and force sys.argv[0] = '-c' */
        _PyOS_optind--;
        argv[_PyOS_optind] = "-c";
    }

    if (module != NULL) {
        /* Backup _PyOS_optind and force sys.argv[0] = '-c'
           so that PySys_SetArgv correctly sets sys.path[0] to ''
           rather than looking for a file called "-m". See
           tracker issue #8202 for details. */
        _PyOS_optind--;
        argv[_PyOS_optind] = "-c";
    }

    PySys_SetArgv(argc-_PyOS_optind, argv+_PyOS_optind);

    if ((Py_InspectFlag || (command == NULL && filename == NULL && module == NULL)) &&
        isatty(fileno(stdin))) {
        PyObject *v;
        v = PyImport_ImportModule("readline");
        if (v == NULL)
            PyErr_Clear();
        else
            Py_DECREF(v);
    }

    if (command) {
        sts = PyRun_SimpleStringFlags(command, &cf) != 0;
        free(command);
    } else if (module) {
        sts = (RunModule(module, 1) != 0);
        free(module);
    }
    else {

        if (filename == NULL && stdin_is_interactive) {
            Py_InspectFlag = 0; /* do exit on SystemExit */
            RunStartupFile(&cf);
        }
        /* XXX */

        sts = -1;               /* keep track of whether we've already run __main__ */

        if (filename != NULL) {
            sts = RunMainFromImporter(filename);
        }

        if (sts==-1 && filename!=NULL) {
            if ((fp = fopen(filename, "r")) == NULL) {
                fprintf(stderr, "%s: can't open file '%s': [Errno %d] %s\n",
                    argv[0], filename, errno, strerror(errno));

                return 2;
            }
            else if (skipfirstline) {
                int ch;
                /* Push back first newline so line numbers
                   remain the same */
                while ((ch = getc(fp)) != EOF) {
                    if (ch == '\n') {
                        (void)ungetc(ch, fp);
                        break;
                    }
                }
            }
            {
                /* XXX: does this work on Win/Win64? (see posix_fstat) */
                struct stat sb;
                if (fstat(fileno(fp), &sb) == 0 &&
                    S_ISDIR(sb.st_mode)) {
                    fprintf(stderr, "%s: '%s' is a directory, cannot continue\n", argv[0], filename);
                    fclose(fp);
                    return 1;
                }
            }
        }

        if (sts==-1) {
            /* call pending calls like signal handlers (SIGINT) */
            if (Py_MakePendingCalls() == -1) {
                PyErr_Print();
                sts = 1;
            } else {
                sts = PyRun_AnyFileExFlags(
                    fp,
                    filename == NULL ? "<stdin>" : filename,
                    filename != NULL, &cf) != 0;
            }
        }

    }

    /* Check this environment variable at the end, to give programs the
     * opportunity to set it from Python.
     */
    if (!Py_InspectFlag &&
        (p = Py_GETENV("PYTHONINSPECT")) && *p != '\0')
    {
        Py_InspectFlag = 1;
    }

    if (Py_InspectFlag && stdin_is_interactive &&
        (filename != NULL || command != NULL || module != NULL)) {
        Py_InspectFlag = 0;
        /* XXX */
        sts = PyRun_AnyFileFlags(stdin, "<stdin>", &cf) != 0;
    }

    Py_Finalize();
#ifdef RISCOS
    if (Py_RISCOSWimpFlag)
        fprintf(stderr, "\x0cq\x0c"); /* make frontend quit */
#endif

#ifdef __INSURE__
    /* Insure++ is a memory analysis tool that aids in discovering
     * memory leaks and other memory problems.  On Python exit, the
     * interned string dictionary is flagged as being in use at exit
     * (which it is).  Under normal circumstances, this is fine because
     * the memory will be automatically reclaimed by the system.  Under
     * memory debugging, it's a huge source of useless noise, so we
     * trade off slower shutdown for less distraction in the memory
     * reports.  -baw
     */
    _Py_ReleaseInternedStrings();
#endif /* __INSURE__ */

    return sts;
}

/* this is gonna seem *real weird*, but if you put some other code between
   Py_Main() and Py_GetArgcArgv() you will need to adjust the test in the
   while statement in Misc/gdbinit:ppystack */

/* Make the *original* argc/argv available to other modules.
   This is rare, but it is needed by the secureware extension. */

void
Py_GetArgcArgv(int *argc, char ***argv)
{
    *argc = orig_argc;
    *argv = orig_argv;
}

#ifdef __cplusplus
}
#endif

#ifdef _AMIGA
/* Amiga-specific module initialization */
Py_InitModule4("amiga", AmigaMethods, ...);
#endif

