/*****************************************************************************
        ZOS2.c
        System dependent code for OS/2

*****************************************************************************/

#include "zport.h"              /* define portability identifiers */
#include "tecoc.h"              /* define general identifiers */
#include "defext.h"             /* define external global variables */
#include "chmacs.h"             /* define character processing macros */
#include "clpars.h"             /* command-line parsing macro */
#include "deferr.h"             /* define identifiers for error messages */
#include "dchars.h"             /* define identifiers for characters */
#include "dscren.h"             /* define identifiers for screen i/o */

#include <stdio.h>
#include <io.h>
#include <malloc.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/termio.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <sys/utime.h>

struct termio oldstate, newstate;       /* saved and current terminal states */

/*****************************************************************************
        The following static data is used in ZPWild and ZSWild to
perform wildcard filename lookups under OS/2.

*****************************************************************************/
static  char **ff_names = NULL;        /* filenames found */
static  char **ff_names_p = NULL;      /* pointer into ff_names */

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

        This function displays error message from the operating system on
the terminal screen.  The error message text is retrieved from the operating
system and imbedded in a TECO-style message with the SYS mnemonic.

*****************************************************************************/

static void ZErMsg(void)
{
        char *Errmsg;
        char *cp;

        if ((Errmsg = strerror(errno)) != NULL) {
                cp = Errmsg + strlen(Errmsg) - 1;
                if (*cp == '\n') {
                        *cp = '\0';
                }
        } else {
                Errmsg = "???";
        }
        ErrStr(ERR_SYS, Errmsg);
}

/*****************************************************************************

        ZAlloc()

        This function allocates memory.  The single argument is the number of
bytes to allocate.  TECO-C uses the ZFree and ZRaloc functions to de-allocate
and re-allocate, respectively,  the memory allocated by this function.

*****************************************************************************/

voidptr ZAlloc(MemSize)         /* allocate memory */
SIZE_T MemSize;
{
        return malloc(MemSize);
}

/*****************************************************************************

        ZBell()

        Thus function rings the terminal bell.  For most platforms,  this
means just writing a bell character (control-G) to the terminal.  Under
MS-DOS, ringing the bell this way produces a yucky sound,  so for MS-DOS
this function controls the signal generator directly.

*****************************************************************************/

VVOID ZBell()
{
        ZDspCh('\7');
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
        5.  If the NoWait argument is set, don't wait
        6.  When the user hits the RETURN key,  TECO is supposed to see
            a carriage return and then a line feed.  The function must
            deal with this by returning a carriage return to the caller
            and then "remembering" to send a line feed on the next call.
        7.  handle ET_BKSP_IS_DEL flag

*****************************************************************************/

static void CntrlC(void)        
{
        if (EtFlag & ET_TRAP_CTRL_C) {          /* if user wants it */
                EtFlag &= ~ET_TRAP_CTRL_C;      /* turn off bit */
        } else {
                if (EtFlag & ET_MUNG_MODE) {    /* if in MUNG mode */
                        TAbort(EXIT_SUCCESS);
                }
                GotCtC = TRUE;                  /* set "stop soon" flag */
        }
}

DEFAULT ZChIn(NoWait)                   /* input a character from terminal */
BOOLEAN NoWait;                         /* return immediately? */
{
        char Charac;
        static BOOLEAN NeedLF = FALSE;
        
        if (NeedLF) {
                NeedLF = FALSE;
                return (DEFAULT)LINEFD;
        }

        if (NoWait) {
                int i;
                ioctl(0, FIONREAD, &i);
                if (i==0) return -1;
        }
        for (;;) {
                if (read(0, &Charac, 1) <= 0) {
                    /* Read fails -- EOF?? */
                    return -1;
                }
                if (Charac == 0x03) {           /* ^C? */
                        CntrlC ();
                        break;
                }
                if (Charac != 0) {              /* not an IBM PC scan code? */
                        break;                  /* ??? what happens on ^@? */
                }
                read(0, &Charac, 1);
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

although more functions may be defined.

*****************************************************************************/

LONG ZClnEG(                    /* execute special :EG command */
        DEFAULT EGWhat,         /* what to get/set/clear: MEM, LIB, etc. */
        DEFAULT EGOper,         /* operation: get, set or clear */
        charptr TxtPtr)         /* if setting,  value to set */
{
        char    *cp=NULL;       /* environment variable name */
        char    buf[100];       /* enough for envname + 80 char filename */
        LONG    retval;         /* -1L on success, 0L on failure */

        DBGFEN(2,"ZClnEG",NULL);
        DBGFEX(2,DbgFNm,"0");

        switch (EGWhat) {
            case EG_INI: cp = "TEC$INIT";    break;
            case EG_LIB: cp = "TEC$LIBRARY"; break;
            case EG_MEM: cp = "TEC$MEMORY";  break;
            case EG_VTE: cp = "TEC$VTEDIT";  break;
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
}

/*****************************************************************************

        ZClnUp()

        This function cleans up in preparation for terminating TECO-C.

*****************************************************************************/

VVOID ZClnUp(void)                      /* clean up for exit */
{
    ioctl(0,TCSETA,&oldstate);
        setmode(1, O_TEXT);
}

/*****************************************************************************

        Zcp2ul()

        This function converts a pointer to an unsigned long.

*****************************************************************************/

#if DEBUGGING
ULONG Zcp2ul(voidptr cp)                /* convert charptr to ULONG */
{
        return ((ULONG)(cp));
}
#endif

/*****************************************************************************

        ZDoCmd()

        This function terminates TECO and feeds a command line to the
command line interpreter.

*****************************************************************************/

VVOID ZDoCmd(charptr GBfBeg, charptr GBfPtr)    /* die and pass command to OS */
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
        if ((space_p = strchr (GBfBeg,' ')) != NULL) {
                *space_p++ = '\0';
        }

        ZClnUp ();


        if ((comspec = getenv("OS2_SHELL")) != NULL) {
                execlp (comspec,
                        comspec,
                        "/c",
                        GBfBeg,
                        (space_p) ? space_p : NULL, NULL);
        } else {
                execlp (GBfBeg,
                        GBfBeg,
                        (space_p) ? space_p : NULL, NULL);
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
is running on,  then this function can simply call ZDspCh for every character
in the buffer.

*****************************************************************************/

VVOID ZDspBf(buffer, length)    /* output a buffer to terminal */
charptr buffer;
SIZE_T length;
{
        write(1, buffer, length);
}

/*****************************************************************************

        ZDspCh()

        This function outputs a single character to the terminal.

*****************************************************************************/

VVOID ZDspCh(Charac)            /* output a character to terminal */
char Charac;
{
        char c = Charac;
        write(1, &c, 1);
}

/*****************************************************************************

        ZExCtB()

        This function implements the TECO ^B command,  which returns the
current date encoded in the following way:

                ((year-1900)*16+month)*32+day

*****************************************************************************/

DEFAULT ZExCtB(void)                    /* return current date */
{
        time_t Clock;
        struct tm *time_of_day;
        int tecodate;

        DBGFEN(1,"ZExCtB","");
        Clock=time(NULL);
        time_of_day=localtime(&Clock);

        tecodate = ((time_of_day->tm_year)*16+time_of_day->tm_mon+1)*32
                + time_of_day->tm_mday ;

        DBGFEX(1,DbgFNm,"PushEx()");
        return PushEx(tecodate, OPERAND);
}

/*****************************************************************************

        ZExCtH()

        This function implements the TECO ^H command,  which returns the
current time encoded in the following way:

                (seconds since midnight) / 2

*****************************************************************************/

DEFAULT ZExCtH()                        /* return current time */
{
        time_t Clock;
        struct tm *time_of_day;
        int tecotime;

        DBGFEN(1,"ZExCtH","");
        Clock=time(NULL);
        time_of_day=localtime(&Clock);

        tecotime = time_of_day->tm_hour * 60    /* hours * 60 */;
        tecotime += time_of_day->tm_min;        /* minutes */
        tecotime *= 30;
        tecotime += time_of_day->tm_sec >> 1;   /* seconds / 2 */

        DBGFEX(1,DbgFNm,"PushEx()");
        return PushEx(tecotime, OPERAND);
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
                DBGFEX(1,DbgFNm,"PushEx(25857)");
                return PushEx((LONG)25857, OPERAND);    /* means "OS/2" */
        }

        if (NArgmt == 0) {
                DBGFEX(1,DbgFNm,"PushEx(getppid())");
                return PushEx((LONG)getppid(), OPERAND);        /* process id */
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

VVOID ZFree(pointer)            /* free memory allocated by ZAlloc */
voidptr pointer;
{
        free(pointer);
}

/*****************************************************************************

        ZHelp()

        This function accepts a help string and displays the corresponding
help text.

        it should be control-C interrupt-able.

*****************************************************************************/

VVOID ZHelp(HlpBeg, HlpEnd, SysLib, Prompt)
charptr HlpBeg;                 /* first char of help request */
charptr HlpEnd;                 /* last character of help request */
BOOLEAN SysLib;                 /* use default HELP library? */
BOOLEAN Prompt;                 /* enter interactive help mode? */
{
        (void)ExeNYI();
}

/*****************************************************************************

        ZIClos()

        This function closes the current input file.  It must

        1.  if current input stream is not open,  simply return
        2.  close the input file
        3.  set open indicator to FALSE

*****************************************************************************/

VVOID ZIClos(IfIndx)                    /* close input file */
DEFAULT IfIndx;                         /* index into IFiles array */
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
#define BUFSIZE 20480
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
    stat(srcname, &statbuf);
    times.actime = statbuf.st_atime;
    times.modtime = statbuf.st_mtime;
    utime(dstname, &times);
    return 0;
}


VVOID ZOClos(DEFAULT OfIndx)    /* close output file */
{
    char        *DotPtr;
    char        *SlaPtr1, *SlaPtr2;
    char        TmpFsp[FILENAME_MAX];
        int                     failflag;

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
                        DotPtr = strrchr(TmpFsp, '.');          /* find the "." */
                        /* We will ignore dot if it is before a slash */
                        if (DotPtr != NULL && (SlaPtr1==NULL || DotPtr > SlaPtr1)) {
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
             * rename "filename" to "filename.bak" if NORENAME flag not set.
             * If NORENAME flag set or if rename fails, try copying file
                         * If a copy is done, then the original file will remain, which
                         * is a good thing for OS/2 anyway!
             */

                        if (EzFlag&EZ_NORENAME ||
                                (failflag = rename(OFiles[OfIndx].OTNam, TmpFsp))) {
                                failflag = fastcopy(OFiles[OfIndx].OTNam, TmpFsp);
                        }
            if (failflag) {
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
        else { /* delete original if not backing up and not "norename" */
            if (!(EzFlag&EZ_NORENAME) &&
                                remove(OFiles[OfIndx].OTNam) != 0) {
                ZErMsg();
                ErrMsg(ERR_UCO);
                DBGFEX(2,DbgFNm,"remove() failed");
                exit(EXIT_FAILURE);
            }
        }


        /*
         * rename "tmpnam" to "filename" if NORENAME flag not set.
         * If NORENAME flag set or if rename fails (which can occur if filename
         * not in current directory) then try copying the file
         */

                if (EzFlag&EZ_NORENAME ||
                        (failflag = rename(OFiles[OfIndx].OFNam,OFiles[OfIndx].OTNam))) {
                        failflag = fastcopy(OFiles[OfIndx].OFNam,OFiles[OfIndx].OTNam);
                }
        if (failflag) {
            ZErMsg();
            ErrMsg(ERR_UCO);
            DBGFEX(2,DbgFNm,"rename() failed, calling exit()");
            exit(EXIT_FAILURE);
        }
                /* delete the temporary file, if it is still around */
                remove(OFiles[OfIndx].OFNam);
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

DEFAULT ZOpInp(IfIndx, EIFile, RepFNF)
DEFAULT IfIndx;                 /* index into file data block array IFiles */
BOOLEAN EIFile;                 /* is it a macro file? (hunt for it) */
BOOLEAN RepFNF;                 /* report "file not found" error? */
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

        This function creates (and opens) an output file.  The name of
the file to be created is pointed to by FBfBeg.  FBfPtr points to the
character following the last character of the file name.

*****************************************************************************/

DEFAULT ZOpOut(OfIndx, RepErr, Backup)          /* open output file */
DEFAULT OfIndx;                         /* output file indicator */
BOOLEAN RepErr;                         /* report errors? */
BOOLEAN Backup;                                                 /* EB */
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
						if (strchr(*++argv, ' ') != NULL) {
							/* embedded space in argument */
							strcat(TmpBuf, "\"");
							strcat(TmpBuf, *argv);
							strcat(TmpBuf, "\"");
						}
						else
							strcat(TmpBuf, *argv);
                        strcat(TmpBuf, " ");
                }
                line_len = strlen(TmpBuf)-1;    /* ignore trailing space */
                QR = &QRgstr[35];               /* 35 = q-register Z */
                if (MakRom(line_len) == FAILURE) {
                        DBGFEX(2,DbgFNm,
                               "MakRom(line_len) failed, calling exit()");
                        exit(EXIT_FAILURE);
                }
                MEMMOVE(QR->Start, TmpBuf, line_len);
                QR->End_P1 += line_len;         /* length of q-reg text */
        }

#if USE_ANSI_CLPARS

/*
 * execute imbedded command line-parsing macro directly from clpars[]
 */


        CStBeg = CBfPtr = clpars; /* command string start */
        CStEnd = clpars + CLPARS_LEN;/* command string end */
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

*****************************************************************************/

DEFAULT ZPWild()                /* preset the wildcard lookup filename */
{

/*
 * if they didn't process all of the filenames from a
 * previous EN' call, discard them
 */

        if (ff_names != NULL) {
                _fnexplodefree(ff_names);
        }

		
/*
 * Find all matching files (and, alas, directories)
 */

		*FBfPtr = '\0';
		ff_names_p = ff_names = _fnexplode((char *)FBfBeg);

		return SUCCESS;
}

/*****************************************************************************

        ZRaloc()

        This function performs the standard C library function realloc.

*****************************************************************************/

voidptr ZRaloc(OldBlk, NewSiz)
voidptr OldBlk;
SIZE_T NewSiz;
{
        return realloc(OldBlk, NewSiz);
}

/*****************************************************************************

        ZRdLin()

        This function reads a line from a file.  It is passed a buffer, the
size of the buffer, a file pointer.  It returns the length of the line,  or
sets IsEofI[] to TRUE if the end of file is encountered.

*****************************************************************************/

DEFAULT ZRdLin(                 /* read a line from a file */
        charptr ibuf,           /* where to put string */
        ptrdiff_t ibuflen,      /* max length of ibuf */
        int IfIndx,             /* index into IFiles[] */
        DEFAULT *retlen)        /* returned length of string */
{
        int character;          /* the last character read */
        DEFAULT charsLeft;      /* number of characters left */
        FILE *fp;               /* input stream pointer to read from */
        char *iBuf;             /* non-huge pointer into IBf for speed */

#if DEBUGGING
        static char *DbgFNm = "ZRdLin";
        sprintf(DbgSBf,"ibuf = %ld, ibuflen = %ld, IfIndx = %d",
                Zcp2ul(ibuf),(LONG)ibuflen,IfIndx);
        DbgFEn(3,DbgFNm,DbgSBf);
#endif

        charsLeft = ibuflen;
        fp = IFiles[IfIndx];
        iBuf = (char *) ibuf;

        while ((character = getc(fp)) >= 0) {    /* we got one */
                *iBuf++ = character;
                if (character == LINEFD || character == VRTTAB) {
                        /* finished with this line */
                        *retlen = ibuflen - charsLeft + 1;
#if DEBUGGING
                        sprintf(DbgSBf,"SUCCESS, retlen = %d", *retlen);
                        DbgFEx(3,DbgFNm,DbgSBf);
#endif
                        return SUCCESS;
                }
                if (character == FORMFD && !(EzFlag & EZ_FF)) {
                        /* toss form feed, then finished */
                        *retlen = ibuflen - charsLeft;
                        FFPage = -1;
#if DEBUGGING
                        sprintf(DbgSBf,"SUCCESS, retlen = %d", *retlen);
                        DbgFEx(3,DbgFNm,DbgSBf);
#endif
                        return SUCCESS;
                }
                if (--charsLeft == 0) {
                        /* no more room, so return */
                        *retlen = ibuflen;
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
        if ((*retlen = ibuflen-charsLeft) == 0) {
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

VVOID ZScrOp(OpCode)            /* do a screen operation */
int OpCode;                     /* code for operation */

{
        switch (OpCode) {
            case SCR_CUP:   ZDspBf("\033[A", 3);   break;
            case SCR_EEL:   ZDspBf("\033[K", 3);   break;
            case SCR_ROF:   ZDspBf("\033[m", 3);   break;
            case SCR_RON:   ZDspBf("\033[7m", 4);   break;
        }
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

DEFAULT ZSWild()                        /* search for next wildcard filename */
{
	struct stat statistics;
	int filename_length;

	/* FNfilename not called? */
	if (ff_names == NULL) return FILENF;

	while (*ff_names_p) {
		/* check that it is a file */
		stat(*ff_names_p, &statistics);
		if (!S_ISREG(statistics.st_mode)) {
			ff_names_p++;
			continue;
		}
		filename_length = strlen (*ff_names_p);
		MEMMOVE (FBfBeg, *ff_names_p, filename_length);
		FBfPtr = FBfBeg + filename_length;
		ff_names_p++;
		return SUCCESS;
	}

	/* list exhausted -- free memory */

	_fnexplodefree(ff_names);
	ff_names = ff_names_p = NULL;

	return FILENF;

}

/*****************************************************************************

        ZTrmnl()

        This function sets up the input/output of commands.  Usually, that
means the input/output channels to the terminal,  but TECOC might be run
from a command procedure (under VMS) or a script file (under __UNIX__),  and
that possibility must be handled.  In addition,  the handling of interrupts
is found here.
        In general,  this function must:

                1. Set TIChan so it can be used to read commands
                2. Set TOChan so it can be used for output
                3. handle interrupts
                4. initialize CrType (what kind of terminal it is)
                5. initialize EtFlag (terminal capability bits)
                6. initialize HtSize (number columns terminal has)
                7. initialize VtSize (number rows terminal has)

*****************************************************************************/

static void CntrlCTrap()
{
        signal(SIGBREAK, SIG_ACK);
        CntrlC();
}


VVOID ZTrmnl()          /* set up I/O to the terminal */
{
        DBGFEN(2,"ZTrmnl",NULL);

    ioctl(0,TCGETA,&oldstate); /* switch to raw mode */
    newstate = oldstate;
    newstate.c_iflag = 0;
    newstate.c_lflag = 0;
    ioctl(0,TCSETA,&newstate);
        setmode(1, O_BINARY);

        EtFlag = ET_READ_LOWER |        /* don't convert lower to upper */
                 ET_BKSP_IS_DEL |       /* backspace key is delete key */
                 ET_SCOPE |             /* ok to use video for del and ^U */
                 ET_EIGHTBIT;           /* terminal uses 8-bit characters */

        CrType = VT100;                 /* Codes are enough like VT100 */
        signal(SIGBREAK, CntrlCTrap);

#if VIDEO
        VtSize = HtSize = 0;
#endif

        DBGFEX(2,DbgFNm,NULL);
}

/*****************************************************************************

        ZVrbos()

        This function s a buffer to a file.

*****************************************************************************/

VVOID ZVrbos(ErrNum, ErMnem)
WORD ErrNum;
char *ErMnem;
{
        char **TmpPtr;
#include "vrbmsg.h"

        ZDspBf("\r\n",2);
        for (TmpPtr = &ParaTx[StartP[LstErr]]; *TmpPtr; ++TmpPtr) {
                ZDspBf((charptr)*TmpPtr, strlen(*TmpPtr));
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
       unsigned long bufsiz = BfrEnd - BfrBeg + 1;

        if (fwrite(BfrBeg,sizeof(char),bufsiz,OFiles[OfIndx].OStrem) != bufsiz) {
                ZErMsg();
                ErrMsg(ERR_UWL);
                DBGFEX(2,DbgFNm,"Zfwrite() failed");
                return FAILURE;
        }

        DBGFEX(2,DbgFNm,"SUCCESS");

        return SUCCESS;
}
