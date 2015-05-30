/*****************************************************************************
	ZUnkn.c
	System dependent code for an unknown compiler, computer, or
operating system.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "dchars.h"		/* define identifiers for characters */
#include "clpars.h"		/* command-line parsing macro */
VVOID exit();
VVOID free();
char *malloc();
char *realloc();
char *strcat();
char *strchr();
char *strcpy();
DEFAULT strlen();
/*****************************************************************************
	IFiles holds the file data blocks for input files.  There are three
static input streams:   the primary input stream,  the secondary input stream,
and the input stream used by the EQq command.  To access these three files,
identifiers defined in file tecoc.h are used to index into this array.
Other elements of this array are used to access input files for the EI
command.
*****************************************************************************/
/*static	struct	IFDBST	IFiles[NIFDBS];*/
/*****************************************************************************
	OFiles holds the file data blocks for the output files.  There are
three output streams:   the primary output stream,  the secondary output
stream and the output stream used by the E%q command.  The array is indexed
using identifiers defined in file tecoc.h.
*****************************************************************************/
/*static	struct	OFDBST	OFiles[NOFDBS];*/
/*****************************************************************************
	ZErMsg()
	This function displays error message from the operating system on
the terminal screen.  The error message text is retrieved from the operating
system and imbedded in a TECO-style message with the SYS mnemonic.
*****************************************************************************/
VVOID ZErMsg(stat1, stat2)
int stat1;
int stat2;
{
	puts("Terminating in function ZErMsg.\n");
	exit(EXIT_FAILURE);
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
DEFAULT ZChIn(NoWait)			/* input a character from terminal */
BOOLEAN NoWait;				/* return immediately? */
{
	puts("Terminating in function ZChIn.\n");
	exit(EXIT_FAILURE);
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
*****************************************************************************/
LONG ZClnEG(EGWhat,EGOper,TxtPtr)
DEFAULT EGWhat;			/* what to get/set/clear: MEM, LIB, etc. */
DEFAULT EGOper;			/* operation: get, set or clear */
charptr TxtPtr;			/* if setting,  value to set */
{
	puts("Terminating in function ZClnEG.\n");
	exit(EXIT_FAILURE);
}
/*****************************************************************************
	ZClnUp()
	This function cleans up in preparation for terminating TECO-C.
*****************************************************************************/
VVOID ZClnUp(void)			/* clean up for exit */
{
	puts("Terminating in function ZClnUp.\n");
	exit(EXIT_FAILURE);
}
/*****************************************************************************
	Zcp2ul()
	This function converts a pointer to an unsigned long.
*****************************************************************************/
#if DEBUGGING
ULONG Zcp2ul(voidptr cp)		/* convert charptr to ULONG */
{
	return ((ULONG)(cp));
}
#endif
/*****************************************************************************
	ZDoCmd()
	This function terminates TECO and feeds a command line to the
command line interpreter.
*****************************************************************************/
VVOID ZDoCmd()			/* die and pass command to OS */
{
	(void)ExeNYI();
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
VVOID ZDspBf(buffer, length)	/* output a buffer to terminal */
charptr buffer;
SIZE_T length;
{
	puts("Terminating in function ZDspBf.\n");
	exit(EXIT_FAILURE);
}
/*****************************************************************************
	ZDspCh()
	This function outputs a single character to the terminal.
*****************************************************************************/
VVOID ZDspCh(Charac)		/* output a character to terminal */
char Charac;
{
	puts("Terminating in function ZDspCh.\n");
	exit(EXIT_FAILURE);
}
/*****************************************************************************
	ZExCtB()
	This function implements the TECO ^B command,  which returns the
current date encoded in the following way:
		((year-1900)*16+month)*32+day
*****************************************************************************/
DEFAULT ZExCtB()			/* return current date */
{
	return ExeNYI();
}
/*****************************************************************************
	ZExCtH()
	This function implements the TECO ^H command,  which returns the
current time encoded in the following way:
		(seconds since midnight) / 2
*****************************************************************************/
DEFAULT ZExCtH()			/* return current time */
{
	return ExeNYI();
}
/*****************************************************************************
	ZExeEJ()
	This function executes an EJ command,  which returns environment
characteristics.  It returns:
	-1EJ	1024 under VAX/VMS	(4*256 = VAX, 0 = VMS in native mode)
		1025 under Ultrix	(4*256 = VAX, 1 = Ultrix)
		25600 under Sun/SunOS	(100*256 = Sun, 0 = SunOS)
		25856 under MS-DOS	(101*256 = IBM-PC, 0 = MS-DOS)
	0EJ	process id on VAXen, 0 on anything else
	1EJ	0 on all systems
	2EJ	UIC, in longword format (unlike TECO-11) on VAX/VMS,
		0 on all other systems.
*****************************************************************************/
DEFAULT ZExeEJ()			/* execute an EJ command */
{
	return ExeNYI();
}
/*****************************************************************************
	ZExit()
	This function terminates TECO-C with a status value.
*****************************************************************************/
VVOID ZExit(DEFAULT estat)		/* terminate TECO-C */
{
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
	puts("Terminating in function ZFree.\n");
	exit(EXIT_FAILURE);
}
/*****************************************************************************
	ZHelp()
	This function accepts a help string and displays the corresponding
help text.
	it should be control-C interrupt-able.
*****************************************************************************/
VVOID ZHelp(HlpBeg, HlpEnd, SysLib, Prompt)
charptr HlpBeg;			/* first char of help request */
charptr HlpEnd;			/* last character of help request */
BOOLEAN SysLib;			/* use default HELP library? */
BOOLEAN	Prompt;			/* enter interactive help mode? */
{
	puts("Terminating in function ZHelp.\n");
	exit(EXIT_FAILURE);
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
	puts("Terminating in function ZIClos.\n");
	exit(EXIT_FAILURE);
}
/*****************************************************************************
	ZCpyBl()
	See the definition of MEMMOVE in ZPORT.H for a description of this
function.
*****************************************************************************/
#if defined(NEEDS_ZCPYBL)
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
	puts("Terminating in function ZOClDe.\n");
	exit(EXIT_FAILURE);
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
	puts("Terminating in function ZOClos.\n");
	exit(EXIT_FAILURE);
}
/*****************************************************************************
	ZOpInp()
	This function opens an input file.  The name of the file is pointed
to by FBfBeg.  FBfPtr points to the character following the last character of
the file name.
*****************************************************************************/
DEFAULT ZOpInp(IfIndx, EIFile, RepFNF)
DEFAULT IfIndx;			/* index into file data block array IFiles */
BOOLEAN	EIFile;			/* is it a macro file? (hunt for it) */
BOOLEAN RepFNF;			/* report "file not found" error? */
{
	puts("Terminating in function ZOpInp.\n");
	exit(EXIT_FAILURE);
}
/*****************************************************************************
	ZOpOut()
	This function creates (and opens) an output file.  The name of
the file to be created is pointed to by FBfBeg.  FBfPtr points to the
character following the last character of the file name.
*****************************************************************************/
DEFAULT ZOpOut(OfIndx, RepErr)		/* open output file */
DEFAULT	OfIndx;				/* output file indicator */
BOOLEAN RepErr;				/* report errors? */
{
	puts("Terminating in function ZOpOut.\n");
	exit(EXIT_FAILURE);
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
	int	i;
	char	TmpBuf[256];
	SIZE_T	line_len;
	DBGFEN(2,"ZPrsCL",NULL);
/*
 * If the command line contains arguments,  construct a replica of the
 * command line in Q-register Z.  It's a "replica" because spacing might
 * be wrong.
 */
	if (argc > 1) {
		TmpBuf[0] = '\0';
		for (i = 1; i < argc; i++) {
			strcat(TmpBuf, *++argv);
			strcat(TmpBuf, " ");
		}
		line_len = strlen(TmpBuf)-1;	/* ignore trailing space */
		QR = &QRgstr[35];		/* 35 = q-register Z */
		if (MakRom(line_len) == FAILURE) {
			DBGFEX(2,DbgFNm,
			       "MakRom(line_len) failed, calling exit()");
			exit(EXIT_FAILURE);
		}
		MEMMOVE(QR->Start, TmpBuf, line_len);
		QR->End_P1 += line_len;		/* length of q-reg text */
	}
#if USE_ANSI_CLPARS
/*
 * execute imbedded command line-parsing macro directly from clpars[]
 */
	CStBeg = CBfPtr = clpars;		/* command string start */
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
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < CLPARS_LINES; i++) {
		line_len = strlen(clpars[i]);
		MEMMOVE(QR->End_P1, clpars[i], line_len);
		QR->End_P1 += line_len;		/* length of q-reg text */
	}
/*
 * Execute an MY$$ command.
 */
	CBfPtr = "my\33\33";			/* command string start */
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
	This function presets the wildcard lookup filename.  It is
called when the user executes an ENfilename$ command.  Later executions of
the EN$ command will cause the ZSWild function to be called to return
successive wildcard matches.
*****************************************************************************/
DEFAULT ZPWild()		/* preset the wildcard lookup filename */
{
	puts("Terminating in function ZPWild.\n");
	exit(EXIT_FAILURE);
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
DEFAULT ZRdLin(buf, buflen, fidx, length)
charptr buf;
SIZE_T buflen;
int fidx;
DEFAULT *length;
{
	puts("Terminating in function ZRdLin.\n");
	exit(EXIT_FAILURE);
}
/*****************************************************************************
	ZScrOp()
	This function is called to perform special screen functions.
*****************************************************************************/
VVOID ZScrOp(OpCode)		/* do a screen operation */
int OpCode;			/* code for operation */
{
	int index;
	static int map[] = {
		1,	/* 0 - VT52 is a VT52 */
		2,	/* 1 - VT61 is a VT61 */
		1,	/* 2 - VT100 in VT52 mode is a VT52 */
		0,	/* 3 - unused */
		3,	/* 4 - VT100 in ANSI mode is a VT100 */
		0,	/* 5 - unused */
		0,	/* 6 - VT05 is a VT05 */
		0,	/* 7 - unused */
		3,	/* 8 - VT102 is a VT100 */
		0,	/* 9 - unused */
		3,	/* 10 - VK100 is a VT100 */
		3,	/* 11 - VT200 in VT200 mode is a VT100 */
		3,	/* 12 - VT200 in VT100 mode is a VT100 */
		1,	/* 13 - VT200 in VT52 mode is a VT52 */
	};
	struct strng
		{
		charptr strt;
		DEFAULT len;
		};
	static struct strng CUP[] = {		/* cursor up one line */
		{"\232\0\0\0\0",	5},	/* VT05 - ? */
		{"\033A",		2},	/* VT52 - ESC A */
		{"",			0},	/* VT61 */
		{"\033[A",		3}	/* VT100 - ESC [ A */
	};
	static struct strng EEL[] = {		/* erase to end of line */
		{"\36",			1},	/* VT05 - RS */
		{"\033K\r",		3},	/* VT52 - ESC K CR */
		{"",			0},	/* VT61 */
		{"\033[K",		3}	/* VT100 - ESC [ K */
	};
	static struct strng ROF[] = {		/* reverse video on */
		{"",			0},	/* VT05 */
		{"",			0},	/* VT52 */
		{"",			0},	/* VT61 */
		{"\033[m",		3}	/* VT100 - ESC [ m */
	};
	static struct strng RON[] = {		/* reverse video off */
		{"",			0},	/* VT05 */
		{"",			0},	/* VT52 */
		{"",			0},	/* VT61 */
		{"\033[7m",		4}	/* VT100 - ESC [ 7 m */
	};
	if (CrType == UNTERM) {		/* if unknown terminal type */
		return;			/* can't do screen operations */
	}
/*
 * The numbering used for CrType comes from TECO-11.  Convert it to get an
 * index into the string arrays.
 */
	index = map[CrType];
	switch (OpCode) {
	    case SCR_CUP:   ZDspBf(CUP[index].strt, CUP[index].len);   break;
	    case SCR_EEL:   ZDspBf(EEL[index].strt, EEL[index].len);   break;
	    case SCR_ROF:   ZDspBf(ROF[index].strt, ROF[index].len);   break;
	    case SCR_RON:   ZDspBf(RON[index].strt, RON[index].len);   break;
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
DEFAULT ZSetTT(		/* tell operating system that we set the terminal */
DEFAULT TTWhat,		/* what terminal parameter to set */
DEFAULT TTVal)		/* what to set it to */
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
DEFAULT ZSWild()			/* search for next wildcard filename */
{
	puts("Terminating in function ZSWild.\n");
	exit(EXIT_FAILURE);
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
VVOID ZTrmnl()		/* set up I/O to the terminal */
{
	puts("Failing in function ZTrmnl\n");
	exit(EXIT_FAILURE);
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
	ZWrLin()
	This function writes a line to a file.
*****************************************************************************/
DEFAULT ZWrLin(OfIndx, BfrBeg, RecSiz)
DEFAULT	OfIndx;			/* index into OFiles array */
charptr BfrBeg;			/* address of output buffer */
ptrdiff_t RecSiz;		/* record size */
{
	puts("Terminating in function ZWrLin.\n");
	exit(EXIT_FAILURE);
}
