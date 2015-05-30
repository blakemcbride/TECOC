/*****************************************************************************
	zosx.c
	System dependent code for Mac OS X
    This is based on zunix.c, with changes (see comments "TAA")
    for enhanced functionality.
*****************************************************************************/
/*
 * Define standard functions. 
 */
#include <sys/types.h>		/* needed before sys/param.h is included */
#include <errno.h>		/* define errno */
#include <signal.h>		/* to catch ^C and ^Z signals */
#include <stdio.h>		/* define stdin */
#include <string.h>		/* strncpy(), strlen(), etc. */
#include <sys/time.h>		/* define tm struct */
#include <dirent.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <glob.h>		/* Rewritten 05/04 by TAA to use glob function */
#if CURSES
#include <curses.h>		/* has to come before zport.h */
#endif
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "chmacs.h"		/* define character processing macros */
#include "clpars.h"		/* command-line parsing macro */
#include "dchars.h"		/* define identifiers for characters */
#include "deferr.h"		/* define identifiers for error messages */
#include "defext.h"		/* define external global variables */
#include "dscren.h"		/* define identifiers for screen i/o */
#if CURSES
/*
 * The ACS_xxx symbols are defined in the SunOS version of curses, but not
 * in either the BSD or X/OPEN Ultrix versions.  Who knows where else they
 * are defined?  Isn't portability wonderful?
 *
 * Note that this code implicitly assumes a VT100-compatible terminal.
 * Tough.  (Jerry Leichter, June 1991)
 */
#ifndef ACS_PLMINUS
#define ACS_PLMINUS	(A_ALTCHARSET | 0x67)
#define ACS_LLCORNER	(A_ALTCHARSET | 0x60)
#define ACS_LRCORNER	(A_ALTCHARSET | 0x6A)
#define ACS_LARROW	(A_ALTCHARSET | 0x7C)	/* Really not-equal	*/
#define ACS_HLINE	(A_ALTCHARSET | 0x71)
#define ACS_BTEE	(A_ALTCHARSET | 0x76)
#define ACS_DIAMOND	(A_ALTCHARSET | 0x60)
#define ACS_RTEE	(A_ALTCHARSET | 0x75)
#endif
WINDOW *curwin;
int ScopeFlg = 0;		/* for ZDspCh and ZDspBuf */
int olddot;
#define DOT (GapBeg - EBfBeg)
#define ISSCW (ScopeFlg)
unsigned char *p_scope_start;
int scope_start;
int winline;
int cursoratbottomline;
int cursorattopline;
static int colsave;
int dotx,doty;			/* position of cursor in scope */
int dotx1, doty1;		/* pos. after printing character in scope */
int skiprefresh = 0;
int cmdx,cmdy;			/* position of cursor in command buffer */
int scope_start, scope_end;
unsigned char *pscope_start;
#define CT ((EzFlag & EZ_VT100GRAPHICS) ? ACS_PLMINUS : '^')
#define LB ((EzFlag & EZ_VT100GRAPHICS) ? ACS_LLCORNER : '[')
#define RB ((EzFlag & EZ_VT100GRAPHICS) ? ACS_LRCORNER : ']')
#else				/* else (not CURSES) ----------------------*/
#ifdef LINUX
static struct termios  out, cur;	/* terminal characteristics buffers */
#else
#include <sgtty.h>		/* define CBREAK, ECHO, TIOCGETP, etc. */
static struct sgttyb out, cur;	/* terminal characteristics buffers */
#endif
static char tbuf[1024];		/* store TERMCAP entry here */
static char tarea[1024];	/* store decoded TERMCAP stuff here */
static char *ce;		/* TERMCAP sequence: clear to end-of-line */
static char *up;		/* TERMCAP sequence: cursor up */
static char *so;		/* TERMCAP sequence: reverse video on */
static char *se;		/* TERMCAP sequence: reverse video off */
int	tputs();		/* send termcap string to a given function */
int	tgetent();		/* load a terminal capability buffer */
char 	*tgetstr();		/* get str value of a terminal capability */
#endif
static BOOLEAN tty_set = FALSE;	/* Has the terminal been set? */
static struct termios  out, cur;	/* terminal characteristics buffers */
static char tbuf[1024];		/* store TERMCAP entry here */
// static char tarea[1024];	/* store decoded TERMCAP stuff here */
// static char *ce;		/* TERMCAP sequence: clear to end-of-line */
// static char *up;		/* TERMCAP sequence: cursor up */
// static char *so;		/* TERMCAP sequence: reverse video on */
// static char *se;		/* TERMCAP sequence: reverse video off */
int	tputs();		/* send termcap string to a given function */
int	tgetent();		/* load a terminal capability buffer */
char 	*tgetstr();		/* get str value of a terminal capability */
static int vernum();		/* see bottom of this file */
//extern	int sys_nerr;		/* number of system error messages */
static	int SupGotCtC = 0;
static glob_t pglob;
static int globindex = 0;
/*****************************************************************************
	IFiles holds the file data blocks for input files.  There are three
static input streams:   the primary input stream,  the secondary input stream,
and the input stream used by the EQq command.  To access these three files,
identifiers defined in file tecoc.h are used to index into this array.
Other elements of this array are used to access input files for the EI
command.
*****************************************************************************/
FILE *IFiles[NIFDBS];
/* TAA Mod 5/04 -- keep track of last character being CR so we will
 * have CRLF -> CRLF instead of CRLFLF in case input file is DOS
 * format. */
static int IFisCR[NIFDBS] = {0};
/*****************************************************************************
	OFiles holds the file data blocks for the output files.  There are
three output streams:   the primary output stream,  the secondary output
stream and the output stream used by the E%q command.  The array is indexed
using identifiers defined in file tecoc.h.
*****************************************************************************/
static struct {
	char	OFNam[FILENAME_MAX];	/* output file name */
	char	OTNam[FILENAME_MAX];	/* temporary output file name */
	FILE	*OStrem;		/* stream */
	BOOLEAN forBackup;          /* TAA Mod */
} OFiles[NOFDBS];
/*****************************************************************************
	ZErMsg()
	This function displays error message from the operating system on
the terminal screen.  The error message text is retrieved from the operating
system and imbedded in a TECO-style message with the SYS mnemonic.
*****************************************************************************/
static VVOID ZErMsg()
{
	if (errno < sys_nerr)
		ErrStr(ERR_SYS, sys_errlist[errno]);
}
/*****************************************************************************
	ZAlloc()
	This function allocates memory.  The single argument is the number of
bytes to allocate.  TECO-C uses the ZFree and ZRaloc functions to de-allocate
and re-allocate, respectively,  the memory allocated by this function.
*****************************************************************************/
voidptr ZAlloc(MemSize)		/* allocate memory */
SIZE_T MemSize;
{
	return (voidptr)malloc(MemSize);
}
/*****************************************************************************
	ZBell()
	Thus function rings the terminal bell.  For most platforms,  this
means just writing a bell character (control-G) to the terminal.  Under
MS-DOS, ringing the bell this way produces a yucky sound,  so for MS-DOS
this function controls the signal generator directly.
*****************************************************************************/
VVOID ZBell(VVOID)
{
#if CURSES
	if (EzFlag & EZ_AUDIO_BEEP)
		beep();				/* audio beep */
	else
		flash();			/* visible flash */
#else
	ZDspCh('\7');
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
	5.  If the NoWait argument is TRUE, don't wait.
	6.  When the user hits the RETURN key,  TECO is supposed to see
	    a carriage return and then a line feed.  The function must
	    deal with this by returning a carriage return to the caller
	    and then "remembering" to send a line feed on the next call.
	7.  handle ET_BKSP_IS_DEL flag
*****************************************************************************/
DEFAULT ZChIn(NoWait)			/* input a character from terminal */
BOOLEAN NoWait;				/* return immediately? */
{
	char Charac;
	static	BOOLEAN	LastLF = FALSE;
	if (LastLF) {
		LastLF = FALSE;
		return (DEFAULT)LINEFD;
	}
#if CURSES
	if ((Charac = getch()) == ERR) {
#else
	if (read(fileno(stdin), &Charac, 1) != 1) {
#endif
		if (GotCtC || SupGotCtC)
			return (DEFAULT)CTRL_C;
		if (!GotCtC) {
			ZErMsg();
			ErrMsg(ERR_URC);
			ZClnUp();
			exit(EXIT_FAILURE);
		}
	}
	GotCtC = FALSE;
	if (Charac == CRETRN) {
		LastLF = TRUE;
		return (DEFAULT)CRETRN;
	}
	if (EtFlag & ET_BKSP_IS_DEL) {
		if (Charac == DELETE) {
			Charac = BAKSPC;
		} else if (Charac == BAKSPC) {
			Charac = DELETE;
		}
	}
	return (DEFAULT)Charac;
}
/*****************************************************************************
	ZClnEG()
	This function executes a :EG command.  The :EG commands are used to
get access to operating system functions.  The minimum set of functions is
	:EGINI$		gets, sets or clears the initialization file name
	:EGMEM$		gets, sets or clears the file name memory
	:EGLIB$		gets, sets or clears the macro library directory
	:EGVTE$		gets, sets or clears the video macro file name
although more functions may be defined.
The :EG command was designed to access logical names,  which are supported
by DEC's VAX/VMS and RSX operating systems.  Logical names are a useful way
to specify, for example, a directory that a program is to find a set of files
in.  A user can define logical names to set up a program's environment.
Programs can read, create or delete logical names.
Logical names are stored separately from program memory, so if a program sets
a logical name and then exits, the logical name still exists.  TECO on a VAX
uses a logical name to store the name of the file being edited.  If the user
starts TECO without specifying a file name, TECO looks for the logical name
and, if it exists, uses the value of the logical name as a filename.  This
allows users to edit a file several times in a session without having to
type the file name each time they start TECO (except the first time).
Unix doesn't have logical names.  The closest thing is environment variables,
which are passed to a program when it is started.  A user can define
environment variables,  and a program can get the values with a getenv call.
A program can even add to it's private list of environment variables,  but
the list disappears when the program exits.  So environment variables don't
fill the needs of the :EG command.
Environment variables are, however, natural for some of what :EG is really
used for.  Users rarely need the :EG command,  even in macros.  The main use
of :EG is in the command-line-parsing macro (in CLPARS.TES, CLPARS.TEC and
CLPARS.H).  That macro can handle a partially-implemented :EG command (it
tests the success/failure flag returned by :EG).
So I partially implemented :EG for Unix.  :EG can read the "INI", "LIB" and
"VTE" values,  but can't set or clear them.  The "MEM" value is supported
using a file (ugh) to save the name of the last-file-edited.  The file is
stored in /tmp so it gets deleted when the system boots.
*****************************************************************************/
	LONG ZClnEG(                    /* execute special :EG command */
				DEFAULT EGWhat,         /* what to get/set/clear: MEM, LIB, etc. */
				DEFAULT EGOper,         /* operation: get, set or clear */
				charptr TxtPtr)         /* if setting,  value to set */
	{
		char    *cp=NULL;       /* environment variable name */
//		char    buf[100];       /* enough for envname + 80 char filename */
		LONG    retval;         /* -1L on success, 0L on failure */
		DBGFEN(2,"ZClnEG",NULL);
		DBGFEX(2,DbgFNm,"0");
		switch (EGWhat) {
			case EG_INI: cp = "TEC_INIT";    break;
			case EG_LIB: cp = "TEC_LIBRARY"; break;
			case EG_MEM: cp = "TEC_MEMORY";  break;
#if VIDEO
			case EG_VTE: cp = "TEC_VTEDIT";  break;
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
			/* Cannot set value */
// 			strcpy(buf, cp);                /* build NAME= */
// 			strcat(buf, "=");
// 			if (EGOper == SET_VAL) {        /* concatenate new value */
// 				strcat(buf, (char *)TxtPtr);
// 			}
// 			retval = (putenv(buf) != 0)     /* if putenv() failed */
// 					 ? 0L            /*   then return failure */
// 					 : -1L;          /*   else return success */
			retval = 0L;
		}
		return retval;
	}
/*****************************************************************************
  See the definition of MEMMOVE in ZPORT.H for a description of this
  function.
*****************************************************************************/
VVOID ZCpyBl(Destin, Source, Length)
charptr Destin;
charptr Source;
SIZE_T Length;
{
	if (Source < Destin) {
		Source += Length;
		Destin += Length;
		while (Length-- > 0) {
			*--Destin = *--Source;
		}
	} else {
		while (Length-- > 0) {
			*Destin++ = *Source++;
		}
	}
}
#if DEBUGGING
ULONG Zcp2ul(cp)		/* convert charptr to ULONG */
voidptr cp;
{
	return (ULONG)(cp);
}
#endif
/*****************************************************************************
	ZClnUp()
	This function cleans up in preparation for terminating TECO-C.
*****************************************************************************/
VVOID ZClnUp(VVOID)			/* cleanup for TECO-C abort */
{
	DBGFEN(3,"ZClnUp","closing terminal channels and exiting");
#if !CURSES
	if (tty_set == TRUE)
#ifdef LINUX
		tcsetattr(0, TCSANOW, &out);
#else
		ioctl(0, TIOCSETP, &out);
#endif
#endif
}
/*****************************************************************************
	ZDoCmd()
	This function terminates TECO and feeds a command line to the
command line interpreter.  The command to be executed is passed to this
function.
*****************************************************************************/
VVOID ZDoCmd(charptr GBfBeg, charptr GBfPtr)	/* die and pass command to OS */
{
	char	buf[128+1];
	char	*space_p;
	DBGFEN(1,"ZDoCmd",NULL);
/*
 * 1. Terminate buf[] and command line in GBf
 * 2. make local copy since GBf will be free'd in ZClnUp()
 * 3. separate program name from arguments, if any
 * 4. Call ZClnUp to free up everything
 * 5. Execute the command line, with optional arguments.  If we know where
 *    the command processor is, use it so we can execute .BAT batch files
 * 6. we shouldn't be here, exit
 */
	buf[128] = *GBfPtr = '\0';
	(void)strncpy(buf, (char *)GBfBeg, 128);
	if ((space_p = strchr(buf,' ')) != NULL) {
		*space_p++ = '\0';
	}
	ZClnUp();
	execlp ("/bin/sh", "sh", "-c", buf, (space_p) ? space_p : NULL, NULL);
	/* we should never reach this statement */
	(void)perror ("");
	ZExit (EXIT_SUCCESS);
}
#if CURSES
static void
zaddch(c,wecho)
char c;
int wecho;					/* if set - also do refresh */
{
    static int retflag = 0;
    int needrefresh = 1;
    int y,x;
    if (c=='\n') {
	if (!retflag)				/* previous char was not \r */
	    waddch(curwin,c);
	else {			/* this LF is part of a CRLF sequence */
	    waddch(curwin,'\n');		/* this may force a scroll */
	    waddch(curwin,'\r');
	}
    } else if (c=='\b') {			/* backspace */
	getyx(stdscr,y,x);
	if (x==0 && y>0)
	    move(y-1,HtSize - 1);
	else if (x>0)
	    move(y,x-1);
	else					/* x==0 and y==0 */
	    ;
    } else {			/* c is neither a newline nor a backspace */
	if (retflag)
	    waddch(curwin,'\r');
	if (c!='\r') {
	    if (ISSCW) {
		getyx(stdscr,y,x);
		if (x < HtSize -1)
		    waddch(curwin,c);
	    } else
		if (wecho) {
#ifdef ULTRIX
		    waddch(curwin,c);
#else
		    wechochar(curwin,c);
		    needrefresh = 0;
#endif
		} else
		    waddch(curwin,c);
	}
    }
    retflag = (c=='\r');
    if (wecho && needrefresh)
	refresh();
}
static void
specon()
{
    if (SpcMrk)
	wattron(stdscr,0400000L * (long) SpcMrk);
}
static void
specoff()
{
    if (SpcMrk)
	wattroff(stdscr,0400000L * (long) SpcMrk);
}
static void
gr_on()
{
    wattron(stdscr,A_ALTCHARSET);
}
static void
gr_off()
{
    wattroff(stdscr,A_ALTCHARSET);
}
static int
intabs(t)
int t;
{
    return (t<0) ? -t : t;
}
static void
zaddch2(c)
char c;
{
    static int retflag = 0;
    if (SeeAll) {
	if (c=='\n') {
	    if (EzFlag & EZ_VT100GRAPHICS) {
		gr_on();
		waddch(curwin, 'e');
		gr_off();
	    } else {
		waddch(curwin,CT);
		waddch(curwin,'J');
	    }
	    if (!retflag) {		/* previous character was not \r */
		waddch(curwin,c);
	      } else {			/* this LF is in a CRLF sequence */
		waddch(curwin,'\n');
		waddch(curwin,'\r');
	    }
	} else {			/* c is not a newline */
	    if (c=='\r') {
		if (EzFlag & EZ_VT100GRAPHICS) {
		    gr_on();
		    waddch(curwin,'d'); gr_off();
		} else {
		    waddch(curwin,CT);
		    waddch(curwin,'M');
		}
	    } else if (c=='\t') {
		if (EzFlag & EZ_VT100GRAPHICS) {
		    int y,x,y1,x1,cntr;
		    getyx(curwin,y,x);
		    waddch(curwin,'\t');
		    getyx(curwin,y1,x1);
		    move(y,x);
		    gr_on();
		    cntr = 0;
		    waddch(curwin,'b');
		    getyx(curwin,y,x);
		    while ((cntr < 8) && (y!=y1 || x!=x1) &&
					(y<VtSize - winline - ScroLn)) {
			waddch(curwin,'~');
			cntr++;
			getyx(curwin,y,x);
		    }
		    gr_off();
		} else {
		    waddch(curwin,CT); waddch(curwin,'I');
		}
	    } else if (c=='\f') {
		if (EzFlag & EZ_VT100GRAPHICS) {
		    gr_on();
		    waddch(curwin,'c');
		    gr_off();
		} else {
		    waddch(curwin,CT);
		    waddch(curwin,'L');
		}
		waddch(curwin,'\n');
		waddch(curwin,'\r');
	    } else if (c==27) {
		if (EzFlag & EZ_VT100GRAPHICS) {
		    gr_on();
		    waddch(curwin,'{');
		    gr_off();
		} else {
		    waddch(curwin,CT);
		    waddch(curwin,'[');
		}
	    } else if (c==VRTTAB) {		/* Vertical tab */
		if (EzFlag & EZ_VT100GRAPHICS) {
		    gr_on();
		    waddch(curwin,'i');
		    gr_off();
		} else {
		    waddch(curwin,CT);
		    waddch(curwin,'K');
		}
		waddch(curwin,'\n');
		waddch(curwin,'\r');
	    } else if (c=='\b') {
		waddch(curwin,CT);
		waddch(curwin,'H');
	    } else if (c & 0200) {
		int i;
		char a,b;
		waddch(curwin,LB);
		c = c & 0177;
		i = c/16;
		if (i==0)
		    a='8';
		else if (i==1)
		    a='9';
		else
		    a = i - 2  + 'A';
		i = (c % 16);
		b = (i > 9) ? (i - 10 + 'A') : (i + '0');
		waddch(curwin,a);
		waddch(curwin,b);
		waddch(curwin,RB);
	    } else if (c < 32) {
		waddch(curwin, CT);
		waddch(curwin, c | 64);
	    } else
		waddch(curwin,c);
	}
	retflag = (c==CRETRN);
    } else {					/* not in SEEALL mode */
	c = c & 0177;				/* dump 8th bit */
	if (c=='\n') {
	    waddch(curwin,'\n');
	    waddch(curwin,'\r');
	} else if (c=='\b') {
	    waddch(curwin,'^');
	    waddch(curwin,'H');
	} else {		/* c is neither a newline nor a backspace */
	    switch (c) {
	        case CRETRN:
	            if (EzFlag & EZ_INVCR) {
			waddch(curwin, ' ');
		    } else if (EzFlag & EZ_VT100GRAPHICS) {
			gr_on();
			waddch(curwin, 'd');
			gr_off();
		    } else if (EzFlag & EZ_ARROW) {
			specon();
			waddch(curwin, ACS_LARROW);
			specoff();
		    } else {
			specon();
			waddch(curwin,'`');
			specoff();
		    }
		    break;
	        case ESCAPE:
		    specon();
		    waddch(curwin,'$');
		    specoff();
		    break;
		case VRTTAB:
		case FORMFD:
		    if (EzFlag & EZ_VT100GRAPHICS) {
			gr_on();
			waddch(curwin,c);
			gr_off();
		    } else {
			waddch(curwin, c);
		    }
		    waddch(curwin, '\n');
		    waddch(curwin, '\r');
		default:
		    waddch(curwin,c);
	    }
	}
	retflag = (c=='\r');
    }
}
#endif
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
VVOID ZDspBf(buffer, length)		/* output a buffer to terminal */
charptr buffer;
SIZE_T length;
{
#if CURSES
	int i;
	int y,x;
	for (i=0;i<length;i++) {
		if (ISSCW) {
			getyx(stdscr,y,x);
			if (x>=(HtSize-1))
				break;
		}
		if (!GotCtC)
			zaddch(*(buffer+i),0);
		else
			break;
	}
	wrefresh(curwin);
#else
	if (write(fileno(stdout), buffer, length) == -1) {
		puts("Unable to write to terminal in function ZDspBf");
		TAbort(EXIT_FAILURE);
	}
#endif
}
/*****************************************************************************
	ZDspCh()
	This function outputs a single character to the terminal.
*****************************************************************************/
VVOID ZDspCh(Charac)		/* output a character to terminal */
char Charac;
{
#if CURSES
	zaddch(Charac,1);
#else
	if (write(fileno(stdout), &Charac, 1) == -1) {
		puts("Unable to write to terminal in function ZDspCh");
		TAbort(EXIT_FAILURE);
	}
#endif
}
/*****************************************************************************
	ZExCtB()
	This function implements the TECO ^B command,  which returns the
current date encoded in the following way:
		((year-1900)*16+month)*32+day
*****************************************************************************/
DEFAULT ZExCtB()			/* return current date */
{
	time_t clockt;
	struct tm *time_of_day;
	int tecodate;
	DBGFEN(1,"ZExCtB","");
	clockt=time(NULL);
	time_of_day=localtime(&clockt);
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
DEFAULT ZExCtH()			/* return current time */
{
	time_t clockt;
	struct tm *time_of_day;
	int tecotime;
	DBGFEN(1,"ZExCtH","");
	clockt=time(NULL);
	time_of_day=localtime(&clockt);
	tecotime = time_of_day->tm_hour * 60	/* hours * 60 */;
	tecotime += time_of_day->tm_min;	/* minutes */
	tecotime *= 30;
	tecotime += time_of_day->tm_sec >> 1;	/* seconds / 2 */
	DBGFEX(1,DbgFNm,"PushEx()");
	return PushEx(tecotime, OPERAND);
}
/*****************************************************************************
	ZExeEJ()
	This function executes an EJ command,  which returns environment
characteristics.  It returns:
	-1EJ	1024 under VAX/VMS	(4*256 = VAX, 0 = VMS in native mode)
        1025 under Ultrix       (4*256 = VAX, 1 = Ultrix)
        25600 under Sun/SunOS   (100*256 = Sun, 0 = SunOS)
		25856 under MS-DOS	(101*256 = IBM-PC, 0 = MS-DOS)
		25857 under OS/2    (101*256 = IBM-PC, 1 = OS/2)
        25858 under Linux   (101*256 = IBM-PC, 2 = Linux)
	0EJ	process id on VAXen or Unix/Linux, 0 on anything else
	1EJ	0 on all systems
	2EJ	UIC, in longword format (unlike TECO-11) on VAX/VMS,
		0 on all other systems.
*****************************************************************************/
DEFAULT ZExeEJ()			/* execute an EJ command */
{
	DBGFEN(1,"ZExeEJ",NULL);
	if (EStTop == EStBot) {			/* if no numeric argument */
		NArgmt = 0;			/* default is 0EJ */
	} else {
		UMinus();			/* if -EJ, make it -1EJ */
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	if (NArgmt == -1) {
		DBGFEX(1,DbgFNm,"PushEx(25858)");
		return PushEx((LONG)25858, OPERAND);	/* means "PC Linux" */
	}
	if (NArgmt == 0) {
		DBGFEX(1,DbgFNm,"PushEx(getppid())");
		return PushEx((LONG)getppid(), OPERAND);
	}
	DBGFEX(1,DbgFNm,"ExeNYI()");
	return ExeNYI();
}
/*****************************************************************************
	ZExit()
	This function terminates TECO-C with a status value.
*****************************************************************************/
VVOID ZExit(estat)		/* terminate TECO-C */
DEFAULT estat;
{
#if CURSES
	nodelay(stdscr,FALSE);	/* to avoid a bug in some System V.2 */
                                /* releases of curses */
	endwin();
#endif
	ZClnUp();
	exit(estat);
}
/*****************************************************************************
	ZFree()
	This function frees memory previously allocated by the ZAlloc
function.
*****************************************************************************/
VVOID ZFree(pointer)		/* free memory allocated by ZAlloc */
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
charptr HlpBeg;		/* first char of help request */
charptr HlpEnd;		/* last character of help request */
BOOLEAN SysLib;		/* use default HELP library? */
BOOLEAN	Prompt;		/* enter interactive help mode? */
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
VVOID ZIClos(IfIndx)			/* close input file */
DEFAULT	IfIndx;				/* index into IFiles array */
{
	DBGFEN(2,"ZIClos",NULL);
	if (IsOpnI[IfIndx]) {			/* if it's open */
		if (fclose(IFiles[IfIndx]) == EOF) {
			ZErMsg();
			ErrMsg(ERR_UCI);
			ZExit(EXIT_FAILURE);
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
VVOID ZOClDe(OfIndx)			/* close and delete output file */
DEFAULT	OfIndx;				/* index into OFiles array */
{
	DBGFEN(2,"ZOClDe",NULL);
	if (IsOpnO[OfIndx]) {		/* if output stream is open */
		if (fclose(OFiles[OfIndx].OStrem) == EOF) {
			ZErMsg();
			ErrStr(ERR_UCO, OFiles[OfIndx].OFNam);
			ZExit(EXIT_FAILURE);
		}
		if (remove(OFiles[OfIndx].OFNam) != 0) {
			ZErMsg();
			ErrStr(ERR_UCD, OFiles[OfIndx].OFNam);
			ZExit(EXIT_FAILURE);
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
VVOID ZOClos(OfIndx)		/* close output file */
DEFAULT	OfIndx;			/* index into OFiles array */
{
	int ver;
	char TmpFsp[FILENAME_MAX];
	char move_err[1024];
	DBGFEN(2,"ZOClos",NULL);
	if (!IsOpnO[OfIndx]) {				/* if it's not open */
		DBGFEX(2,DbgFNm,NULL);
		return;					/* we're done */
	}
	if (fclose(OFiles[OfIndx].OStrem) == EOF) {	/* close it */
		ZErMsg();
		ErrMsg(ERR_UCO);			/* unable to close */
		ZExit(EXIT_FAILURE);
	}
	if (OFiles[OfIndx].OTNam[0] != '\0') {	/* if temporary output file */
        if (OFiles[OfIndx].forBackup) {
		(void)strcpy(TmpFsp, OFiles[OfIndx].OTNam);/* copy to TmpFsp */
		if ((EzFlag&EZ_NO_STRIP)==0) {
			char *DotPtr = strchr(TmpFsp, '.');	/* find the "." */
			if (DotPtr != NULL) {		/* if "." exists */
				*DotPtr = '\0';			/* make it null */
			}
		}
		if (EzFlag&EZ_NO_VER) {
			(void)strcat(TmpFsp, ".bak");		/* append ".bak" */
			if (access(TmpFsp, 0) == 0) {		/* old "x.bak"? */
#if DEBUGGING
				sprintf(DbgSBf,
						"deleting old version of %s\r\n",
						TmpFsp);
				DbgFMs(2,DbgFNm,DbgSBf);
#endif
				if (remove(TmpFsp) != 0) {	/* delete it */
					ZErMsg();
					ErrMsg(ERR_UCO);
					ZClnUp();
					exit(EXIT_FAILURE);
				}
			}
		} else {
			ver = vernum(TmpFsp);
			if (ver==(-3)) {
				puts("\nWARNING: Versioning disabled\n");
				(void)strcat(TmpFsp, ".bak");	/* append ".bak" */
				if (access(TmpFsp, 0) == 0) {	/* old "x.bak"? */
#if DEBUGGING
					sprintf(DbgSBf,
							"deleting old version of %s\r\n",
							TmpFsp);
					DbgFMs(2,DbgFNm,DbgSBf);
#endif
					if (remove(TmpFsp) != 0) {  /* delete it */
						ZErMsg();
						ErrMsg(ERR_UCO);
						ZClnUp();
						exit(EXIT_FAILURE);
					}
				}
			} else if (ver==(-2) || ver==0) {
				(void)strcat(TmpFsp, ";1");
			} else if (ver==(-1)) {			/* can't read dir */
				ZErMsg();
				ErrMsg(ERR_UCO);		/* una to close o */
				ZClnUp();
				exit(EXIT_FAILURE);
			} else {				/* ver > 0 */
				int ln = strlen(TmpFsp);
				ver++;
				strcat(TmpFsp, ";");
				MakDBf((LONG)ver, 10);
				strncat(TmpFsp, (char *)DBfBeg, DBfPtr-DBfBeg);
				*(TmpFsp+ln+(1+DBfPtr-DBfBeg)+1) = '\0';
			}
		}
#if DEBUGGING
		sprintf(DbgSBf,"renaming %s to %s\r\n",
				OFiles[OfIndx].OTNam, TmpFsp);
		DbgFMs(2,DbgFNm,DbgSBf);
#endif
		if (rename(OFiles[OfIndx].OTNam, TmpFsp)) { /* TAA changed to use rename */
			ZErMsg();
			ZDspBf((unsigned char *)"Edit saved in ", 14);
			ZDspBf(OFiles[OfIndx].OFNam,
				   strlen(OFiles[OfIndx].OFNam));
			ErrMsg(ERR_UCO);
			ZClnUp();
			exit(EXIT_FAILURE);
		}
		} else { /* Delete original if not backing up */
			if (remove(OFiles[OfIndx].OTNam) != 0) {
				ZErMsg();
				ErrMsg(ERR_UCO);
				DBGFEX(2,DbgFNm,"remove() failed");
				exit(EXIT_FAILURE);
			}
		}
#if DEBUGGING
		sprintf(DbgSBf,"renaming %s to %s\r\n",
				OFiles[OfIndx].OFNam, OFiles[OfIndx].OTNam);
		DbgFMs(2,DbgFNm,DbgSBf);
#endif
		if (rename(OFiles[OfIndx].OFNam,
					 OFiles[OfIndx].OTNam)) {
			ZErMsg();
			ErrMsg(ERR_UCO);
			ZClnUp();
			exit(EXIT_FAILURE);
		}
	}
	IsOpnO[OfIndx] = FALSE;
	DBGFEX(2,DbgFNm,NULL);
}
/*****************************************************************************
	ZOpInp()
	This function opens an input file.  The name of the file is pointed
to by FBfBeg.  FBfPtr points to the character following the last character of
the file name.
	This function is used to open all files,  including macro files
needed by the "EI" command.  The "EIFlag" argument tells this function if
it's an "EI" file.  If it is,  some extra file searching is done to make
things convenient for the user.  The extra processing is modelled after what
happens under VMS (or really,  what SHOULD happen under VMS).  The basic idea
is to find the macro file whether the user has specificed the ".tec" or not,
and whether it's in the current directory or the macro library directory.
The basic Unix logic is like this:
	if (the file exists)
	    open it and return SUCCESS
	if (EIfile) {
	    if (there's no dot and appending ".tec" works)
		open it and return SUCCESS
	    if (prepending default library directory works)
		open it and return SUCCESS
	    if (prepending library and appending ".tec" works)
		open it and return SUCCESS
	}
	file not found, so return with error
Under VAX/VMS,  it's a little different.  VMS tries to open the file only
twice,  each time with the RMS "default type" field set to ".TEC",  so VMS
will insert ".TEC" if the user doesn't.  There's no straightforward way to
avoid putting ".TEC" on the end of your TECO macro file namess under VMS,
which some would argue is a good thing,  as long as you don't have to type
the ".TEC" when you use them.
Under MS-DOS,  the above PDL works,  except that when the logic talks about
appending ".tec",  it doesn't happen if there's alreay a dot in the file
name,  as you can only have one dot in MS-DOS file names.
*****************************************************************************/
DEFAULT ZOpInp(IfIndx, EIFile, RepFNF)
DEFAULT IfIndx;			/* index into file data block array IFiles */
BOOLEAN	EIFile;			/* is it a macro file (hunt for it) */
BOOLEAN RepFNF;			/* report "file not found" error? */
{
#if DEBUGGING
    static char *DbgFNm = "ZOpInp";
    sprintf(DbgSBf,", FBf = \"%.*s\"", (int)(FBfPtr-FBfBeg), FBfBeg);
    DbgFEn(2,DbgFNm,DbgSBf);
#endif
    *FBfPtr = '\0';			/* terminate the file name */
    if ((IFiles[IfIndx] = fopen((char *)FBfBeg, "r")) != NULL) {
	DBGFEX(1,DbgFNm,"SUCCESS");
	IFisCR[IfIndx] = 0;
	return SUCCESS;
    }
    if (EIFile) {
	charptr dummyp = NULL;
	char TmpBfr[FILENAME_MAX];
	ptrdiff_t TmpLen = FBfPtr-FBfBeg;
	BOOLEAN noDot;
	if (noDot = strchr((char *)FBfBeg,'.') == NULL) {	/* if no dot */
	    (void)strcat(FBfBeg,".tec");		/* append .tec */
	    FBfPtr += 4;
	    if ((IFiles[IfIndx] = fopen(FBfBeg, "r")) != NULL) {
		DBGFEX(1,DbgFNm,"SUCCESS");
		IFisCR[IfIndx] = 0;
		return SUCCESS;
	      }
	}
	MEMMOVE(TmpBfr, FBfBeg, TmpLen);		/* save file name */
	if (ZClnEG(EG_LIB, GET_VAL, dummyp) != -1) {	/* get dir spec */
	  goto open_failed;
	}
	MEMMOVE(FBfPtr, TmpBfr, TmpLen);	/* append name to dir spec */
	FBfPtr += TmpLen;
	*FBfPtr = '\0';				/* terminate file name */
	if ((IFiles[IfIndx] = fopen(FBfBeg, "r")) != NULL) {
	    DBGFEX(1,DbgFNm,"SUCCESS");
	    IFisCR[IfIndx] = 0;
	    return SUCCESS;
	}
	if (noDot) {					/* if no dot */
	    (void)strcat(FBfBeg,".tec");		/* append .tec */
	    FBfPtr += 4;
	    if ((IFiles[IfIndx] = fopen(FBfBeg, "r")) != NULL) {
		IFisCR[IfIndx] = 0;
		DBGFEX(1,DbgFNm,"SUCCESS");
		return SUCCESS;
	      }
	}
    }
/*
 * The fopen failed,  so return with error.
 */
open_failed:
    if (!RepFNF && ((errno == ENODEV) || (errno == ENOENT))) {
	DBGFEX(2,DbgFNm,"FILENF");
	return FILENF;
    }
    ZErMsg();
    DBGFEX(2,DbgFNm,"FAILURE");
    return FAILURE;
}
/*****************************************************************************
	ZOpOut()
	This function creates (and opens) an output file.  The name of
the file to be created is pointed to by FBfBeg.  FBfPtr points to the
character following the last character of the file name.
*****************************************************************************/
/*
 * Unix file names do not have version numbers,  so we have to deal with
 * creating ".bak" versions of files.  For output files,  this means that
 * when the output file is opened,  we check if a file with the same name
 * already exists.  If a file already exists,  then we open a temporary
 * output file and,  when the file is closed,  the close routine will deal
 * with renaming files to make them come out right.  If no file with the
 * same name already exists,  then the output file can simply be opened.
 * The close routine will only rename files if a temporary file was created
 * by this routine.
 */
DEFAULT ZOpOut(OfIndx, RepErr, Backup)		/* open output file */
DEFAULT	OfIndx;				/* output file indicator */
BOOLEAN RepErr;				/* report errors? */
BOOLEAN Backup;				/* TAA Added */
{
	char *tfname;
	struct stat *bufstat = NULL;
	int tmpflag = 0;
#if DEBUGGING
	static char *DbgFNm = "ZOpOut";
	sprintf(DbgSBf,", FBf = \"%.*s\"",(int)(FBfPtr-FBfBeg),FBfBeg);
	DbgFEn(2,DbgFNm,DbgSBf);
#endif
/*
 * If the output file already exists,  make a temporary file.
 */
	*FBfPtr = '\0';
	if (access(FBfBeg, 0) == 0) {		/* if file already exists */
		char *dirname,*s;
	 	int n;
		bufstat = (struct stat *)malloc(sizeof(struct stat));
		stat(FBfBeg, bufstat);
		tmpflag = 1;
		n = FBfPtr - FBfBeg;		/* strlen(FBfBeg) */
		dirname = (char *)malloc(n+4);
		strcpy(dirname, FBfBeg);
		s=dirname + n;
		while (*s != '/' && s > dirname)
			s--;
		if (*s=='/' && s==dirname) {	/* must be root directory */
			*(s+1)='\0';
		} else if (*s=='/')  {		/* we have string/name */
			*s='\0';
		} else {			/* must have s==dirname and
						   *s != '/', so current
						   directory */
			*dirname='.'; *(dirname+1)='\0';
		}
		tfname = tempnam(dirname,"tecoc");
		(void)strcpy(OFiles[OfIndx].OFNam, tfname);
		free(tfname);
		free(dirname);
		(void)strcpy(OFiles[OfIndx].OTNam, FBfBeg);
#if DEBUGGING
		sprintf(DbgSBf,	"file %s already exists\r\n", FBfBeg);
		DbgFMs(2,DbgFNm,DbgSBf);
#endif
	} else {
		(void)strcpy(OFiles[OfIndx].OFNam, FBfBeg);
		OFiles[OfIndx].OTNam[0] = '\0';
	}
#if DEBUGGING
	sprintf(DbgSBf,	"creating file %s\r\n",	OFiles[OfIndx].OFNam);
	DbgFMs(2,DbgFNm,DbgSBf);
#endif
	OFiles[OfIndx].OStrem = fopen(OFiles[OfIndx].OFNam, "w");
	if (OFiles[OfIndx].OStrem == NULL) {
		if (RepErr)
			ZErMsg();
		DBGFEX(2,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (tmpflag) {
		fchmod(fileno(OFiles[OfIndx].OStrem),
		       bufstat->st_mode | S_IRUSR | S_IWUSR);
		free(bufstat);
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
VVOID ZPrsCL(argc, argv)		/* parse a TECOC command line */
int argc;
char *argv[];
{
	int i;
	char TmpBuf[256];
	SIZE_T	line_len;
	char *cmd;
	DBGFEN(2,"ZPrsCL",NULL);
/*
 * If the command line contains arguments,  construct a replica of the
 * command line in Q-register Z.  It's a "replica" because spacing might
 * be wrong.
 */
	TmpBuf[0] = '\0';
	cmd = strrchr(argv[0], '/');
	if (cmd) {
		cmd++;
	} else {
		cmd = argv[0];
	}
	if (strcmp(cmd, "teco") == 0 ||
		  strcmp(cmd, "mung") == 0) {
		strcat(TmpBuf, cmd);
		strcat(TmpBuf, " ");
	} else if (strcmp(cmd, "inspect") == 0) {
		strcat(TmpBuf, "teco -inspect ");
	} else if (strcmp(cmd, "Make") == 0) {
		strcat(TmpBuf, "make ");
	}
	if (argc > 1 || TmpBuf[0] != '\0') {
		for (i=1; i<argc; i++) {
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
		}
		line_len = strlen(TmpBuf)-1;	/* remove trailing space */
		QR = &QRgstr[35];		/* 35 = q-register Z */
		if (MakRom(line_len) == FAILURE) {
			DBGFEX(2,DbgFNm,"exiting with EXIT_FAILURE");
			ZExit(EXIT_FAILURE);
		}
		MEMMOVE(QR->Start, TmpBuf, line_len);
		QR->End_P1 += line_len;		/* length of q-reg text */
	}
#if USE_ANSI_CLPARS
/*
 * execute imbedded command line-parsing macro directly from clpars[]
 */
	CStBeg = clpars;		/* command string start */
	CBfPtr = clpars;		/* command string start */
	CStEnd = clpars + CLPARS_LEN;		/* command string end */
	EStTop = EStBot;			/* clear expression stack */
	ExeCSt();				/* execute command string */
#else
/*
 * Load imbedded command-line parsing macro into Q-register Y
 */
	QR = &QRgstr[34];			/* 34 = q-register Y */
	if (MakRom((SIZE_T)CLPARS_LEN) == FAILURE) {
		DBGFEX(2,DbgFNm,"MakRom(CLPARS_LEN) failed, calling exit()");
		ZExit(EXIT_FAILURE);
	}
	for (i = 0; i < CLPARS_LINES; i++) {
		line_len = strlen(clpars[i]);
		MEMMOVE(QR->End_P1, clpars[i], line_len);
		QR->End_P1 += line_len;		/* length of q-reg text */
	}
/*
 * Execute an MY$$ command.
 */
	CBfPtr = (charptr)"my\33\33";		/* command string start */
	CStEnd = CBfPtr + 3;			/* command string end */
	EStTop = EStBot;			/* clear expression stack */
	ExeCSt();				/* execute command string */
/*
 * Clear the command-line parsing macro from Q-register Y
 */
	QR = &QRgstr[34];			/* 34 = q-register Y */
	ZFree (QR->Start);
	QR->Start = QR->End_P1 = NULL;
#endif
	DBGFEX(2,DbgFNm,NULL);
}
/*****************************************************************************
	ZPWild()
	This function presets the wildcard lookup filename.  It is called
when the user executes an ENfilename$ command.  Later executions of the EN$
command will cause the ZSWild function to be called to return successive
wildcard matches.
*****************************************************************************/
/* Rewritten 05/04 by TAA */
DEFAULT ZPWild()		/* preset the wildcard lookup filename */
{
	int result,i;
/*
 * if we didn't process all names from a previous EN` call then discard them
 */
	globfree(&pglob);
	globindex = 0;
	*FBfPtr='\0';				/* terminate string */
	result = glob(FBfBeg, GLOB_ERR|GLOB_MARK, NULL, &pglob);
	if (result != 0) {
		return FAILURE;
	}
	result = 0; /* Count files matched */
	for (i = 0; i < pglob.gl_pathc; i++) {
		if (pglob.gl_pathv[i][strlen(pglob.gl_pathv[i])-1] != '/') {
			result++;
		}
	}
	if (result > 0) {
		return SUCCESS;
	}
	return FAILURE;
}
/*****************************************************************************
	ZRaloc()
	This function performs the standard C library function realloc.
*****************************************************************************/
voidptr ZRaloc(OldBlk, NewSiz)		/* re-allocate memory */
voidptr OldBlk;
SIZE_T NewSiz;
{
	return (voidptr)realloc(OldBlk, NewSiz);
}
/*****************************************************************************
	ZRdLin()
	This function reads a line from a file.  It is passed a buffer, the
size of the buffer, and a file pointer.  It returns the length of the line,
or sets IsEofI[] to TRUE if the end of file is encountered.
*****************************************************************************/
DEFAULT ZRdLin(ibuf, ibuflen, IfIndx, retlen)
charptr ibuf;			/* where to put string */
ptrdiff_t ibuflen;		/* max length of ibuf */
int IfIndx;			/* index into IFiles[] */
DEFAULT *retlen;		/* returned length of string */
{
	int character;		/* the last character read */
	DEFAULT shortBuf;	/* max size to read, 32767 or ibuflen */
	DEFAULT charsLeft;	/* number of characters left */
	FILE *fp;		/* input stream pointer to read from */
	char *iBuf;		/* non-huge pointer into IBf for speed */
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
	while ((character = getc(fp)) >= 0) {	 /* we got one */
		*iBuf++ = character;
		if (character == LINEFD) {
			*retlen = shortBuf - charsLeft + 1;
			if ((EzFlag & EZ_UNIXNL) == 0 && IFisCR[IfIndx] == 0) {
				ibuf[(*retlen) -1] = CRETRN;
				ibuf[(*retlen)] = LINEFD;
				(*retlen)++;
			}
			IFisCR[IfIndx] = 0;
#if DEBUGGING
			sprintf(DbgSBf,"SUCCESS, retlen = %d", *retlen);
			DbgFEx(3,DbgFNm,DbgSBf);
#endif
			return SUCCESS;
		}
		IFisCR[IfIndx] = character == '\r'; /* last character was a CR -- DOS file? */
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
		/* TAA Mod 5/04 -- must allow for extra character if
		 * UNIXNL since LF becomes CRLF */
		if (--charsLeft == ((EzFlag&EZ_UNIXNL) ? 0 : 1)) {
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
	if (ferror(fp)) {			/* if we got an error */
		*retlen = 0;			/* say didn't read anything */
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
		IsEofI[IfIndx] = TRUE;		/* say we reached EOF */
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
VVOID ZScrOp(OpCode)		/* do a screen operation */
int OpCode;			/* code for operation */
{
#if CURSES
	int x,y;
	switch (OpCode) {
		case SCR_EEL:	clrtoeol();
				break;
		case SCR_CUP:	getyx(stdscr,y,x);
				if (y-1)
					move(y-1,x);
				break;
		case SCR_ROF:	standend(); break;
		case SCR_RON:	standout(); break;
	}
	refresh();
#endif
	
// 	if (CrType == UNTERM || tbuf[0] == 0) {/* if unknown terminal type */
// 		return;			/* can't do screen operations */
// 	}
// 
// 	switch (OpCode) {
// 		case SCR_CUP:	tputs(up, 1, ZDspCh);
// 				break;
// 		case SCR_EEL:	tputs(ce, 1, ZDspCh);
// 				break;
// 		case SCR_ROF:	tputs(se, 1, ZDspCh);
// 				break;
// 		case SCR_RON:	tputs(so, 1, ZDspCh);
// 	}
}
/*****************************************************************************
	ZSetTT()
	This function sets or clears terminal parameters.  The only terminal
parameters that TECO can set are
	1. whether the terminal can display 8-bit characters
	2. the number of rows
	3. the number of columns
*****************************************************************************/
DEFAULT ZSetTT(TTWhat, TTVal)	/* tell operating system we set the term. */
DEFAULT TTWhat;		/* what terminal parameter to set */
DEFAULT TTVal;		/* what to set it to */
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
/* Rewritten 05/04 by TAA */
DEFAULT ZSWild()		/* search for next wildcard filename */
{
	int filename_length;
	if (pglob.gl_pathc == 0)
		return FAILURE;
	do {
		if (globindex >= pglob.gl_pathc) return FILENF;
		if (pglob.gl_pathv[globindex][strlen(pglob.gl_pathv[globindex]-1)] != '/') break;
		globindex++;
	} while (1);
/*
 * we really have a file name now
 */
	filename_length = strlen(pglob.gl_pathv[globindex]);
	MEMMOVE(FBfBeg, pglob.gl_pathv[globindex], filename_length);
	FBfPtr = FBfBeg + filename_length;
	globindex++;
	return SUCCESS;
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
	When TECO is started, the terminal will probably be set up for
buffered I/O,  so characters won't be received until a RETURN is hit,  and
they will be automatically echoed.  Set the terminal up for raw I/O,  so each
character is received when it is struck, and no echoing is performed.  Save
the terminal characteristics so when we exit we can reset them (in ZClnUp)
to what they were before we changed them.
*****************************************************************************/
static VVOID CntrlC()
{
    signal(SIGINT, SIG_IGN);
    SupGotCtC = 0;
    if (EtFlag & ET_TRAP_CTRL_C) {		/* if user wants it */
	EtFlag &= ~ET_TRAP_CTRL_C;		/* turn off bit */
	SupGotCtC = 1;
    } else {					/* user doesn't want it */
	if (EtFlag & ET_MUNG_MODE) {		/* if in MUNG mode */
	    TAbort(EXIT_SUCCESS);
	}
	GotCtC = TRUE;				/* set "stop soon" flag */
    }
    signal(SIGINT, CntrlC);
}
/*
 * sighup - what we do if we get a hang up?
 */
static void sighup()
{
	TAbort(EXIT_FAILURE);
}
/*
 * sigstop - what to do if we get a ^Z
 */
static void sigstop()
{
	tcsetattr(0, TCSANOW, &out);
	puts("[Suspending...]\r\n");
	kill(getpid(), SIGSTOP);
	puts("[Resuming...]\r\n");
	tcsetattr(0, TCSANOW, &cur);
}
/*
 * ZTrmnl - set up terminal modes
 */
VVOID ZTrmnl()			/* set up I/O to the terminal */
{
	EtFlag = ET_READ_LOWER |	/* guess: term has lowercase and */
#if CURSES
		 ET_WAT_SCOPE |		/* "WATCH" support exists */
#endif
		 ET_EIGHTBIT |          /* terminal uses 8-bit characters */
     		 ET_SCOPE;		/* it's a scope, not hardcopy */
	EzFlag = EZ_NO_VER |		/* don't do VMS-style file versions */
#if CURSES
/*		 EZ_UNIXNL |		* don't add CRs to newlines */
#endif
	         EZ_INVCR;		/* don't show little c/r characters */
/*
 * get terminal characteristics and set some signals
 */
#if !CURSES
	if (tcgetattr(0,  &out) != -1)
		tty_set = TRUE;		/* tell ZClnUp to clean up */
	tcgetattr(0, &cur);
#ifdef SIGTSTP
	signal(SIGTSTP, sigstop);	/* call sigstop on stop (control-Z) */
#endif
#endif
/*
 * set CBREAK/noECHO/noCRMOD
 */
#if CURSES
	initscr();			/* initialize screens */
#ifdef ULTRIX
	setupterm(0,1,0);		/* Enable termcap compatibility */
#endif
	scrollok(stdscr,TRUE);		/* scroll if cursor moves too hi/lo */
	idlok(stdscr,TRUE);		/* use hardware insert/delete line */
	cbreak();			/* don't wait for CRs (CBREAK) */
	noecho();			/* don't echo characters (ECHO) */
	nonl();				/* don't add CRs to NEWLINEs (CRMOD) */
	curwin = stdscr;
	ScopeFlg = 0;			/* not writing in scope */
#else  /* NOT CURSES */
	cur.c_lflag &= ~ICANON;
	cur.c_lflag &= ~ECHO;
	cur.c_oflag &= ~ONLCR;
	cur.c_iflag &= ~(ICRNL | INLCR);
	tcsetattr(0, TCSANOW, &cur);
#endif
	signal(SIGINT, CntrlC);		/* call CntrlC on interrupt */
	signal(SIGHUP, sighup);		/* call sighup on hang up */
	siginterrupt(SIGINT, 1);
/*
 * set up termcap stuff
 */
	CrType = VT102;			/* Let's pretend we are a VT102
					   even though we are really using
					   termcap! */
#if !CURSES
	tbuf[0] = 0;
#ifdef LINUX // BAD for now but do we really want this??
#else
	if ((ta = getenv("TERM")) == NULL) {	/* get terminal type */
		ta = "dumb";
	}
	tgetent(tbuf, ta);		/* tbuf gets terminal description */
	ta = tarea;
	ce = xtgetstr("ce",&ta);	/* clear to end of line */
	se = xtgetstr("se",&ta);	/* end standout mode (rev. video) */
	so = xtgetstr("so",&ta);	/* begin standout mode */
	up = xtgetstr("up",&ta);	/* cursor up */
#endif
#endif
#if VIDEO
#if CURSES
#ifdef ULTRIX
	VtSize = LINES;
	HtSize = COLS;
#else
	getmaxyx(stdscr, VtSize, HtSize);
#endif
#else
	VtSize = HtSize = 0;
#endif
#endif
}
/*****************************************************************************
	ZVrbos()
	This function displays the verbose form of an error message.
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
	This function writes a buffer to a file,  one line at a time.  It is
passed an output file index and pointers to the beginning and end of the
buffer to be output.
*****************************************************************************/
DEFAULT ZWrBfr(OfIndx, BfrBeg, BfrEnd)
DEFAULT OfIndx;			/* index into OFiles array */
charptr BfrBeg;			/* address of output buffer beginning */
charptr BfrEnd;			/* address of output buffer end */
{
#if DEBUGGING
	static char *DbgFNm = "ZWrBfr";
	sprintf(DbgSBf,"OfIndx = %d, BfrBeg = %ld, BfrEnd = %ld",
		OfIndx, Zcp2ul(BfrBeg), Zcp2ul(BfrEnd));
	DbgFEn(2,DbgFNm,DbgSBf);
#endif
/*
 * If we're doing Unix-style editing,  where lines are terminated with a
 * line feed (newline) instead of a carriage-return/line-feed pair,  then
 * we can just fwrite the buffer.  Otherwise,  we have to scan the buffer
 * and convert CR/LF pairs to just LF before output.
 */
	if (EzFlag & EZ_UNIXNL) {
	    ptrdiff_t bufsiz = BfrEnd-BfrBeg+1;
	    if (fwrite(BfrBeg, sizeof(char), bufsiz,
		       OFiles[OfIndx].OStrem) != bufsiz) {
		ZErMsg();
		ErrMsg(ERR_UWL);
		DBGFEX(2,DbgFNm,"Zfwrite() failed");
		return FAILURE;
	    }
	} else {
	    charptr BfrPtr = BfrBeg;		/* output buffer pointer */
	    while (BfrPtr <= BfrEnd) {
		if (*BfrPtr == CRETRN) {
		    BfrPtr++;
		    if ((BfrPtr > BfrEnd) || (*BfrPtr != LINEFD)) {
			BfrPtr--;
		    }
		}
		if (putc(*BfrPtr, OFiles[OfIndx].OStrem) == EOF) {
		    ZErMsg();
		    ErrMsg(ERR_UWL);
		    DBGFEX(2,DbgFNm,"FAILURE");
		    return FAILURE;
		}
		++BfrPtr;
	    }
	}
	DBGFEX(2,DbgFNm,"SUCCESS");
	return SUCCESS;
}
/****************************************************************************
	vernum()
	From Mark Henderson,  was in a separate file named vernum.c.
*****************************************************************************/
static int vernum(target)
/*
maximum version number of target (similar to VMS mechanism)
return
-3	error - problems other than file not found and can't open directory
-2	error - did not find file
-1	error - cannot open directory containing target
 0	file found - no version numbers found
 n > 0	version number of highest name;n
*/
char *target;
{
	DIR            *dirp;
	int found = 0;	/* file found flag */
	char *ftarget;
	int maxver = 0;
	char *dirname;
	struct dirent  *dp;
	int n;
	char *s;
	n = strlen(target);
	dirname=(char *)malloc(strlen(target)+4);
	ftarget=(char *)malloc(strlen(target)+4);
	strcpy(dirname, target);
	s=dirname + n;
	while (*s != '/' && s > dirname)
		s--;
	if (*s=='/' && s==dirname) { /* must be root directory */
		strcpy(ftarget,s+1);
		*(s+1)='\0';
	} else if (*s=='/')  { /* we have string/name */
		strcpy(ftarget,s+1);
		*s='\0';
	} else { /* must have s==dirname and *s!='/', so current directory */
		strcpy(ftarget,target);
		*dirname='.'; *(dirname+1)='\0';
	}
	dirp = opendir(dirname);
	if (dirp == NULL) {
		ZDspBf("\nerror openning directory ", 26);
		ZDspBf(dirname, strlen(dirname));
		ZDspCh('\n');
		free(dirname);
		free(ftarget);
		return(-1);
	}
	n=strlen(ftarget);
	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		if (strncmp(dp->d_name,ftarget,n) == 0) {
			char *suffix = dp->d_name + n;
			if (*suffix == '\0') {
				found = 1;
				if (maxver <= 0)
					maxver = 0;
			}
			if (*suffix == ';') {
				int k;
				found = 1;
				k=atoi(suffix+1);
				if (k<=0) {
					free(dirname);
					free(ftarget);
					closedir(dirp);
					return(-3);
				}
				if (k>=maxver)
					maxver = k;
			}
		}
	}
	free(dirname);
	free(ftarget);
	closedir(dirp);
	return (found) ? maxver : -2;
}
#if CURSES
static void movedot(HowFar)
LONG HowFar;			/* positive or negative displacement */
{
    if (HowFar > 0) {
	if ((GapEnd+HowFar) > EBfEnd) {
	    HowFar = EBfEnd - GapEnd;
	}
	MEMMOVE(GapBeg, GapEnd+1, HowFar);
	GapBeg += HowFar;
	GapEnd += HowFar;
    } else {
	if ((GapBeg+HowFar) < EBfBeg) {
	    HowFar = EBfBeg - GapBeg;
	}
	GapBeg += HowFar;
	GapEnd += HowFar;
	MEMMOVE(GapEnd+1, GapBeg, -(HowFar));
    }
}
static void
drawline()
{
    int i;
    if (winline) {
	move(VtSize - ScroLn - 1, 0);
	for (i=0;i<HtSize; i++)
	    addch(ACS_HLINE);
    }
}
static void
initialize_scope()
{
/*
 * not dotx, doty will be character and line offsets from 0,0
 * wrap will not effect these
 */
    dotx = doty = 0;
    olddot = 0;
    getyx(stdscr,cmdy,cmdx);
    move(0,0);
    clear();
    p_scope_start=GapBeg;
    scope_start = DOT;
    if (winline)
	drawline();
    move(cmdy,cmdx);
    refresh();
}
static void
finish()
{
    int x;
    int y;
    getyx(stdscr,y,x);
    if (y >= VtSize - winline - ScroLn)
	return;
    clrtoeol();
    for (x=y+1;x<VtSize - winline - ScroLn; x++) {
	move(x,0);
	clrtoeol();
    }
}
void
redraw()
{
	int cccmdx;
	int cccmdy;
	int flag = 0;
	int x;
	int y;
	int charcounter;
	unsigned char *curptr;
	if (ScroLn == 0)
		return;
	getyx(stdscr,cccmdy,cccmdx);
draw_all:
	if (EBfBeg == GapBeg && EBfEnd==GapEnd) {
		int i;
		move(0,0);
		scope_start = 0;
		p_scope_start = EBfBeg;
		standout();
		specon();
		getyx(stdscr, doty, dotx);
		addch((EzFlag & EZ_BTEE) ? ACS_BTEE : ACS_DIAMOND);
		specoff();
		standend();
		getyx(stdscr, doty1, dotx1);
		clrtoeol();
		for (i=1; i<=VtSize - ScroLn - 1 - winline;i++) {
			move(i,0);
			clrtoeol();
		}
		drawline();
		move(cccmdy,cccmdx);
		olddot = 0;
		return;
	}
	if ((intabs(olddot - DOT) > HtSize * (VtSize - ScroLn))
		|| HldFlg) {
		/* admittedly a heuristic ! */
		int linedisp, disp;
		int siz = VtSize - ScroLn - 1 - winline;
		linedisp = siz/2-2;
		if (linedisp > 0) {
			disp = Ln2Chr(-linedisp);
			p_scope_start = GapBeg + disp;
			scope_start = DOT + disp;
			goto l1; /* no need to do stuff to get to beginning
				  of line */
		}
	}
	if (p_scope_start > EBfBeg) {
	/* set p_scope_start to beginning of line */
	if (scope_start > DOT) { /* move */
		p_scope_start = GapBeg;
		scope_start = DOT;
	}
	flag = 0;
	while ((p_scope_start > EBfBeg) && !flag) {
		--p_scope_start;
		if (IsEOL(*p_scope_start))
			flag++;
	}
	if (p_scope_start != EBfBeg || IsEOL(*p_scope_start))
		++p_scope_start;
	}
/*
 * note here p_scope_start may still be at GapBeg after all this number of
 * characters we "backed up" is GapBeg - p_scope_start
 */
l1:	olddot = DOT; move(0,0);
	charcounter = GapBeg - p_scope_start;
	if (p_scope_start < GapBeg) {
		for (curptr=p_scope_start; curptr < GapBeg; curptr++) {
			int redrawflag;
			zaddch2(*curptr);
			getyx(stdscr,y,x);
			if (y>VtSize - ScroLn - 1 - winline) {
				int lines = 0;
				unsigned char *charpointer;
/*
 * we didn't get to print dot - try moving p_scope_start one line forward or
 * if lots of lines in between DOT and p_scope_start then just redraw
 */
				move(y-1,0);
				clrtoeol();
				redrawflag = 0;
				if (p_scope_start >= EBfEnd) {
					finish();
					drawline();
					move(cccmdy,cccmdx);
					return; /* blew it */
				}
				/* about how many lines ? */
				for (charpointer = p_scope_start;
				charpointer < GapBeg; charpointer++) {
					if (IsEOL(*charpointer))
						lines++;
					if (lines > 2*(VtSize - ScroLn))
						break;
				}
				if (lines > 2*(VtSize-ScroLn)) {
					/* just redraw everything */
					p_scope_start = GapBeg;
					scope_start = DOT;
					move(0,0);
					/* clear(); */
					p_scope_start = GapBeg;
					scope_start = DOT;
					goto draw_all;
				}
				/* move forward a line */
				while (p_scope_start < GapBeg && !IsEOL(*p_scope_start)) {
					++p_scope_start;
					++scope_start;
				}
				if (IsEOL(*p_scope_start)) {
					p_scope_start++; scope_start++;
				}
				if (p_scope_start >= GapBeg) {
					p_scope_start = GapBeg;
					scope_start = DOT;
				}
				goto l1;
			}
		}
	}
	curptr = GapEnd+1;
	standout();
	getyx(stdscr, doty, dotx);
	if (curptr > EBfEnd) {
		getyx(stdscr,y,x);
		if (y <= VtSize - ScroLn - 1 - winline) {
			specon();
			addch((EzFlag & EZ_BTEE) ? ACS_BTEE : ACS_DIAMOND);
			specoff();
			standend();
			getyx(stdscr, doty1, dotx1);
		}
		finish();
		drawline();
		move(cccmdy,cccmdx);
		clrtoeol();
		return;
	} else {
		if (*curptr=='\n') {
		if (!(SeeAll) && (y <= VtSize - ScroLn - 1 - winline)) {
			if (EzFlag & EZ_VT100GRAPHICS) {
				gr_on(); waddch(curwin,'e'); gr_off();
			} else
				addch(ACS_RTEE);
		}
		}
		zaddch2(*curptr);
	}
	standend();
	getyx(stdscr, doty1, dotx1);
	curptr++;
	getyx(stdscr,y,x);
	if (y>VtSize - ScroLn - 1 - winline) {
		drawline();
		move(cccmdy,cccmdx);
		return;
	}
	while ((curptr <= EBfEnd)) {
		zaddch2(*curptr);
		getyx(stdscr,y,x);
		if (y>=VtSize - ScroLn - winline)
			break;
		++curptr;
	}
	if (y < VtSize - ScroLn - winline && curptr > EBfEnd) {
		specon();
		addch((EzFlag & EZ_BTEE) ? ACS_BTEE : ACS_DIAMOND);
		specoff();
	}
	finish();
	drawline();
	move(cccmdy,cccmdx);
}
void
centre()
{
	int linedisp, disp;
	int siz = VtSize - ScroLn - 1 - winline;
	linedisp = siz/2-2;
	if (linedisp > 0) {
		disp = Ln2Chr(-linedisp);	/* disp <= 0 */
		p_scope_start = GapBeg + disp;
		scope_start = DOT + disp;
	} else {
		p_scope_start = GapBeg;
		scope_start = DOT;
	}
	redraw();
}
void
dolf()
{
    if (!skiprefresh)
	redraw();
}
void
dobs()
{
    if (!skiprefresh)
	redraw();
}
void
ccs()
{
    colsave = (-1);
}
void
do_right()
{
    colsave = (-1);
    CmdMod &= ~COLON;
    if (GapEnd < EBfEnd) {
	DoCJR(1);
	redraw();
	refresh();
    }
}
void
do_left()
{
    colsave = (-1);
    CmdMod &= ~COLON;
    if (EBfBeg < GapBeg) {
	DoCJR(-1);
	redraw();
	refresh();
    }
}
void
do_sf()
{
}
void
do_sr()
{
}
void
do_up()
{
    int disp = Ln2Chr(-1); int ll; int dist;
    if (colsave < 0) {
	colsave = -Ln2Chr(0);
    }
    CmdMod &= ~COLON;
/*
 * find length of prev line in characters
 */
    ll = Ln2Chr(0) - Ln2Chr(-1) - 1;
    if (ll==(-1))
	return;
    dist = disp + ((colsave >= ll) ? ll : colsave);
    movedot(dist);
    redraw();
    refresh();
}
void
do_down()
{
    int disp = Ln2Chr(1);
    int ll, dist;
    if (colsave < 0) {
	colsave = -Ln2Chr(0);
    }
    CmdMod &= ~COLON;
    ll = Ln2Chr(2) - Ln2Chr(1) - 1;
    if (ll==(-1)) {
	movedot(EBfEnd - GapEnd);
    } else {
	dist = disp + ((colsave >= ll) ? ll : colsave);
	movedot(dist);
    }
    redraw();
    refresh();
}
void
do_seetog()		/* toggle SeeAll mode */
{
    SeeAll = (SeeAll > 0) ? 0 : 1;
    redraw();
    refresh();
}
void
keypad_on()
{
    keypad(stdscr, (KeyPad & 1));
    keypad(stdscr, (KeyPad & 2));
}
void
keypad_off()
{
#ifndef ULTRIX
    notimeout(stdscr, FALSE);
#endif
    keypad(stdscr, FALSE);
}
void
Scope(bot)	/* first crack */
int bot;
{
	if (bot <0)
		return;
	if (bot == 0) {
		setscrreg(0, VtSize -1);
	} else {
		winline = (EzFlag & EZ_WIN_LINE) ? 1 : 0;
		setscrreg(VtSize - bot, VtSize - 1);
		move(VtSize - bot, 0);
		curwin = stdscr;
		initialize_scope();
	}
}
#endif
