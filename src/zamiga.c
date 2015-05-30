/*****************************************************************************
	ZAmiga.c
	System dependent code for Amiga (SAS/C 5.10)
	This file created by Ben Mesander, ben@piglet.cr.usgs.gov, by
	starting with zunix.c and making the necessary changes.
*****************************************************************************/
/*
 * RCS information
 *
 * $Header: Elros:src/c/teco/zamiga.c,v 1.4 91/01/26 22:40:31 ben Exp Locker: ben $
 *
 * $Log:	zamiga.c,v $
 * Revision 1.4  91/01/26  22:40:31  ben
 * Removed possible recursive call of TAbort() in function sendpkt().
 * 
 * Revision 1.3  91/01/26  22:21:51  ben
 * Cleaned up program exit logic:
 * changed calls to ZExit() to TAbort().
 * 
 * Revision 1.2  91/01/26  21:23:12  ben
 * Added RCS information
 * 
 */
/*
 * Define standard functions. 
 */
#include <errno.h>		/* define errno */
#include <signal.h>		/* to catch ^C signal */
#include <stdio.h>		/* define stdin */
#include <string.h>		/* strncpy(), strlen(), etc. */
#include <time.h>		/* stuff for time commands */
#include <fcntl.h>		/* for isatty() call */
/*
 * Stuff used to fake a tempnam() call.
 */
char	*tempnam();		/* not in Amiga libraries */
/*
 * Amiga-specific stuff
 */
#include "exec/types.h"
#include "exec/ports.h"
#include "exec/io.h"
#include "exec/memory.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#undef GLOBAL		/* name conflict here, doesn't matter */
#define MAXARGS 7L	/* limit in packet structure (dosextens.h) */
#define NARGS	1L	/* number of arguments */
#define ESC    27L
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "clpars.h"		/* command-line parsing macro */
#include "dchars.h"		/* define identifiers for characters */
#include "deferr.h"		/* define identifiers for error messages */
#include "defext.h"		/* define external global variables */
#include "dscren.h"		/* define identifiers for screen i/o */
extern	int sys_nerr;		/* number of system error messages */
extern	char *sys_errlist[];	/* error message text */
static	BOOLEAN tty_set = FALSE;	/* Has the terminal been set? */
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
	char	OFNam[FILENAME_MAX];	/* output file name */
	char	OTNam[FILENAME_MAX];	/* temporary output file name */
	FILE	*OStrem;		/* stream */
} OFiles[NOFDBS];
/*****************************************************************************
	sendpkt()
	This function sends a message packet to an AmigaDOS handler
	It is used in teco to send control messages to the console -
	(turn on and off echo of characters)
*****************************************************************************/
static long sendpkt(pid,action,args,nargs)
struct MsgPort *pid;	/* process indentifier ... (handlers message port ) */
long action,           /* packet type ... (what you want handler to do )   */
     args[],          /* a pointer to a argument list */
     nargs;          /* number of arguments in list  */
{
  
 struct MsgPort        *replyport;
 struct StandardPacket *packet;
 
 long   count, *pargs, res1; 
 if(nargs > MAXARGS) ZExit(EXIT_FAILURE); /* not TAbort due to possible loop */
 
 replyport = (struct MsgPort *) CreatePort(NULL,NULL); /* make reply port */
 if(!replyport) return(NULL);
 packet = (struct StandardPacket *) 
   AllocMem((long)sizeof(*packet),MEMF_PUBLIC | MEMF_CLEAR);
 if(!packet) 
   {
    DeletePort(replyport);
    return(NULL);
   }
 packet->sp_Msg.mn_Node.ln_Name = (char *) &(packet->sp_Pkt); /* link packet- */
 packet->sp_Pkt.dp_Link         = &(packet->sp_Msg);        /* to message    */
 packet->sp_Pkt.dp_Port         = replyport;         /* set-up reply port   */
 packet->sp_Pkt.dp_Type         = action;           /* what to do... */
/* move all the arguments to the packet */
 pargs = &(packet->sp_Pkt.dp_Arg1);        /* address of first argument */
 for(count=NULL;count < nargs;count++) 
   pargs[count]=args[count];
 PutMsg(pid,packet); /* send packet */
#if DEBUGGING
  printf("Waiting for packet...\n"); 
#endif
 WaitPort(replyport); /* wait for packet to come back */
 GetMsg(replyport);   /* pull message */
 res1 = packet->sp_Pkt.dp_Res1; /* get result */
/* all done clean up */
 FreeMem(packet,(long)sizeof(*packet)); 
 DeletePort(replyport); 
 return(res1);   
  
}
/*****************************************************************************
  Non-ANSI-contemptible tempnam() call. Would that the SAS Institute ever
  really live up to their promise of an "ANSI-compatible" library.  Sigh.
*****************************************************************************/
static char *tempnam(dir,pfx)
char	*dir, *pfx;
{
	char *s;
	long timeVal;
	char timeAry[4];
	char tBuf[7];
	struct tm *ut;
	s = (char *)calloc(strlen(dir)+strlen(pfx)+8, sizeof(char));
	/* pathological case */
	if (s==NULL) return s;
	/* get current time to make a unique filename */
	
	timeVal = time(NULL);
	ut = localtime(&timeVal);
	timeAry[0] = (unsigned char)ut->tm_hour;
	timeAry[1] = (unsigned char)ut->tm_min;
	timeAry[2] = (unsigned char)ut->tm_sec;
	timeAry[3] = (unsigned char)0;
	stptime(tBuf, 1, timeAry);
	
	return(strcat(strcat(strcat(s,dir),pfx),tBuf));
}
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
	return (voidptr *)malloc(MemSize);
}
/*****************************************************************************
	ZBell()
	Thus function rings the terminal bell.  For most platforms,  this
means just writing a bell character (control-G) to the terminal.  An amusing
hack makes the Amiga SAY 'ding' out loud whenever a bell is hit...
*****************************************************************************/
VVOID ZBell(VVOID)
{
	FILE	*fp;
	
	if ((fp=fopen("SPEAK:","w"))==NULL) {
/*
 * can't open speak device, just flash the screen
 */
		ZDspCh('\7');
	} else {
/*
 * say 'ding'.
 */
		fputs("ding",fp);
		fclose(fp);
	}
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
	unsigned char Charac;
	static	BOOLEAN	LastLF = FALSE;
	if (LastLF) {
		LastLF = FALSE;
		return (DEFAULT)LINEFD;
	}
	if (read(fileno(stdin), &Charac, 1) != 1) {
		if (!GotCtC) {
			ZErMsg();
			ErrMsg(ERR_URC);
			exit(EXIT_FAILURE);
		}
	}
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
	This function executes a :EG command.  The :EG commands are
used to get access to operating system functions.  The minimum set of
functions is
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
	DBGFEN(1,"ZClnEG",NULL);
	DBGFEX(1,DbgFNm,"0");
	return 0;		/* :EG is unsupported */
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
VVOID ZClnUp (VVOID)			/* cleanup for TECO-C abort */
{
	DBGFEN(3,"ZClnUp","closing terminal channels and exiting");
	if (tty_set == TRUE) {
		struct MsgPort *conid;           /* for process id     */
		long arg[NARGS];                /* array of arguments */
		struct Process *myprocess;     /* me!                */
		myprocess = (struct Process *) FindTask(NULL);     
/*
 * get console handler
 */
		conid = (struct MsgPort *) myprocess->pr_ConsoleTask;
		arg[0]=FALSE; /* turn RAW mode off */
/*
 * No point in checking result here - we're going to exit anyway.
 */
		sendpkt(conid,ACTION_SCREEN_MODE,arg,NARGS);
#if DEBUGGING
		printf("CON: mode set...\n");
#endif
	}		
}
/*****************************************************************************
	ZDoCmd()
	This function terminates TECO and feeds a command line to the
command line interpreter.  The command to be executed is passed to this
function in the file name buffer (FBf).
*****************************************************************************/
VVOID ZDoCmd()			/* die and pass command to OS */
{
	char	buf[128+1];
	char	*space_p;
	DBGFEN(1,"ZDoCmd",NULL);
/*
 * 1. Terminate buf[] and command line in FBf
 * 2. make local copy since FBf will be free'd in ZClnUp()
 * 3. separate program name from arguments, if any
 * 4. Call ZClnUp to free up everything
 * 5. Execute the command line, with optional arguments.  If we know where
 *    the command processor is, use it so we can execute .BAT batch files
 * 6. we shouldn't be here, exit
 */
	buf[128] = *FBfPtr = '\0';
	(void)strncpy(buf, FBfBeg, 128);
	if ((space_p = strchr(buf,' ')) != NULL) {
		*space_p++ = '\0';
	}
	ZClnUp();
/* 
 * We should unload teco before calling system() - this will require some
 * low-level dorking about with AmigaDOS which I don't want to do right now.
 */
	if (*buf) {
		system(buf);
	}
	TAbort(EXIT_SUCCESS);
/* 
 * We should never reach this statement.
 */
	(void)perror ("");
	TAbort (EXIT_FAILURE);
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
	if (write(fileno(stdout), buffer, length) == -1) {
		/* Like this will work!??? */
		puts("Unable to write to terminal in function ZDspBf");
		TAbort(EXIT_FAILURE);
	}
}
/*****************************************************************************
	ZDspCh()
	This function outputs a single character to the terminal.
*****************************************************************************/
VVOID ZDspCh(Charac)		/* output a character to terminal */
char Charac;
{
	if (write(fileno(stdout), &Charac, 1) == -1) {
		puts("Unable to write to terminal in function ZDspCh");
		TAbort(EXIT_FAILURE);
	}
}
/*****************************************************************************
	ZExCtB()
	This function implements the TECO ^B command,  which returns the
current date encoded in the following way:
		((year-1900)*16+month)*32+day
*****************************************************************************/
DEFAULT ZExCtB()			/* return current date */
{
	time_t clock;
	struct tm *time_of_day;
	int tecodate;
	DBGFEN(1,"ZExCtB","");
	clock=time(NULL);
	time_of_day=localtime(&clock);
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
	time_t clock;
	struct tm *time_of_day;
	int tecotime;
	DBGFEN(1,"ZExCtH","");
	clock=time(NULL);
	time_of_day=localtime(&clock);
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
		1025 under Ultrix	(4*256 = VAX, 1 = Ultrix)
		25600 under Sun/SunOS	(100*256 = Sun, 0 = SunOS)
		25856 under MS-DOS	(101*256 = IBM-PC, 0 = MS-DOS)
		26112 under AmigaDOS 1.3 (102*256 = Amiga, 0 = AmigaDOS 1.3)
	0EJ	process id on VAXen, 0 on anything else
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
		DBGFEX(1,DbgFNm,"PushEx(26112)");
		return PushEx((LONG)26112, OPERAND);	/* means "Amiga" */
	}
	if (NArgmt == 0) {
		DBGFEX(1,DbgFNm,"PushEx(0)");
		return PushEx((LONG)0, OPERAND);
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
			exit(EXIT_FAILURE);
		}
		if (remove(OFiles[OfIndx].OFNam) != 0) {
			ZErMsg();
			ErrStr(ERR_UCD, OFiles[OfIndx].OFNam);
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
VVOID ZOClos(OfIndx)		/* close output file */
DEFAULT	OfIndx;			/* index into OFiles array */
{
	char	*DotPtr;
	char	TmpFsp[FILENAME_MAX];
	DBGFEN(2,"ZOClos",NULL);
	if (!IsOpnO[OfIndx])				/* if it's not open */
		return;					/* we're done */
	if (fclose(OFiles[OfIndx].OStrem) == EOF) {	/* close it */
		ZErMsg();
		ErrMsg(ERR_UCO);			/* unable to close */
		exit(EXIT_FAILURE);
	}
	if (OFiles[OfIndx].OTNam[0] != '\0') {	/* if temporary output file */
		(void)strcpy(TmpFsp, OFiles[OfIndx].OTNam);/* copy to TmpFsp */
		DotPtr = strchr(TmpFsp, '.');		/* find the "." */
		if (DotPtr != NULL) {			/* if "." exists */
			*DotPtr = '\0';			/* make it null */
		}
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
				exit(EXIT_FAILURE);
			}
		}
#if DEBUGGING
		sprintf(DbgSBf,"renaming %s to %s\r\n",
			OFiles[OfIndx].OTNam, TmpFsp);
		DbgFMs(2,DbgFNm,DbgSBf);
#endif
		if (rename(OFiles[OfIndx].OTNam,TmpFsp)) {
			ZErMsg();
			ErrMsg(ERR_UCO);
			exit(EXIT_FAILURE);
		}
#if DEBUGGING
		sprintf(DbgSBf,"renaming %s to %s\r\n",
			OFiles[OfIndx].OFNam, OFiles[OfIndx].OTNam);
		DbgFMs(2,DbgFNm,DbgSBf);
#endif
		if (rename(OFiles[OfIndx].OFNam,OFiles[OfIndx].OTNam)) {
			ZErMsg();
			ErrMsg(ERR_UCO);
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
the file name.  This function opens files in response to the EB, ER and EQ
commands.  It does not open input files for the EI command.
*****************************************************************************/
DEFAULT ZOpInp(IfIndx, EIFile, RepFNF)
DEFAULT IfIndx;			/* index into file data block array IFiles */
BOOLEAN	EIFile;			/* is it a macro file? (hunt for it) */
BOOLEAN RepFNF;			/* report "file not found" error? */
{
#if DEBUGGING
	static char *DbgFNm = "ZOpInp";
	sprintf(DbgSBf,", FBf = \"%.*s\"", (int)(FBfPtr-FBfBeg), FBfBeg);
	DbgFEn(2,DbgFNm,DbgSBf);
#endif
	*FBfPtr = '\0';			/* terminate the file name */
/*
 * If we're executing an EI command,  then the default file type (the part
 * of the filename following the ".") is "tec".  If we're supposed to use
 * the default type,  and if the filename doesn't already have a type,
 * then append ".tec" to the filename.
 */
	if (EIFile) {
		if (strchr(FBfBeg,'.') == NULL) {
			(void)strcat(FBfBeg,".tec");
			FBfPtr += 4;
		}
	}
	if ((IFiles[IfIndx] = fopen(FBfBeg, "r")) == NULL) {
		if (!RepFNF && ((errno == ENODEV) || (errno == ENOENT))) {
			DBGFEX(2,DbgFNm,"FILENF");
			return FILENF;
		}
		ZErMsg();
		DBGFEX(2,DbgFNm,"FAILURE");
		return FAILURE;
	}
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
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
 * with renaming things to make them come out right.  If no file with the
 * same name already exists,  then the output file can simply be opened.
 * The close routine will only rename files if a temporary file was created
 * by this routine.
 */
DEFAULT ZOpOut(OfIndx, RepErr)		/* open output file */
DEFAULT	OfIndx;				/* output file indicator */
BOOLEAN RepErr;				/* report errors? */
{
	char *tfname;
#if DEBUGGING
	static char *DbgFNm = "ZOpOut";
	sprintf(DbgSBf,", FBf = \"%.*s\"",(int)(FBfPtr-FBfBeg),FBfBeg);
	DbgFEn(2,DbgFNm,DbgSBf);
#endif
/*
 * If the output file already exists,  make a temporary file.  Note:  the
 * tempnam function is used instead of the ANSI C tmpnam function because
 * tmpnam will make a file name in /usr/tmp,  which we may not be able to
 * rename later if /usr/tmp is across disk partitions.
 * (That's for UNIX) AmigaDOS is similar. The tempdir, 't:' isn't going to
 * have files in it that are 'renamable' to the current directory.
 */
	*FBfPtr = '\0';
	if (access(FBfBeg, 0) == 0) {		/* if file already exists */
		tfname = (char *)tempnam("","tecoc"); /* null = current dir */
		(void)strcpy(OFiles[OfIndx].OFNam, tfname);
		free(tfname);
		(void)strcpy(OFiles[OfIndx].OTNam, FBfBeg);
#if DEBUGGING
			sprintf(DbgSBf,
				"file %s already exists\r\n",
				FBfBeg);
			DbgFMs(2,DbgFNm,DbgSBf);
#endif
	} else {
		(void)strcpy(OFiles[OfIndx].OFNam, FBfBeg);
		OFiles[OfIndx].OTNam[0] = '\0';
	}
#if DEBUGGING
			sprintf(DbgSBf,
				"creating file %s\r\n",
				OFiles[OfIndx].OFNam);
			DbgFMs(2,DbgFNm,DbgSBf);
#endif
	OFiles[OfIndx].OStrem = fopen(OFiles[OfIndx].OFNam, "w");
	if (OFiles[OfIndx].OStrem == NULL) {
		if (RepErr)
			ZErMsg();
		DBGFEX(2,DbgFNm,"FAILURE");
		return FAILURE;
	}
	DBGFEX(2,DbgFNm,"SUCCESS");
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
	DBGFEN(2,"ZPrsCL",NULL);
/*
  If the command line contains arguments,  construct a replica of the command
  line in Q-register Z.  It's a "replica" because spacing might be wrong.
*/
	if (argc > 1) {
		TmpBuf[0] = '\0';
		for (i=1; i<argc; i++) {
			(void)strcat(TmpBuf, *++argv);
			(void)strcat(TmpBuf, " ");
		}
		line_len = strlen(TmpBuf)-1;	/* remove trailing space */
		QR = &QRgstr[35];		/* 35 = q-register Z */
		if (MakRom(line_len) == FAILURE) {
			DBGFEX(2,DbgFNm,"exiting with EXIT_FAILURE");
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
	This function presets the wildcard lookup filename.  It is
called when the user executes an ENfilename$ command.  Later executions of
the EN$ command will cause the ZSWild function to be called to return
successive wildcard matches.
*****************************************************************************/
DEFAULT ZPWild()		/* preset the wildcard lookup filename */
{
/* ??? use sh -c ? */
	return ExeNYI();
}
/*****************************************************************************
	ZRaloc()
	This function performs the standard C library function realloc.
*****************************************************************************/
voidptr ZRaloc(OldBlk, NewSiz)		/* re-allocate memory */
voidptr OldBlk;
SIZE_T NewSiz;
{
	return (voidptr *)realloc(OldBlk, NewSiz);
}
/*****************************************************************************
        ZRdLin()
        This function reads a line from a file.  It is passed a buffer, the
size of the buffer, and a file pointer.  It returns the length of the line,
or sets IsEofI[] to TRUE if the end of file is encountered.
*****************************************************************************/
DEFAULT ZRdLin(ibuf, ibuflen, IfIndx, retlen)
charptr ibuf;
ptrdiff_t ibuflen;
int IfIndx;
DEFAULT *retlen;
{
        DBGFEN(3,"ZRdLin",NULL);
/*
 * make sure ibuflen is < 32K since fgets() requires an int
 */
        if (ibuflen > (ptrdiff_t) 32767) {
                ibuflen = (ptrdiff_t) 32767;
        }
/*
 * guarantee a trailing NUL in the buffer so strlen() always works
 */
        --ibuflen;
        ibuf[ibuflen] = '\0';
        if (fgets((char *)ibuf, (int)ibuflen, IFiles[IfIndx]) == NULL) {
                if (ferror (IFiles[IfIndx])) {
			ZErMsg();
                        DBGFEX(3,DbgFNm,"FAILURE, ferror() after fgets()");
                        return (FAILURE);
                }
		IsEofI[IfIndx] = TRUE;
                DBGFEX(3,DbgFNm,"SUCCESS, eof after fgets()");
        } else {
                *retlen = strlen((char *)ibuf);
		if ((EzFlag & EZ_UNIXNL) == 0) {       /* cnvrt \n to \r\n? */
		    if (ibuf[(*retlen)-1] == '\n') {
			ibuf[(*retlen)-1] = CRETRN;
			ibuf[*retlen] = LINEFD;
			*retlen += 1;
		    }
		}
#if DEBUGGING
                sprintf(DbgSBf,"SUCCESS, retlen = %d", *retlen);
                DbgFEx(3,DbgFNm,DbgSBf);
#endif
        }
        return SUCCESS;
}
/*****************************************************************************
	ZScrOp()
	This function is called to perform special screen functions.
	This is not yet finished...
*****************************************************************************/
VVOID ZScrOp(OpCode)		/* do a screen operation */
int OpCode;			/* code for operation */
{
	if (CrType == UNTERM) {		/* if unknown terminal type */
		return;			/* can't do screen operations */
	}
	switch (OpCode) {
		case SCR_CUP:	fputs("\0x1b[1A",stdout); /* cursor up */
				break;
		case SCR_EEL:	fputs("\0x1b[1K",stdout); /* clear to end of line */
				break;
		case SCR_ROF:	fputs("\0x1b[31m",stdout); /* reset standout mode */
				break;
		case SCR_RON:	fputs("\0x1b[33m",stdout); /* standout mode on */
				break;
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
DEFAULT ZSWild()		/* search for next wildcard filename */
{
	return ExeNYI();
}
/*****************************************************************************
	ZTrmnl()
	This function sets up the input/output of commands.  Usually, that
means the input/output channels to the terminal,  but TECOC might be run
from a command procedure (under VMS) or a script file (under Unix),  and
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
	if (EtFlag & ET_TRAP_CTRL_C) {		/* if user wants it */
		EtFlag &= ~ET_TRAP_CTRL_C;	/* turn off bit */
	} else {
		if (EtFlag & ET_MUNG_MODE)	/* if in MUNG mode */
			TAbort(EXIT_SUCCESS);
		GotCtC = TRUE;			/* set "stop soon" flag */
	}
	signal(SIGINT, CntrlC);
}
/*
 * ZTrmnl - set up terminal modes
 *
 */
VVOID ZTrmnl()			/* set up I/O to the terminal */
{
	EtFlag = ET_READ_LOWER |	/* guess: term has lowercase and */
		 ET_SCOPE;		/* it's a scope, not hardcopy */
/*
 * get terminal characteristics and set some signals
 */
	if (isatty(fileno(stdin))) tty_set = TRUE;	/* tell ZClnUp to clean up */
	signal(SIGINT, CntrlC);			/* call CntrlC on interrupt */
/*
 * set window to RAW: mode instead of CON: mode - this gets rid of echo.
 */
	if (tty_set) {
		struct MsgPort *conid;         /* for process id     */
		long   arg[NARGS],res1;        /* holds result       */ 
		struct Process *myprocess;     /* me!                */
		myprocess = (struct Process *) FindTask(NULL);     
/*
 * get console handler.
 */
		conid = (struct MsgPort *) myprocess->pr_ConsoleTask;
		arg[0]=DOSTRUE;			/* arg1=TRUE - set RAW mode */
		res1 = sendpkt(conid,ACTION_SCREEN_MODE,arg,NARGS);
/*
 * If it is not possible to talk to our console, something really strange
 * has happened to AmigaDOS. The system will probably GURU soon.
 * We best exit immediately.
 */ 
		if(!res1)
			TAbort(EXIT_FAILURE);
#if DEBUGGING
		puts("In RAW: mode.");
#endif
	}
	CrType = VT102;			/* Let's pretend we are a VT102 */
#if VIDEO
	VtSize = HtSize = 0;
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
 * and convert CR/LF pairs to just LF for output.
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
		if (fputc(*BfrPtr, OFiles[OfIndx].OStrem) == EOF) {
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
