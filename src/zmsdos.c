/*****************************************************************************
        Zmsdos.c

        System dependent code for MS-DOS and the Turbo C v2.0 compiler.
*****************************************************************************/

#include <alloc.h>              /* prototype for farfree(), farmalloc() */
#include <dir.h>                /* prototype for findfirst(), findnext() */
#include <dos.h>                /* prototypes for sound(), nosound(), etc. */
#include <ctype.h>              /* for tolower() */
#include <errno.h>              /* define errno */
#include <fcntl.h>              /* O_BINARY */
#include <io.h>                 /* prototype for access() */
#include <process.h>            /* prototype for execlp() */
#include <stdio.h>              /* prototype for fread(), fwrite()... */
#include <stdlib.h>             /* prototype for exit(), system() */
#include <string.h>             /* prototypes for strlen(), strcat(), ... */
#include <utime.h>              /* prototype for utime() */
#include <sys/stat.h>           /* prototype for stat() */
#include <malloc.h>

#include "zport.h"              /* define portability identifiers */
#include "tecoc.h"              /* define general identifiers */
#include "defext.h"             /* define external global variables */
#include "chmacs.h"             /* define character processing macros */
#include "clpars.h"             /* command-line parsing macro */
#include "deferr.h"             /* define identifiers for error messages */
#include "dchars.h"             /* define identifiers for characters */
#include "dscren.h"             /* define identifiers for screen i/o */

#define CHUNK 0xFFF0U           /* 64K-16 bytes, used in ZCpyBl & Zfwrite */

#if USE_PROTOTYPES
static SIZE_T Zfwrite(charptr Buffer, SIZE_T Length, FILE *fp);
static int _Cdecl CntrlC(void);
#endif

/*****************************************************************************
        Keep track of Malloc'd data
*****************************************************************************/

#if DEBUGGING

#define MMAX 256                /* max number of malloc'ed items */
static  voidptr MPtrs[MMAX];    /* save malloc'ed pointers */

#endif

/*****************************************************************************
        Define Power C's cursor positioning functions in terms of Turbo C's
functions.  Be aware that Power C's upper left corner coordinates are (0,0)
while Turbo C's are (1,1).
*****************************************************************************/

#if defined (__POWERC)
#include <bios.h>
#define wherex()        (curscol()+1)
#define wherey()        (cursrow()+1)
#define gotoxy(x,y)     poscurs((x)-1,(y)-1)
#endif

/*****************************************************************************
        The following static data is used in ZPWild and ZSWild to
perform wildcard filename lookups under DOS.

        Since Turbo C findfirst and findnext only return the filename
and extension of a file, we save the drive code and subdirectory path
in ff_drive[] and ff_dir[] when we call ZPWild so we can return a fully
expanded filename later on.

        After a wildcard filename has been preset with ENfilename$, on the
first call to EN$, we find all the matching filenames in the directory and
save them in the dynamically allocated string ff_names.  We use ff_names_p
to point into ff_names when returning filenames on subsequent EN$ calls.
We read the entire directory instead of simply findnext'ing for a new
filename to avoid the scenario where you get a filename from EN$, and
then modify and save it.  The original filename will be renamed filename.BAK
and a new filename created.  The new filename's directory entry may appear
after the filename.BAK entry in the DOS directory list, and therefore may
be returned on a subsequent EN$.  Since we don't want this to happen, we
read the entire directory at once.

        The ff_preset flag is:  -1 if the wildcard lookup hasn't been preset
yet;  0 if we haven't done any findfirst/findnext'ing yet; or, 1 if ff_names
should hold names returned by findfirst/findnext.

        ??? Making ff_drive[] & ff_dir[] global will cause problems later on
if we want to recurse down subdirectories.  Also, we'll be doing a lot of
ZRaloc'ing if we try to read a humongous directory.

*****************************************************************************/

static  char            ff_dir[MAXDIR];         /* filename path name */
static  char            ff_drive[MAXDRIVE];     /* filename drive code */
static  char            ff_path[MAXPATH];       /* complete EN filespec */
static  charptr         ff_names = NULL;        /* filenames found */
static  charptr         ff_names_p = NULL;      /* pointer into ff_names */
static  int             ff_preset = -1;         /* say ZPWild not called */

/*****************************************************************************
        IFiles holds the file data blocks for input files.  There are three
static input streams:   the primary input stream,  the secondary input stream,
and the input stream used by the EQq command.  To access these three files,
identifiers defined in file tecoc.h are used to index into this array.
Other elements of this array are used to access input files for the EI
command.
*****************************************************************************/

FILE *IFiles[NIFDBS];

/*****************************************************************************
        OFiles holds the file data blocks for the output files.  There are
three output streams:   the primary output stream,  the secondary output
stream and the output stream used by the E%q command.  The array is indexed
using identifiers defined in file tecoc.h.
*****************************************************************************/

static struct {
        char    OFNam[FILENAME_MAX];    /* output file name */
        char    OTNam[FILENAME_MAX];    /* temporary output file name */
        FILE    *OStrem;                /* stream */
    BOOLEAN forBackup;          /* TAA Mod */
} OFiles[NOFDBS];

/*****************************************************************************

        ZErMsg()

        This function displays an error message from the operating system on
the terminal screen.  The error message text is retrieved from the operating
system and imbedded in a TECO-style message with the SYS mnemonic.

*****************************************************************************/

static VVOID ZErMsg(void)
{
        char *ErrMsg;
        char *cp;

        if ((ErrMsg = strerror(errno)) != NULL) {
                cp = ErrMsg + strlen(ErrMsg) - 1;
                if (*cp == '\n') {
                        *cp = '\0';
                }
        } else {
                ErrMsg = "???";
        }
        ErrStr(ERR_SYS, ErrMsg);
}

/*****************************************************************************

        Zfwrite()

        This function implements fwrite() for the large model.  The Turbo C
v2.0 fwrite() returns and takes size_t as it's size arguments.  Under Turbo,
size_t is always a 16-bit unsigned int.  Therefore, fwrite() can't write more
than 64K at a time.  Zfwrite() decomposes writes larger than 64K into CHUNK
size bytes.  CHUNK is 16 bytes less than 64K since writes close to 64K can be
dangerous.

*****************************************************************************/

static SIZE_T Zfwrite(charptr Buffer, SIZE_T Length, FILE *fp)
{
        size_t  NBytes;         /* number of bytes to write (0-64K) */
        size_t  WBytes;         /* number of bytes written */
        SIZE_T  RetVal;         /* return value */

        RetVal = 0;
        while (Length != 0) {
                NBytes = (Length > CHUNK) ? CHUNK : (size_t) Length;
                WBytes = fwrite((char *)Buffer, sizeof(char), NBytes, fp);
                RetVal += WBytes;
                if (WBytes != NBytes) {
                        break;
                }
                Length -= NBytes;
                Buffer += NBytes;
        }
        return RetVal;
}

/*****************************************************************************

        ZAlloc()

        This function allocates memory.  The single argument is the number
of bytes to allocate.  TECO-C uses the ZFree() and ZRaloc() functions to
de-allocate and re-allocate, respectively,  the memory allocated by this
function.

*****************************************************************************/

voidptr ZAlloc(SIZE_T MemSiz)           /* allocate memory */
{
        voidptr NewBlk;

#if DEBUGGING
        static char *DbgFNm = "ZAlloc";
        sprintf(DbgSBf,"MemSiz = %ld", MemSiz);
        DbgFEn(4,DbgFNm,DbgSBf);
#endif

        NewBlk = farmalloc(MemSiz);

#if DEBUGGING

/*
 * is the heap corrupted?
 */

#if defined(__TURBOC__) && (__TURBOC__ >= 0x0295)
        if (heapcheck () < 0) {
                puts ("ZAlloc: heapcheck failed");
                exit (1);
        }
#endif

/*
 * save malloc'ed block in first NULL entry in MPtrs[]
 */

        if (NewBlk != NULL) {
                int i;

                for (i = 0; i < MMAX && MPtrs[i] != NULL; ++i) {
                        ;
                }
                if (i == MMAX) {
                        puts ("ZAlloc: MPtrs[] stack exceeded");
                        exit (1);
                }
                MPtrs[i] = NewBlk;
        }

#endif

        DBGFEX(4,DbgFNm,(NewBlk == NULL) ? "NULL" : NULL);
        return NewBlk;
}

/*****************************************************************************

        ZBell()

        Thus function rings the terminal bell.  For most platforms,  this
means just writing a bell character (control-G) to the terminal.  Under
MS-DOS, ringing the bell this way produces a yucky sound,  so for MS-DOS
this function controls the signal generator directly.

*****************************************************************************/

VVOID ZBell(void)
{
#ifdef WIN32
	putch('\a');
#else
        sound(0);               /* turns PC speaker on at 0 hertz */
        sound(800);             /* turns PC speaker on at 800 hertz */
        delay(10);              /* suspend execution for 10 milliseconds */
        nosound();              /* turn PC speaker off */
#endif
}

/*****************************************************************************

        ZChIn()

        This function inputs a single character from the terminal.

        1.  the character is not echoed on the terminal
        2.  ^C calls an interrupt routine.  Note that this must be
            implemented so that a ^C will cancel a current output via
            ZDspBf.  The ^C must be a true interrupt.
        3.  type-ahead is always nice
        4.  The character must be returned immediately:  no fooling
            around waiting for a carriage-return before returning.
        5.  If the NoWait argument is TRUE, don't wait
        6.  When the user hits the RETURN key,  TECO is supposed to see
            a carriage return and then a line feed.  The function must
            deal with this by returning a carriage return to the caller
            and then "remembering" to send a line feed on the next call.
        7.  handle ET_BKSP_IS_DEL flag

*****************************************************************************/

DEFAULT ZChIn(BOOLEAN NoWait)   /* input a character from terminal */
{
        int Charac;
        static BOOLEAN NeedLF = FALSE;

        if (NeedLF) {
                NeedLF = FALSE;
                return (DEFAULT)LINEFD;
        }
        if (NoWait && (kbhit() == 0)) {         /* if no char available */
                return -1;                      /* return immediately */
        }
        for (;;) {
                Charac = getch();
                if (Charac == 0x03) {           /* ^C? */
                        CntrlC ();
                        break;
                }
                if (Charac != 0) {              /* not an IBM PC scan code? */
                        break;                  /* ??? what happens on ^@? */
                }
                Charac = getch();               /* get 2nd scan code byte */
                if (Charac == 0x53) {           /* DEL key? */
                        Charac = DELETE;
                        break;
                }
        }

        if (Charac == CRETRN) {
                NeedLF = TRUE;
        } else {
                if (EtFlag & ET_BKSP_IS_DEL) {
                        if (Charac == DELETE) {
                                Charac = BAKSPC;
                        } else if (Charac == BAKSPC) {
                                Charac = DELETE;
                        }
                }
        }
        return (DEFAULT)Charac;
}

/*****************************************************************************

        ZClnEG()

        This function executes a :EG command.  The :EG commands are used to
get access to operating system functions.  The minimum set of functions is

        :EGINI$         gets, sets or clears the initialization file name
        :EGMEM$         gets, sets or clears the file name memory
        :EGLIB$         gets, sets or clears the macro library directory
        :EGVTE$         gets, sets or clears the video macro file name

although more functions may be defined.  This functions returns 0L on failure
or -1L on success.

        Under MS-DOS, the values of the INI, MEM, LIB, and VTE functions are
stored in the TEC$INIT, TEC$MEMORY, TEC$LIBRARY, and TEC$VTEDIT environment
variables.

*****************************************************************************/

LONG ZClnEG(                    /* execute special :EG command */
        DEFAULT EGWhat,         /* what to get/set/clear: MEM, LIB, etc. */
        DEFAULT EGOper,         /* operation: get, set or clear */
        charptr TxtPtr)         /* if setting,  value to set */
{
#if 0
        EGWhat = EGWhat;        /* stop TC parameter not used warnings */
        EGOper = EGOper;
        TxtPtr = TxtPtr;
        return 0L;              /* return failure */
#else

        char    *cp=NULL;       /* environment variable name */
        char    buf[100];       /* enough for envname + 80 char filename */
        LONG    retval;         /* -1L on success, 0L on failure */

        DBGFEN(2,"ZClnEG",NULL);
        DBGFEX(2,DbgFNm,"0");

        switch (EGWhat) {
            case EG_INI: cp = "TEC$INIT";    break;
            case EG_LIB: cp = "TEC$LIBRARY"; break;
            case EG_MEM: cp = "TEC$MEMORY";  break;
#if VIDEO
            case EG_VTE: cp = "TEC$VTEDIT";  break;
#endif
			default: return 0L;
        }

        if (EGOper == GET_VAL) {
                if ((cp = getenv(cp)) == NULL) {
                        retval = 0L;            /* return failure */
                } else {
                        retval = -1L;           /* success, copy to FBf */
                        strcpy((char*)FBfBeg, cp);
                        FBfPtr = FBfBeg + strlen(cp);
                }
        } else {
                strcpy(buf, cp);                /* build NAME= */
                strcat(buf, "=");
                if (EGOper == SET_VAL) {        /* concatenate new value */
                        strcat(buf, (char *)TxtPtr);
                }
                retval = (putenv(buf) != 0)     /* if putenv() failed */
                                ? 0L            /*   then return failure */
                                : -1L;          /*   else return success */
        }
        return retval;
#endif
}

/*****************************************************************************

        ZClnUp()

        This function cleans up in preparation for terminating TECO-C.

*****************************************************************************/

VVOID ZClnUp(VVOID)                     /* cleanup for TECO-C abort */
{
        DBGFEN(3,"ZClnUp",NULL);
#if CHECKSUM_CODE                       /* MS-DOS debugging code */
        check_code_checksums ();        /* check one final time */
#endif

        ZDspBf("\r\n", 2);              /* final carriage return/line feed */
}

/*****************************************************************************

        ZCpyBl()

        This function implements memmove() for the large data model.  The
Turbo C v2.0 memory move functions all deal with size_t bytes.  Since size_t
is always a 16-bit unsigned int, memmove() and memcpy() can't move blocks
larger than 64K.  ZCpyBl() decomposes moves larger than 64K into CHUNK size
bytes.  CHUNK is 16 bytes less than 64K since moves close to 64K can be
dangerous.

*****************************************************************************/

VVOID ZCpyBl(charptr Destin, charptr Source, SIZE_T Length)
{
        size_t  NBytes;         /* number of bytes to move (0-64K) */

        if (Source < Destin) {          /* do move backwards */
                Source += Length;
                Destin += Length;
                while (Length > 0) {
                        NBytes = (Length > CHUNK) ? CHUNK : (size_t) Length;
                        Length -= NBytes;
                        Source -= NBytes;
                        Destin -= NBytes;
                        memmove((void *) Destin, (void *) Source, NBytes);
                }
        } else {                                /* do move forwards */
                while (Length > 0) {
                        NBytes = (Length > CHUNK) ? CHUNK : (size_t) Length;
                        memcpy((void *) Destin, (void *) Source, NBytes);
                        Length -= NBytes;
                        Source += NBytes;
                        Destin += NBytes;
                }
        }
}

/*****************************************************************************

        Zcp2ul()

        Converts a huge pointer to a long for debugging messages.

*****************************************************************************/

#if DEBUGGING || CONSISTENCY_CHECKING
ULONG Zcp2ul(voidptr cp)                /* convert voidptr to ULONG */
{
#if TC_SMALL_DATA
        return (unsigned long) cp;
#else
        return (((ULONG) FP_SEG(cp)) << 4) + ((ULONG) FP_OFF(cp));
#endif
}
#endif

/*****************************************************************************

        ZDoCmd()

        This function terminates TECO and feeds a command line to the
command line interpreter.  The command to be executed is passed to this
function in the file name buffer (FBf).

*****************************************************************************/

VVOID ZDoCmd(charptr GBfBeg, charptr GBfPtr) /* die and pass command to OS */
{
        char    *space_p;
        char    *comspec;

        DBGFEN(1,"ZDoCmd",NULL);

/*
 * 1. Terminate command line in GBf
 * 2. separate program name from arguments, if any
 * 3. Call ZClnUp to free up everything
 * 4. Execute the command line, with optional arguments.  If we know where
 *    the command processor is, use it so we can execute .BAT batch files
 * 5. we shouldn't be here, exit
 */

        *GBfPtr = '\0';
        if ((space_p = strchr ((char *)GBfBeg,' ')) != NULL) {
                *space_p++ = '\0';
        }
        ZClnUp ();

        if ((comspec = getenv("COMSPEC")) != NULL) {
                execlp (comspec,
                        comspec,
                        " /c ",
                        GBfBeg,
                        (space_p) ? space_p : NULL, NULL);
        } else {
#ifdef WIN32
			execlp ((char *)GBfBeg,
					(char *)GBfBeg,
					(space_p) ? space_p : NULL, NULL);
#else
                execlp ((char _far *)GBfBeg,
                        (char _far *)GBfBeg,
                        (space_p) ? space_p : NULL, NULL);
#endif
        }
        perror (NULL);
        ZExit (EXIT_SUCCESS);
}

/*****************************************************************************

        ZDspBf()

        This function displays a buffer of a given length on the terminal
screen.  On the VAX (and maybe other systems) doing any kind of output
involves a fair amount of overhead,  regardless of the size of the buffer
being output.  It is therefore better to make a single call to the operating
system's output function than to call the function for each and every
character.  If such improvements do not apply to the system this program
is running on,  then this function can simply manually output every character
in the buffer.

*****************************************************************************/

VVOID ZDspBf(charptr buffer, SIZE_T length)
{
#if DEBUGGING
    static char *DbgFNm = "ZDspBf";
    sprintf(DbgSBf,"buffer = %ld, length = %ld", Zcp2ul(buffer), length);
    DbgFEn(5,DbgFNm,DbgSBf);
#endif

    if (Zfwrite (buffer, length, stdout) != length) {
        puts("\nZDspBf: Zfwrite() failed, unable to write to terminal");
#if DEBUGGING
        printf("*buffer = %X(h) %o(o) %d(d)\n", *buffer, *buffer, *buffer);
        perror("perror");
#endif
        TAbort(EXIT_FAILURE);
    }

    DBGFEX(5,DbgFNm,NULL);
}

/*****************************************************************************

        ZDspCh()

        This function outputs a single character to the terminal.

*****************************************************************************/

VVOID ZDspCh(char Charac)               /* output a character to terminal */
{
        if (fwrite(&Charac, sizeof(char), 1, stdout) != 1)
                TAbort(EXIT_FAILURE);
}

/*****************************************************************************

        ZExCtB()

        This function implements the TECO ^B command,  which returns the
current date encoded in the following way:

                ((year-1900)*16+month)*32+day

        This function calls DOS to get the current date.

*****************************************************************************/

DEFAULT ZExCtB(void)                    /* return current date */
{
#ifdef WIN32
	struct date foo;
	getdate(&foo);
	return PushEx(((foo.da_year-1900)*16+foo.da_mon)*32+foo.da_day, OPERAND);
#else
        unsigned int    m;                      /* month */
        unsigned int    d;                      /* day */
        unsigned int    y;                      /* year */
        LONG            td;                     /* encoded teco date */
        union REGS      regs;                   /* registers for intdos() */

        memset (&regs, 0, sizeof (union REGS));
        td = 0;

        regs.h.ah = 0x2A;                       /* AH = get date function */
        intdos (&regs, &regs);                  /* call DOS via INT 0x21 */

        if (regs.x.cflag != 0) {                /* if DOS error */
                /* should we report DOS error??? */
        } else {
                m = regs.h.dh;                  /* DH = month (1-12) */
                d = regs.h.dl;                  /* DL = day   (1-31) */
                y = regs.x.cx;                  /* CX = year  (1980-2099) */
                if (m < 1 || m > 12 ||
                    d < 1 || d > 31 ||
                    y < 1980 || y > 2099) {
                        /* should we report illegal date??? */
                } else {
                        td = (((LONG)y - 1900) * 16 + (LONG)m) * 32 + (LONG)d;
                }
        }

        return PushEx(td, OPERAND);             /* return date on stack */
#endif
}

/*****************************************************************************

        ZExCtH()

        This function implements the TECO ^H command,  which returns the
current time encoded in the following way:

                (seconds since midnight) / 2

        This function calls DOS to get the current time.  Alternatively,
this function could directly interrogate the master clock count at 0040:006C
which is the number of timer ticks since midnight.

        Note:  to execute this function, enter <CARET>H, as ^H is interpreted
as a backspace.

*****************************************************************************/

DEFAULT ZExCtH(void)                    /* return current time */
{
#ifdef WIN32
	struct time foo;
	gettime(&foo);
	return PushEx((foo.ti_hour*60+foo.ti_min)*30+foo.ti_sec/2, OPERAND);
#else
        unsigned int    h;                      /* hours */
        unsigned int    m;                      /* minutes */
        unsigned int    s;                      /* seconds */
        LONG            tt;                     /* encoded teco time */
        union REGS      regs;                   /* registers for intdos() */

        memset (&regs, 0, sizeof (union REGS));
        tt = 0;

        regs.h.ah = 0x2C;                       /* AH = get time function */
        intdos (&regs, &regs);                  /* call DOS via INT 0x21 */

        if (regs.x.cflag != 0) {                /* if DOS error */
                /* should we report DOS error? */
        } else {
                h = regs.h.ch;                  /* CH = hour (0-23) */
                m = regs.h.cl;                  /* CL = minutes (0-59) */
                s = regs.h.dh;                  /* DH = seconds (0-59) */
                if (h > 23 || m > 59 || s > 59) {
                        /* should we report illegal time? */
                } else {
/*
 * this is the same as (h * 3600 + m * 60 + s) / 2
 */
                        tt = (LONG)h * 1800 + (LONG)m * 30 + (LONG)s / 2;
                }
        }

        return PushEx(tt, OPERAND);     /* return time on stack */
#endif
}

/*****************************************************************************

        ZExeEJ()

        This function executes an EJ command,  which returns environment
characteristics.  It returns:

        -1EJ    1024 under VAX/VMS      (4*256 = VAX, 0 = VMS in native mode)
                1025 under Ultrix       (4*256 = VAX, 1 = Ultrix)
                25600 under Sun/SunOS   (100*256 = Sun, 0 = SunOS)
                25856 under MS-DOS      (101*256 = IBM-PC, 0 = MS-DOS)

        0EJ     process id on VAXen, 0 on anything else

        1EJ     0 on all systems

        2EJ     UIC, in longword format (unlike TECO-11) on VAX/VMS,
                0 on all other systems.

*****************************************************************************/

DEFAULT ZExeEJ(void)                    /* execute an EJ command */
{
        DBGFEN(1,"ZExeEJ",NULL);

        if (EStTop == EStBot) {                 /* if no numeric argument */
                NArgmt = 0;                     /* default is 0EJ */
        } else {
                UMinus();                       /* if -EJ, make it -1EJ */
                if (GetNmA() == FAILURE) {      /* get numeric argument */
                        DBGFEX(1,DbgFNm,"FAILURE, GetNmA() failed");
                        return FAILURE;
                }
        }

        if (NArgmt == -1) {
#ifdef WIN32
			DBGFEX(1,DbgFNm,"PushEx(25857)");
			return PushEx((LONG)25857, OPERAND);    /* means "WIN-32" or OS/2 */
#else
			DBGFEX(1,DbgFNm,"PushEx(25856)");
			return PushEx((LONG)25856, OPERAND);    /* means "MS-DOS" */
#endif
        }

        if (NArgmt == 0) {
                DBGFEX(1,DbgFNm,"PushEx(0)");
                return PushEx(0L, OPERAND);     /* process id */
        }

        DBGFEX(1,DbgFNm,"ExeNYI");
        return ExeNYI();
}

/*****************************************************************************

        ZExit()

        This function terminates TECO-C with a status value.

*****************************************************************************/

VVOID ZExit(DEFAULT estat)              /* terminate TECO-C */
{
        exit(estat);
}

/*****************************************************************************

        ZFree()

        This function frees memory previously allocated by the ZAlloc
function.

*****************************************************************************/

VVOID ZFree(voidptr OldBlk)     /* free memory allocated by ZAlloc */
{
#if DEBUGGING
        static char *DbgFNm = "ZFree";
        sprintf(DbgSBf,"OldBlk = %ld", Zcp2ul(OldBlk));
        DbgFEn(4,DbgFNm,DbgSBf);

/*
 * is the heap corrupted?
 */

#if defined(__TURBOC__) && (__TURBOC__ >= 0x0295)
        if (heapcheck () < 0) {
                puts ("ZFree: heapcheck failed");
                exit (1);
        }
#endif

/*
 * don't allow them to free a NULL pointer
 */

        if (OldBlk == NULL) {
                puts ("ZFree: trying to free NULL pointer");
                exit (1);
        } else {
/*
 * find malloc'ed block in MPtrs[] and free it
 */
                int i;
                for (i = 0; i < MMAX && MPtrs[i] != OldBlk; ++i) {
                        ;
                }
                if (i == MMAX) {
                        puts ("Zfree: trying to free non-malloc'ed pointer");
                        exit (1);
                }
                MPtrs[i] = NULL;
        }
#endif

        farfree ((void *)OldBlk);

        DBGFEX(4,DbgFNm,NULL);
}

/*****************************************************************************

        ZHelp()

        This function accepts a help string and displays the corresponding
help text.

        it should be control-C interrupt-able.

*****************************************************************************/

VVOID ZHelp(                    /* display a help message */
        charptr HlpBeg,         /* first char of help request */
        charptr HlpEnd,         /* last character of help request */
        BOOLEAN SysLib,         /* use default HELP library? */
        BOOLEAN Prompt)         /* enter interactive help mode? */
{
        HlpBeg = HlpBeg;   /* avoid "parameter not used" Turbo C warning */
        HlpEnd = HlpEnd;   /* avoid "parameter not used" Turbo C warning */
        SysLib = SysLib;   /* avoid "parameter not used" Turbo C warning */
        Prompt = Prompt;   /* avoid "parameter not used" Turbo C warning */

        (void)ExeNYI();
}

/*****************************************************************************

        ZIClos()

        This function closes the current input file.  It must

        1.  if current input stream is not open,  simply return
        2.  close the input file
        3.  set open indicator to FALSE

*****************************************************************************/

VVOID ZIClos(DEFAULT IfIndx)            /* close input file */
{
#if DEBUGGING
        static char *DbgFNm = "ZIClos";
        sprintf(DbgSBf,"IfIndx = %d", IfIndx);
        DbgFEn(2,DbgFNm,DbgSBf);
#endif

        if (IsOpnI[IfIndx]) {                   /* if it's open */
                if (fclose(IFiles[IfIndx]) == EOF) {
                        ZErMsg();
                        ErrMsg(ERR_UCI);
                        DBGFEX(2,DbgFNm,"fclose() failed");
                        exit(EXIT_FAILURE);
                }
                IsOpnI[IfIndx] = FALSE;
        }

        DBGFEX(2,DbgFNm,NULL);
}

/*****************************************************************************

        ZOClDe()

        This function closes and deletes the current output stream.  It must

        1.  if no current output stream is defined,  simply return
        2.  close the output stream
        3.  delete the file just closed

*****************************************************************************/

VVOID ZOClDe(DEFAULT OfIndx)            /* close and delete output file */
{
        DBGFEN(2,"ZOClDe",NULL);

        if (IsOpnO[OfIndx]) {           /* if output stream is open */
                if (fclose(OFiles[OfIndx].OStrem) == EOF) {
                        ZErMsg();
                        ErrStr(ERR_UCO, OFiles[OfIndx].OFNam);
                        DBGFEX(2,DbgFNm,"fclose() failed, calling exit()");
                        exit(EXIT_FAILURE);
                }
                if (remove(OFiles[OfIndx].OFNam) != 0) {
                        ZErMsg();
                        ErrStr(ERR_UCD, OFiles[OfIndx].OFNam);
                        DBGFEX(2,DbgFNm,", remove() failed, calling exit()");
                        exit(EXIT_FAILURE);
                }
                IsOpnO[OfIndx] = FALSE;
        }

        DBGFEX(2,DbgFNm,NULL);
}

/*****************************************************************************

        ZOClos()

        This function closes the current output stream.  It is only called
when an output stream is defined.  It must

        1.  flush output to the stream,  if neccessary
        2.  close the stream
        3.  set OFile to -1

*****************************************************************************/

static DEFAULT fastcopy(char *srcname, char *dstname) {
        /* fastcopy copies from srcname to dstname then deletes the source
           file. It returns a 0 on success. */
    int srchandle, dsthandle;
#define BUFSIZE 4096
    char *buffer;
    struct utimbuf times;
    struct stat statbuf;
    int size;

    if ((buffer = (char *)malloc(BUFSIZE)) == NULL) return 1;

    if ((srchandle = open(srcname, O_RDONLY|O_BINARY)) == -1) {
        free(buffer);
        return 1;
    }
    if ((dsthandle = open(dstname, O_WRONLY|O_CREAT|O_BINARY|O_TRUNC,
                          S_IREAD|S_IWRITE)) == -1) {
        close(srchandle);
        free(buffer);
        return 1;
    }
    while ((size=read(srchandle, buffer, BUFSIZE)) >0) {
        if (write(dsthandle, buffer, size) != size) {
            size = -1; /* signal an error */
            break;
        }
    }
    close(srchandle);
    close(dsthandle);
    free(buffer);
    if (size < 0) {
        remove(dstname);
        return 1;
    }
#if (TC_SMALL_CODE==0)  /* Not enough room to update stamp if medium model */
    stat(srcname, &statbuf);
    times.actime = statbuf.st_atime;
    times.modtime = statbuf.st_mtime;
    utime(dstname, &times);
#endif
    remove(srcname);
    return 0;
}



VVOID ZOClos(DEFAULT OfIndx)    /* close output file */
{
    char        *DotPtr;
    char        *SlaPtr1, *SlaPtr2;
    char        TmpFsp[FILENAME_MAX];

    DBGFEN(2,"ZOClos",NULL);

    if (!IsOpnO[OfIndx]) {                              /* if it's not open */
        DBGFEX(2,DbgFNm,NULL);
        return;                                 /* we're done */
    }

    if (fclose(OFiles[OfIndx].OStrem) == EOF) { /* close it */
        ZErMsg();
        ErrMsg(ERR_UCO);                        /* unable to close */
        DBGFEX(2,DbgFNm,"fclose() failed");
        exit(EXIT_FAILURE);
    }

    if (OFiles[OfIndx].OTNam[0] != '\0') {      /* if temporary output file */

        if (OFiles[OfIndx].forBackup) {
            /* only do backup thing if we were doing EB originally */
            /*
             * build "filename.bak" filespec in TmpFsp[]
             */

            strcpy(TmpFsp, OFiles[OfIndx].OTNam);       /* copy to TmpFsp */
            SlaPtr1 = strrchr(TmpFsp, '/');
            SlaPtr2 = strrchr(TmpFsp, '\\');
            if ((SlaPtr1 == NULL) || 
                (SlaPtr2 != NULL && SlaPtr2 > SlaPtr1))
                SlaPtr1 = SlaPtr2;
            DotPtr = strrchr(TmpFsp, '.');              /* find the "." */
            if (DotPtr != NULL) {                       /* if "." exists */
                *DotPtr = '\0';                 /* make it null */
            }
            strcat(TmpFsp, ".bak");                     /* append ".bak" */

            /*
             * (maybe) delete an old "filename.bak" file
             */

            if (access(TmpFsp, 0) == 0) {               /* old "x.bak"? */
#if DEBUGGING
                sprintf(DbgSBf,"removing old version of %s", TmpFsp);
                DbgFMs(2,DbgFNm,DbgSBf);
#endif
                if (remove(TmpFsp) != 0) {      /* delete it */
                    ZErMsg();
                    ErrMsg(ERR_UCO);
                    DBGFEX(2,DbgFNm,"remove() failed");
                    exit(EXIT_FAILURE);
                }
            }

#if DEBUGGING
            sprintf(DbgSBf,"renaming %s to %s",
                    OFiles[OfIndx].OTNam, TmpFsp);
            DbgFMs(2,DbgFNm,DbgSBf);
#endif

            /*
             * rename "filename" to "filename.bak"
             */

            if (rename(OFiles[OfIndx].OTNam,TmpFsp) &&
                fastcopy(OFiles[OfIndx].OTNam,TmpFsp)) {
                ZErMsg();
                ErrMsg(ERR_UCO);
                DBGFEX(2,DbgFNm,"rename() failed, calling exit()");
                exit(EXIT_FAILURE);
            }

#if DEBUGGING
            sprintf(DbgSBf,"renaming %s to %s",
                    OFiles[OfIndx].OFNam, OFiles[OfIndx].OTNam);
            DbgFMs(2,DbgFNm,DbgSBf);
#endif
        }  /* End backup renaming */
        else { /* delete original if not backing up */
            if (remove(OFiles[OfIndx].OTNam) != 0) {
                ZErMsg();
                ErrMsg(ERR_UCO);
                DBGFEX(2,DbgFNm,"remove() failed");
                exit(EXIT_FAILURE);
            }
        }


        /*
         * rename "tmpnam" to "filename"
         */

        if (rename(OFiles[OfIndx].OFNam,OFiles[OfIndx].OTNam) &&
            fastcopy(OFiles[OfIndx].OFNam,OFiles[OfIndx].OTNam)) {
            ZErMsg();
            ErrMsg(ERR_UCO);
            DBGFEX(2,DbgFNm,"rename() failed, calling exit()");
            exit(EXIT_FAILURE);
        }
    }

    IsOpnO[OfIndx] = FALSE;             /* mark it as closed */

    DBGFEX(2,DbgFNm,NULL);
}

/*****************************************************************************

        ZOpInp()

        This function opens an input file.  The name of the file is pointed
to by FBfBeg.  FBfPtr points to the character following the last character of
the file name.

*****************************************************************************/

DEFAULT ZOpInp(                 /* open input file */
        DEFAULT IfIndx,         /* index into file data block array IFiles */
        BOOLEAN EIFile,         /* is it a macro file (hunt for it) */
        BOOLEAN RepFNF)         /* report "file not found" error? */
{
#if DEBUGGING
        static char *DbgFNm = "ZOpInp";
        sprintf(DbgSBf,"FBf = \"%.*s\"", (int)(FBfPtr-FBfBeg), FBfBeg);
        DbgFEn(2,DbgFNm,DbgSBf);
#endif

        *FBfPtr = '\0';                 /* terminate the file name */

/*
 * If we're executing an EI command,  then the default file type (the part of
 * the filename following the ".") is "tec".  If we're supposed to use the
 * default type,  and if the filename doesn't already have a type,  then
 * append ".tec" to the filename.
 */

        if (EIFile) {
                if (strchr((char *)FBfBeg,'.') == NULL) {
                        strcat((char *)FBfBeg,".tec");
                        FBfPtr += 4;
                }
        }

        if ((IFiles[IfIndx] = fopen((char *)FBfBeg, "rb")) == NULL) {
                if (EIFile) {
                    char fnbuf[FILENAME_MAX];
                    char *envp;
                    envp = getenv("TEC$LIBRARY");
                    if (envp) {
                        strcpy(fnbuf, envp);
                        strcat(fnbuf, (char *)FBfBeg);
                        if ((IFiles[IfIndx] = fopen(fnbuf, "rb"))!= NULL) {
                            return SUCCESS;
                        }
                    }
                }
                if (!RepFNF && ((errno == ENODEV) || (errno == ENOENT))) {
                        DBGFEX(2,DbgFNm,"FILENF");
                        return FILENF;
                }

                ZErMsg();
                DBGFEX(2,DbgFNm,"FAILURE, fopen() failed");
                return FAILURE;
        }

        DBGFEX(2,DbgFNm,"SUCCESS");

        return SUCCESS;
}

/*****************************************************************************

        ZOpOut()

        This function creates (and opens) an output file.  The name of the
file to be created is pointed to by FBfBeg.  FBfPtr points to the character
following the last character of the file name.

        MS-DOS does not have file version numbers like VAX/VMS,  so we have
to deal with creating ".bak" versions of files.  For output files, this means
that when the output file is opened,  we check if a file with the same name
already exists.  If a file already exists,  then we open a temporary output
file and,  when the file is closed,  the ZOClos function will deal with
renaming things to make them come out right.  If no file with the same name
already exists,  then the output file can simply be opened.  The ZOClos
function will only rename files if a temporary file was created by this
funtion.

*****************************************************************************/

DEFAULT ZOpOut(DEFAULT OfIndx, BOOLEAN RepErr, BOOLEAN Backup)
{
#if DEBUGGING
        static char *DbgFNm = "ZOpOut";
        sprintf(DbgSBf,"FBf = \"%.*s\"", (int)(FBfPtr-FBfBeg), FBfBeg);
        DbgFEn(2,DbgFNm,DbgSBf);
#endif

/*
 * If the output file already exists,  make a temporary file.
 */

        *FBfPtr = '\0';
        if (access((char *)FBfBeg, 0) == 0) {   /* if file already exists */
                if (access((char *)FBfBeg, 2) != 0) { /* Can't write it? */
                        if (RepErr) {
                                ZErMsg();
                        }
                        return FAILURE;
                }
                tmpnam(OFiles[OfIndx].OFNam);
                strcpy(OFiles[OfIndx].OTNam, (char *)FBfBeg);
#if DEBUGGING
                sprintf(DbgSBf,"\"%s\" file exists, using tmpnam()", FBfBeg);
                DbgFMs(2,DbgFNm,DbgSBf);
#endif
        } else {
                strcpy(OFiles[OfIndx].OFNam, (char *)FBfBeg);
                OFiles[OfIndx].OTNam[0] = '\0';
        }

#if DEBUGGING
        sprintf(DbgSBf,"creating file \"%s\"", OFiles[OfIndx].OFNam);
        DbgFMs(2,DbgFNm,DbgSBf);
#endif

        OFiles[OfIndx].OStrem = fopen(OFiles[OfIndx].OFNam, "wb");
        if (OFiles[OfIndx].OStrem == NULL) {
                if (RepErr) {
                        ZErMsg();
                }
#if DEBUGGING
                sprintf(DbgSBf,"FAILURE, fopen(\"%s\",\"w\") failed",
                        OFiles[OfIndx].OFNam);
                DbgFEx(2,DbgFNm,DbgSBf);
#endif
                return FAILURE;
        }

        DBGFEX(2,DbgFNm,"SUCCESS");

        OFiles[OfIndx].forBackup = Backup;

		if (OFiles[OfIndx].OTNam[0] != '\0' && RepErr && !Backup) {
			ZDspBf("% Superseding existing file\r\n", 29);
		}
        return SUCCESS;
}

/*****************************************************************************

        ZPrsCL()

        Parse the command line using a TECO macro.

        load q-register Z with the command line
        if USE_ANSI_CLPARS
                directly execute command-line parsing macro in clpars[]
        else
                load q-register Y with a command-line parsing macro
                do an MY$$

*****************************************************************************/

VVOID ZPrsCL(int argc, char **argv)
{
        int     i;
        char    TmpBuf[256];
        SIZE_T  line_len;

        DBGFEN(2,"ZPrsCL",NULL);

/*
 * If the command line contains arguments,  construct a replica of the
 * command line in Q-register Z.  It's a "replica" because spacing might
 * be wrong.
 */

        if (argc > 1) {
                TmpBuf[0] = '\0';
                for (i = 1; i < argc; i++) {
#ifdef WIN32
					if ( strchr(*++argv, ' ') != NULL ) {
						// There is embedded space, turn into quoted string
						strcat(TmpBuf, "\"");
						strcat(TmpBuf, *argv);
						strcat(TmpBuf, "\" ");
					}
					else {
						strcat(TmpBuf, *argv);
						strcat(TmpBuf, " ");
					}
#else
                        strcat(TmpBuf, *++argv);
                        strcat(TmpBuf, " ");
#endif
                }
                line_len = strlen(TmpBuf)-1;    /* ignore trailing space */
                QR = &QRgstr[35];               /* 35 = q-register Z */
                if (MakRom(line_len) == FAILURE) {
                        DBGFEX(2,DbgFNm,
                               "couldn't make room, exiting EXIT_FAILURE");
                        exit(EXIT_FAILURE);
                }
                MEMMOVE(QR->Start, TmpBuf, line_len);
                QR->End_P1 += line_len;         /* length of q-reg text */
        }

#if USE_ANSI_CLPARS

/*
 * execute embedded command line-parsing macro directly from clpars[]
 */

#ifdef WIN32
		CStBeg = CBfPtr = (unsigned char  *)clpars;/* command string start */
		CStEnd = (unsigned char *)clpars + CLPARS_LEN;           /* command string end */
#else
        CStBeg = CBfPtr = (unsigned char _far *)clpars;/* command string start */
        CStEnd = (unsigned char _far *)clpars + CLPARS_LEN;           /* command string end */
#endif
        EStTop = EStBot;                        /* clear expression stack */
        ExeCSt();                               /* execute command string */

#else

/*
 * Load imbedded command-line parsing macro into Q-register Y
 */

        QR = &QRgstr[34];                       /* 34 = q-register Y */
        if (MakRom((SIZE_T)CLPARS_LEN) == FAILURE) {
                DBGFEX(2,DbgFNm,"MakRom(CLPARS_LEN) failed, calling exit()");
                exit(EXIT_FAILURE);
        }
        for (i = 0; i < CLPARS_LINES; i++) {
                line_len = strlen(clpars[i]);
                MEMMOVE(QR->End_P1, clpars[i], line_len);
                QR->End_P1 += line_len;         /* length of q-reg text */
        }

/*
 * Execute an MY$$ command.
 */

        CBfPtr = "my\33\33";                    /* command string start */
        CStEnd = CBfPtr + 3;                    /* command string end */
        EStTop = EStBot;                        /* clear expression stack */
        ExeCSt();                               /* execute command string */

/*
 * Clear the command-line parsing macro from Q-register Y
 */

        QR = &QRgstr[34];                       /* 34 = q-register Y */
        ZFree (QR->Start);
        QR->Start = QR->End_P1 = NULL;

#endif

        DBGFEX(2,DbgFNm,NULL);
}

/*****************************************************************************

        ZPWild()

        This function presets the wildcard lookup filename.  It is
called when the user executes an ENfilename$ command.  Later executions of
the EN$ command will cause the ZSWild function to be called to return
successive wildcard matches.

        Under Turbo C, we save the drive code and subdirectory path so
later on we can return full filenames.

*****************************************************************************/

DEFAULT ZPWild(void)            /* preset the wildcard lookup filename */
{
        DBGFEN(2,"ZPWild",NULL);
/*
 * say wildcard lookup has been preset, but we haven't done any
 * findfirst or findnext'ing yet.
 */

        ff_preset = 0;

/*
 * if they didn't process all of the filenames from a
 * previous EN' call, discard them
 */

        if (ff_names != NULL) {
                ZFree (ff_names);
                ff_names = NULL;
        }
        ff_names_p = NULL;

/*
 * "clean out" ff_drive, ff_dir, and ff_path.
 */
        memset (ff_drive, 0, MAXDRIVE);
        memset (ff_dir,   0, MAXDIR);
        memset (ff_path,  0, MAXPATH);

/*
 * NULL-terminate wildcard filename in FBf so fnsplit knows
 * where it ends, copy it into ff_path[], and save drive code
 * and directory path in ffdrive[] and ff_path[].
 */

        *FBfPtr = '\0';
        strncpy (ff_path, (char *)FBfBeg, MAXPATH-1);
        strlwr (ff_path);
        fnsplit (ff_path, ff_drive, ff_dir, NULL, NULL);

        DBGFEX(2,DbgFNm,"SUCCESS");

        return SUCCESS;

}

/*****************************************************************************

        ZRaloc()

        This function performs the standard C library function realloc.

*****************************************************************************/

voidptr ZRaloc(voidptr OldBlk, SIZE_T NewSiz)
{
        voidptr NewBlk;

#if DEBUGGING
        static char *DbgFNm = "ZRaloc";
        sprintf(DbgSBf,"OldBlk = %ld, NewSiz = %ld",
                Zcp2ul(OldBlk),(LONG)NewSiz);
        DbgFEn(4,DbgFNm,DbgSBf);
#endif

/*
 *      The Turbo C v2.0 farrealloc() calls movedata() to move the data
 * from the old block to the new block.  Unfortunately, movedata() takes an
 * unsigned int for the number of bytes to move; so, if the old buffer was
 * greater than 64K bytes long, the old data wasn't correctly moved to the
 * new block.  This version of ZRaloc simulates what farrealloc() does, using
 * Teco-C's MEMMOVE() to move the data.
 *
 * TC++ v1.0 has __TURBOC__ as 0x0295
 */

#if defined(__TURBOC__) && (__TURBOC__ < 0x0295) && !TC_SMALL_DATA

/*
 * the far memory allocation functions deal with a structure which
 * look something like:
 *
 *      struct _blk {
 *              unsigned long   size;
 *              unsigned long   something;
 *              char            block[size];
 *      }
 *
 * size:        the size of the allocated block in bytes, this might
 *              be rounded to the next paragraph (16-byte) boundary
 * something:   ? pointer to next block on free list when free'ed ?
 * block:       a region "size" bytes long.  a pointer to this
 *              region is what the memory allocation functions
 *              actually return to the user.
 *
 * Thus, to get to the size of the old block, we simply subtract
 * 8 (2 * sizeof(unsigned long)) from the pointer we have, treat
 * that like an unsigned long pointer, and look at what's there.
 */

/*
 * manually allocate a new block
 */

        NewBlk = ZAlloc (NewSiz);

/*
 * if there was old data and the malloc was successful,
 * move the data from the old block to the new block and
 * free the old block.
 */

        if (OldBlk != NULL && NewBlk != NULL) {
                SIZE_T OldSiz;
                OldSiz = *((unsigned long far *)((charptr)OldBlk - 8));
                OldSiz += -9L;  /* TC's farrealloc() does this (?) */
                MEMMOVE(NewBlk, OldBlk, (NewSiz > OldSiz) ? OldSiz : NewSiz);
                ZFree (OldBlk);
        }

#else

        NewBlk = farrealloc ((void *)OldBlk, NewSiz);

#if DEBUGGING
/*
 * update OldBlk's entry in MPtrs[]
 */

        if (NewBlk != NULL && OldBlk != NewBlk) {
            int i;
            for (i = 0; i < MMAX && MPtrs[i] != OldBlk; ++i) {
                        ;
            }
            if (i == MMAX) {
                puts ("ZRaloc: trying to realloc non-malloc'ed pointer");
                exit (1);
            }
            MPtrs[i] = NewBlk;
        }
#endif
#endif

#if DEBUGGING
/*
 * is the heap corrupted?
 */

#if defined(__TURBOC__) && (__TURBOC__ >= 0x0295)
        if (heapcheck () < 0) {
                puts ("ZRaloc: heapcheck failed");
                exit (1);
        }
#endif


        sprintf(DbgSBf,"NewBlk = %ld", Zcp2ul(NewBlk));
        DbgFEx(4,DbgFNm,DbgSBf);
#endif

        return NewBlk;
}

/*****************************************************************************

        ZRdLin()

        This function reads a line from a file.  It is passed a buffer, the
size of the buffer, a file pointer and a pointer to a place to leave the
length of the line.

*****************************************************************************/

DEFAULT ZRdLin(                 /* read a line from a file */
        charptr ibuf,           /* where to put string */
        ptrdiff_t ibuflen,      /* max length of ibuf */
        int IfIndx,             /* index into IFiles[] */
        DEFAULT *retlen)        /* returned length of string */
{
        int character;          /* the last character read */
        DEFAULT shortBuf;       /* max size to read, 32767 or ibuflen */
        DEFAULT charsLeft;      /* number of characters left */
        FILE *fp;               /* input stream pointer to read from */
        char *iBuf;             /* non-huge pointer into IBf for speed */

#if DEBUGGING
        static char *DbgFNm = "ZRdLin";
        sprintf(DbgSBf,"ibuf = %ld, ibuflen = %ld, IfIndx = %d",
                Zcp2ul(ibuf),(LONG)ibuflen,IfIndx);
        DbgFEn(3,DbgFNm,DbgSBf);
#endif

        shortBuf = (ibuflen > 32767) ? 32767 : (DEFAULT) ibuflen;
        charsLeft = shortBuf;
        fp = IFiles[IfIndx];
        iBuf = (char *) ibuf;

        while ((character = getc(fp)) >= 0) {    /* we got one */
                *iBuf++ = character;
                if (character == LINEFD || character == VRTTAB) {
                        /* finished with this line */
                        *retlen = shortBuf - charsLeft + 1;
#if DEBUGGING
                        sprintf(DbgSBf,"SUCCESS, retlen = %d", *retlen);
                        DbgFEx(3,DbgFNm,DbgSBf);
#endif
                        return SUCCESS;
                }
                if (character == FORMFD && !(EzFlag & ED_FF)) {
                        /* toss form feed, then finished */
                        *retlen = shortBuf - charsLeft;
                        FFPage = -1;
#if DEBUGGING
                        sprintf(DbgSBf,"SUCCESS, retlen = %d", *retlen);
                        DbgFEx(3,DbgFNm,DbgSBf);
#endif
                        return SUCCESS;
                }
                if (--charsLeft == 0) {
                        /* no more room, so return */
                        *retlen = shortBuf;
#if DEBUGGING
                        sprintf(DbgSBf,"SUCCESS, retlen = %d", *retlen);
                        DbgFEx(3,DbgFNm,DbgSBf);
#endif
                        return SUCCESS;
                }
        }

/*
 * If we made it to here, the read has failed --- EOF or Error.
 */
        if (ferror(fp)) {                       /* if we got an error */
                *retlen = 0;                    /* say didn't read anything */
                ZErMsg();
#if DEBUGGING
                sprintf(DbgSBf,"ferror() FAILURE");
                DbgFEx(3,DbgFNm,DbgSBf);
#endif
                return FAILURE;
        }

/*
 * If we made it to here, the read has failed because of EOF.
 */
        if ((*retlen = shortBuf-charsLeft) == 0) {
                IsEofI[IfIndx] = TRUE;          /* say we reached EOF */
        }
#if DEBUGGING
        sprintf(DbgSBf,"SUCCESS, retlen = %d", *retlen);
        DbgFEx(3,DbgFNm,DbgSBf);
#endif
        return SUCCESS;
}

/*****************************************************************************

        ZScrOp()

        This function is called to perform special screen functions.

*****************************************************************************/

VVOID ZScrOp(DEFAULT OpCode)    /* do a screen operation */
{
#if VIDEO
        int index;
        static int map[] = {
                1,      /* 0 - VT52 is a VT52 */
                2,      /* 1 - VT61 is a VT61 */
                1,      /* 2 - VT100 in VT52 mode is a VT52 */
                0,      /* 3 - unused */
                3,      /* 4 - VT100 in ANSI mode is a VT100 */
                0,      /* 5 - unused */
                0,      /* 6 - VT05 is a VT05 */
                0,      /* 7 - unused */
                3,      /* 8 - VT102 is a VT100 */
                0,      /* 9 - unused */
                3,      /* 10 - VK100 is a VT100 */
                3,      /* 11 - VT200 in VT200 mode is a VT100 */
                3,      /* 12 - VT200 in VT100 mode is a VT100 */
                1,      /* 13 - VT200 in VT52 mode is a VT52 */
        };
        struct strng
                {
                charptr strt;
                DEFAULT len;
                };
        static struct strng CUP[] = {           /* cursor up one line */
                {"\232\0\0\0\0",        5},     /* VT05 - ? */
                {"\033A",               2},     /* VT52 - ESC A */
                {"",                    0},     /* VT61 */
                {"\033[A",              3}      /* VT100 - ESC [ A */
        };
        static struct strng EEL[] = {           /* erase to end of line */
                {"\36",                 1},     /* VT05 - RS */
                {"\033K\r",             3},     /* VT52 - ESC K CR */
                {"",                    0},     /* VT61 */
                {"\033[K",              3}      /* VT100 - ESC [ K */
        };
        static struct strng ROF[] = {           /* reverse video on */
                {"",                    0},     /* VT05 */
                {"",                    0},     /* VT52 */
                {"",                    0},     /* VT61 */
                {"\033[m",              3}      /* VT100 - ESC [ m */
        };
        static struct strng RON[] = {           /* reverse video off */
                {"",                    0},     /* VT05 */
                {"",                    0},     /* VT52 */
                {"",                    0},     /* VT61 */
                {"\033[7m",             4}      /* VT100 - ESC [ 7 m */
        };

        if (CrType == UNTERM) {         /* if unknown terminal type */
                return;                 /* can't do screen operations */
        }

        if (CrType == IBMPC) {
#endif
                switch (OpCode) {
                case SCR_CUP:   /* cursor up one line */
                        gotoxy(wherex(), wherey()-1);
                        break;

                case SCR_EEL:   /* erase to end-of-line */
                        {
                        int x = wherex();
                        char spaces[80];
                        memset(spaces, ' ', 80);
                        ZDspBf(spaces, 80 - x);
                        gotoxy(x,wherey());
                        }
                        break;

                case SCR_ROF:   /* turn on reverse video */
                case SCR_RON:   /* turn off reverse video */
                        break;
                }
                return;
#if VIDEO
        }

/*
 * The numbering used for CrType comes from TECO-11.  Convert it to get an
 * index into the string arrays.
 */
        index = map[CrType];

        switch (OpCode) {
            case SCR_CUP: ZDspBf(CUP[index].strt, CUP[index].len); break;
            case SCR_EEL: ZDspBf(EEL[index].strt, EEL[index].len); break;
            case SCR_ROF: ZDspBf(ROF[index].strt, ROF[index].len); break;
            case SCR_RON: ZDspBf(RON[index].strt, RON[index].len); break;
        }
#endif
}

/*****************************************************************************

        ZSetTT()

        This function sets or clears terminal parameters.  The only terminal
parameters that TECO can set are

        1. whether the terminal can display 8-bit characters
        2. the number of rows
        3. the number of columns

*****************************************************************************/

DEFAULT ZSetTT(         /* tell operating system that we set the terminal */
DEFAULT TTWhat,         /* what terminal parameter to set */
DEFAULT TTVal)          /* what to set it to */
{
        TTWhat = TTWhat;   /* avoid "parameter not used" Turbo C warning */
        TTVal = TTVal;     /* avoid "parameter not used" Turbo C warning */
        return ExeNYI();
}

/*****************************************************************************

        ZSWild()

        This function searches for the next wildcard filename.  It
is called when the user executes an "EN$" or ":EN$" command.  If the user
executes an "ENfilename$" command,  the ZPWild function is called,  not this
function.

        This function returns

                1. SUCCESS if the filename buffer has a new file name
                2. FAILURE if the search failed somehow other than FILENF
                3. FILENF if no more occurrences of the wildcard exist

*****************************************************************************/

DEFAULT ZSWild(void)            /* search for next wildcard filename */
{
        charptr         cp;
        struct ffblk    ff_blk;                 /* wildcard control block */
        const size_t    ff_exp = (MAXPATH*20);  /* expansion increment */
        size_t          ff_max;                 /* max_size of ff_names */
        size_t          ff_size;                /* current size of ff_names */
        int             ff_status;
        charptr         ffp;
        size_t          filename_length;

        DBGFEN(2,"ZSWild",NULL);

        switch (ff_preset) {
        case -1:
/*
 * if ZPWild hasn't been called, return FILENF (or FAILURE ???)
 */
                DBGFEX(2,DbgFNm,"FILENF, ENfilename$ not called");
                return FILENF;

        case 0:
/*
 * if ZPWild has been called, but we haven't done any directory searching
 * yet, search the directory and build a list of filenames in ff_names
 * like "filespec\0filespec\0...filespec\0\0"
 */
                ff_preset = 1;
                ffp = ff_names = NULL;
                ff_size = ff_max = 0;           /* force initial ZRaloc */
                ff_status = findfirst (ff_path, &ff_blk, 0);
                while (ff_status == 0) {

/*
 * allocate if ff_names is not big enough to hold a full filename
 */
                        if (ff_max - ff_size < MAXPATH) {
                                ff_max += ff_exp;
                                ffp = (charptr) ZRaloc (ff_names, ff_max);
                                if (ffp == NULL) {
                                    if (ff_names != NULL) {
                                        ZFree (ff_names);
                                        ff_names = NULL;
                                    }
                                    ff_names_p = NULL;
                                    ff_preset = -1;
                                    DBGFEX(2,DbgFNm,"FAILURE, ZRaloc failed");
                                    return FAILURE;
                                }
                                ff_names = ffp;
                                ffp = &ff_names[ff_size];
                        }
/*
 * copy drive, subdirectory path, and filename
 * to ff_names, then NULL-terminate filename
 */
                        for (cp = ff_drive; *cp; ++ff_size) {
                                *ffp++ = *cp++;
                        }
                        for (cp = ff_dir; *cp; ++ff_size) {
                                *ffp++ = *cp++;
                        }
                        for (cp = ff_blk.ff_name; *cp; ++ff_size, ++cp) {
                                *ffp++ = tolower (*cp);
                        }
                        *ffp++ = '\0';
                        ++ff_size;

/*
 * find next match
 */
                        ff_status = findnext (&ff_blk);
                }

/*
 * NULL-terminate filename list
 */

                if (ffp) {
                        *ffp = '\0';
                }

                ff_names_p = ff_names;

/*
 * fall through to return first name in ff_names list
 */

        default:
/*
 * if no more names in ff_names, free what we found
 */

                if (ff_names_p != NULL && *ff_names_p == '\0')  {
                        ZFree (ff_names);
                        ff_names = ff_names_p = NULL;
                }

                if (ff_names_p == NULL) {
                        DBGFEX(2,DbgFNm,"FILENF, no more names in ff_names");
                        return FILENF;
                }

/*
 * copy next name in ff_names to FBf and make
 * ff_names_p point to next name in ff_names
 */

                filename_length = strlen ((char *)ff_names_p);
                MEMMOVE (FBfBeg, ff_names_p, filename_length);
                FBfPtr = FBfBeg + filename_length;
                ff_names_p += (filename_length + 1);
        }

        DBGFEX(2,DbgFNm,"SUCCESS");

        return SUCCESS;
}
/*****************************************************************************

        ZTrmnl()

        This function sets up the input/output of commands.  Usually, that
means the input/output channels to the terminal,  but TECOC might be run
from a BATCH file and that possibility must be handled.  In addition,  the
handling of interrupts is found here.

        In general,  this function must:

                1. Set TIChan so it can be used to read commands
                2. Set TOChan so it can be used for output
                3. handle interrupts
                4. initialize CrType (what kind of terminal it is)
                5. initialize EtFlag (terminal capability bits)
                6. initialize HtSize (number columns terminal has)
                7. initialize VtSize (number rows terminal has)

*****************************************************************************/

static int _Cdecl CntrlC(void)                  /* see ZPORT.H for _Cdecl */
{
        if (EtFlag & ET_TRAP_CTRL_C) {          /* if user wants it */
                EtFlag &= ~ET_TRAP_CTRL_C;      /* turn off bit */
        } else {
                if (EtFlag & ET_MUNG_MODE) {    /* if in MUNG mode */
                        TAbort(EXIT_SUCCESS);
                }
                GotCtC = TRUE;                  /* set "stop soon" flag */
        }
        return 1;
}

VVOID ZTrmnl(void)                      /* set up I/O to the terminal */
{
        DBGFEN(2,"ZTrmnl",NULL);

/*
 * To prevent line feeds written to the terminal from being converted to
 * carriage-return/line-feeds,  reset stdout to binary mode.  This is enough
 * to fix the problem in Microsoft C,  but Turbo-C requires that the FILE
 * struct member "flags" also be twiddled with.
 */
        setmode(fileno(stdout), O_BINARY);
#if defined(__TURBOC__)
        stdout->flags |= _F_BIN;
#endif

        EtFlag = ET_READ_LOWER |        /* don't convert lower to upper */
                 ET_BKSP_IS_DEL |       /* backspace key is delete key */
                 ET_SCOPE |             /* ok to use video for del and ^U */
                 ET_EIGHTBIT;           /* terminal uses 8-bit characters */
#ifndef WIN32
        ctrlbrk(CntrlC);
#endif
        CrType = IBMPC;                 /* use ROM BIOS routines */

#if VIDEO
        VtSize = HtSize = 0;
#endif

        DBGFEX(2,DbgFNm,NULL);
}

/*****************************************************************************

        ZVrbos()

        This function displays the verbose form of an error message.

*****************************************************************************/

VVOID ZVrbos(WORD ErrNum, char *ErMnem)
{
        char **TmpPtr;

#include "vrbmsg.h"

        ErrNum = ErrNum;   /* avoid "parameter not used" Turbo C warning */
        ErMnem = ErMnem;   /* avoid "parameter not used" Turbo C warning */
        ZDspBf("\r\n",2);
        for (TmpPtr = &ParaTx[StartP[LstErr]]; *TmpPtr; ++TmpPtr) {
                ZDspBf((charptr)*TmpPtr, strlen (*TmpPtr));
                ZDspBf("\r\n",2);
        }
}

/*****************************************************************************

        ZWrBfr()

        This function writes a buffer to a file.  It is passed an output
file index and pointers to the beginning and end (plus 1) of the buffer
to be output.

*****************************************************************************/

DEFAULT ZWrBfr(
           DEFAULT OfIndx,      /* index into OFiles array */
           charptr BfrBeg,      /* address of output buffer beginning */
           charptr BfrEnd)      /* address of output buffer end */
{
        ptrdiff_t bufsiz = BfrEnd - BfrBeg + 1;

#if DEBUGGING
        static char *DbgFNm = "ZWrBfr";
        sprintf(DbgSBf,"OfIndx = %d, BfrBeg = %ld, BfrEnd = %ld",
                OfIndx, Zcp2ul(BfrBeg), Zcp2ul(BfrEnd));
        DbgFEn(2,DbgFNm,DbgSBf);
#endif

        if (Zfwrite(BfrBeg,bufsiz,OFiles[OfIndx].OStrem) != bufsiz) {
                ZErMsg();
                ErrMsg(ERR_UWL);
                DBGFEX(2,DbgFNm,"Zfwrite() failed");
                return FAILURE;
        }

        DBGFEX(2,DbgFNm,"SUCCESS");

        return SUCCESS;
}

#if CHECKSUM_CODE                       /* MS-DOS only debugging aid */

/*****************************************************************************

        While debugging on the PC, we ran into some bizzare behavior which led
us to think code was being overwritten somewhere.  The following routines
calculate checksums for each module in TECO-C.  These checksums can be checked
during runtime to see if the code has been changed.  If a checksum has
changed, the offending module is displayed and TECOC-C is exited.

        This code only works under Turbo-C.  This code doesn't work under
Turbo Debugger since TD modifies code while it runs.

        After init_code_checksums() is called in main() above,
check_code_checksums() is called in ExeCSt() after the successful completion
of each command.  For more frequent checking, it can also be called in the
DbgDMs() routine above but then the "DEBUGGING" macro in ZPORT.H has to be
set to "TRUE" as well.

        If the "CHECKSUM_CODE" macro in ZPORT.H is set to "FALSE", then none
of this code is compiled.

*****************************************************************************/

#pragma inline                  /* use Turbo Asm version of calc_checksum() */

/*
 * As of 1-Jun-1990, TECO-C contains about 230 modules so I've hardwired
 * module_checksums[] to contain 300 entries, plus a final empty entry which
 * signifies the end of the module list.  some space is wasted, but
 * hardwiring it is simpler. Manfred Siemsen.
 */

#define MAX_MODULES             200

/*
 * The large model module names in the map file are in the form "NAME_TEXT"
 * and it turns out that "NAME" is never more than 8 characters. Since I
 * strip off the "_TEXT", I've made the maximum length of the module name 8
 * characters and left space for the trailing '\0'.  This might be a bad
 * assumption.  Some space is wasted but again hardwiring it is simpler.
 */

#define MAX_MODULE_NAME_LEN     8       /* is 8 enough? */

/*
 * The chksum struct contains the name, length, and checksum of each module
 * in TECO-C.
 */

 struct chksum {
        char            name[MAX_MODULE_NAME_LEN+1];
        unsigned        length;
        unsigned        checksum;
} module_checksums[MAX_MODULES+1];

/*
 * ZFirst() is an empty function who's object file is the first object file
 * named on the TLINK command line (even before the C0? startup code).  The
 * reason for this is so we know where the first function in the executable
 * is.  I tried using "main()" as the first function in the TECOC module,
 * but that meant I couldn't checksum a few functions which are included
 * along with the startup code in the "_TEXT" module, and I thought it was
 * better to check everything.
 */

extern void ZFirst (void);

/*
 * calc_checksum (cp, length)
 *
 * this routine calculates and returns a simple checksum for a block of
 * memory starting at "cp" that is "length" bytes long.
 *
 * the assembler version is much quicker and seems to produce the same
 * result as the C version.  we'll assume a module is <64K.
 */

unsigned calc_checksum (charptr cp, unsigned length)
{
#if 1   /* ASM version */

        unsigned off;
        unsigned seg;

#if TC_SMALL_CODE
        off = (unsigned) cp;
        seg = _CS;
#else
        off = FP_OFF (cp);
        seg = FP_SEG (cp);
#endif

        off = (unsigned) cp;                            /* dos.h: FP_OFF() */
        seg = (unsigned) (((unsigned long) cp) >> 16);  /* dos.h: FP_SEG() */

        asm     push ds                 /* save DS */
        asm     push si                 /* save SI */

        asm     xor ax,ax               /* AX = checksum = 0 */
        asm     mov ds,seg              /* DS = module segment */
        asm     mov si,off              /* SI = module offset */
        asm     mov cx,length           /* CX = module length */
        asm     xor bh,bh               /* BH is always zero for add */
        asm     inc cx                  /* prepare for 1st decrement */
        asm     jmp short loop_test     /* jump directly to loop test */

cks_loop:                               /* calculate checksum loop */
        asm     mov bl,[si]             /* BL = character at DS:SI */
        asm     add ax,bx               /* checksum += character */
        asm     inc si                  /* ++(module offset) */
loop_test:
        asm     loop cks_loop           /* dec CX and loop if not zero */

        asm     pop si                  /* restore SI */
        asm     pop ds                  /* restore DS */

        return _AX;                     /* is this redundant? */

#else   /* C version */

        unsigned checksum;

        for (checksum = 0; length != 0; ++cp, --length) {
                checksum += *cp;
        }
        return checksum;

#endif
}

/*
 * init_code_checksums ()
 *
 * this routine reads a Turbo C TLINK version 2.0 generated map file for
 * the names and lengths of the various TECO-C modules.  once the map
 * file has been read, checksums are calculated for each module.
 *
 * this routine assumes the map file is in perfect shape.  the format
 * of the map file is:
 *
 *      ""
 *      " Start  Stop   Length Name               Class"
 *      ""
 *      " 00000H 0000BH 0000CH FRSTFUNC_TEXT      CODE"
 *      " 0000CH 00A51H 00A46H _TEXT              CODE"
 *      " 00A52H 01AFDH 010ACH TECOC_TEXT         CODE"
 *      " 01AFEH 026F8H 00BFBH BAKSRC_TEXT        CODE"
 *      " 026F9H 02E0CH 00714H BLDSTR_TEXT        CODE"
 *
 *      etc, etc (one line for each code module) ...
 *
 *      " 16D83H 16E23H 000A1H VRAM_TEXT          CODE"
 *      " 16E24H 16E59H 00036H WHEREXY_TEXT       CODE"
 *      " 16E5AH 1E699H 07840H _DATA              DATA"
 *      " 1E69AH 1E69DH 00004H _EMUSEG            DATA"
 *
 *      etc, etc (the rest is ignored) ...
 */

#define MAX_MAP_LINE_LEN        128     /* 128 should be enough (???) */
#define NAME_COL                22      /* "Name" column */
#define CLASS_COL               41      /* "Class" column */

void init_code_checksums (void)
{
        char            buf[MAX_MAP_LINE_LEN];
        charptr         cp;
        FILE            *fp;
        int             i;
        struct chksum   *mcp;
        long            start;
        long            stop;
        char            temp_name[CLASS_COL-NAME_COL+1];

        memset (buf, 0, MAX_MAP_LINE_LEN);
        memset (temp_name, 0, CLASS_COL-NAME_COL+1);

/*
 * open map file
 */

        if ((fp = fopen ("tecoc.map","r")) == NULL) {
                printf ("init_code_checksums: can't open tecoc.map file\n");
                exit (EXIT_FAILURE);
        }

/*
 * skip the first three header lines
 */

        for (i = 0; i < 3; ++i) {
            if (fgets (buf, MAX_MAP_LINE_LEN, fp) == NULL) {
                printf ("init_code_checksums: premature eof on map file\n");
                exit (EXIT_FAILURE);
            }
        }

/*
 * read the map file until we reach a line with "DATA" in the "Class" column
 * instead of "CODE".
 */

        mcp = module_checksums;
        while (fgets (buf, MAX_MAP_LINE_LEN, fp) && buf[CLASS_COL] == 'C') {
                if (sscanf (buf, " %5lxH %5lxH %5xH %s",
                                &start,
                                &stop,
                                &mcp->length,
                                temp_name) != 4) {
                        printf ("init_code_checksums: sscanf() failed\n");
                        exit (EXIT_FAILURE);
                }

/*
 * zap "_TEXT" from the end of the "NAME_TEXT" module name and copy it to
 * mcp->name.
 *
 * we start strstr() at &temp_name[1] because we don't want to zap "_TEXT"
 * off the module named "_TEXT".
 */

                if ((cp = strstr (&temp_name[1], "_TEXT")) != NULL) {
                        *cp = '\0';
                }
                if (strlen (temp_name) > MAX_MODULE_NAME_LEN) {
                        printf("init_code_checksums: module name");
                        printf(" \"%s\" > MAX_MODULE_NAME_LEN\n", temp_name);
                        exit (EXIT_FAILURE);
                }
                strcpy (mcp->name, temp_name);

/*
 * go to next entry in module_checksums[]
 */

                if (++mcp == &module_checksums[MAX_MODULES-1]) {
                        printf ("init_code_checksums: more code modules ");
                        printf("than MAX_MODULES (%d)\n", MAX_MODULES);
                        exit (EXIT_FAILURE);
                }

/*
 * make sure the while() test fails if the next line we read is less than
 * CLASS_COL characters long.
 */

                buf[CLASS_COL] = '\0';
        }
        fclose(fp);

/*
 * now, calculate checksums for all the modules that have been read in from
 * the map file
 */

        cp = (charptr) ZFirst;
        mcp = &module_checksums[1];
        while (mcp->name[0] != '\0') {
                mcp->checksum = calc_checksum (cp, mcp->length);
                cp += mcp->length;
                ++mcp;
        }
}

/*
 * check_code_checksums ()
 *
 * this routine calculates a current checksum for each module and
 * compares it with the module's initial checksum.  if they are
 * different then something has probably modified the code in that
 * module.
 */

void check_code_checksums (void)
{
        struct chksum   *mcp;
        charptr         cp;
        BOOLEAN         chksum_failed;

/*
 * '\373' is the IBM PC square-root "check" character
 *
 * fputc ('\373', stdout);
 */

        chksum_failed = FALSE;
        cp = (charptr) ZFirst;
        mcp = module_checksums;
        while (mcp->name[0] != '\0') {
                if (mcp->checksum != calc_checksum (cp, mcp->length)) {
                        printf ("\n%s checksum has changed!\n", mcp->name);
                        chksum_failed = TRUE;
                }
                cp += mcp->length;
                ++mcp;
        }

        if (chksum_failed) {
                exit (EXIT_FAILURE);
        }
}

#endif  /* #if CHECKSUM_CODE */
