/*****************************************************************************
	Zvms.c
	System dependent code for VAX/VMS.
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>		/* EXIT_SUCCESS and EXIT_FAILURE */
#include <string.h>		/* prototype for strlen */
#include dcdef			/* device class identifiers */
#include descrip		/* string descriptor macros */
#include dvidef			/* dvi item identifier codes */
#include fab			/* RMS file access block structures */
#include hlpdef			/* HLP$... identifiers */
#include iodef			/* i/o code identifiers */
#include jpidef			/* JPI$_... identifiers */
#include libclidef		/* LIB$K_CLI_LOCAL_SYM */
#include lib$routines		/* lib$ routines */
#include rab			/* RMS record access block structures */
#include rmsdef			/* RMS return status identifiers */
#include signal			/* arguments to function signal() */
#include ssdef			/* SS$_NORMAL and SS$_BADESCAPE */
#include stsdef			/* $VMS_STATUS_SUCCESS macro */
#include starlet		/* sys$ routines */
#include ttdef			/* ttdef union */
#include tt2def			/* tt2def union */
#include "zport.h"		/* portability identifiers */
#include "tecoc.h"		/* general identifiers */
#include "chmacs.h"		/* character processing macros */
#include "clpars.h"		/* command-line parsing macro */
#include "dchars.h"		/* identifiers for characters */
#include "deferr.h"		/* identifiers for error messages */
#include "defext.h"		/* external global variables */
#include "dscren.h"		/* identifiers for screen i/o */
#define SOBSIZE		1024
#define TERM_IN_EFN	1	/* terminal input event flag */
#define TERM_OUT_EFN	2	/* terminal output event flag */
struct getxxx_iosb_struct {		/* for calling $getdvi or $getjpi */
	unsigned int io_status;
	unsigned int reserved_to_Digital;
};
struct tt_mode_iosb_struct {		/* for setting/sensing term. modes */
	unsigned short io_status;
	unsigned char transmit_speed;
	unsigned char receive_speed;
	unsigned char CR_fill_count;
	unsigned char LF_fill_count;
	unsigned char parity_flags;
	unsigned char unused;
};
struct tt_rw_iosb_struct {		/* for reading/writing to terminal */
	unsigned short io_status;
	unsigned short byte_count;
	unsigned short terminator;
	unsigned short terminator_size;
};
struct tt_mode_characteristics_struct {
	unsigned char class;		/* device class */
	unsigned char type;		/* terminal type */
	unsigned short nbr_columns;	/* number of columns */
	union ttdef lw2;		/* longword 2 */
};
static	struct	FAB IFab;		/* input file access block */
static	struct	NAM INam;		/* input name block */
static	struct	RAB IRab;		/* input file record access block */
static	struct	FAB OFab;		/* output file access block */
static	struct	NAM ONam;		/* output name block */
static	struct	RAB ORab;		/* output file record access block */
static	unsigned char *TIBBeg;		/* SYS$INPUT buffer */
static	unsigned char *TIBEnd;		/* SYS$INPUT buffer end */
static	unsigned char *TIBERc;		/* ptr to end of record in buffer */
static	unsigned char *TIBPtr;		/* ptr to current char in record */
static	unsigned char TOBBeg[SOBSIZE];	/* SYS$OUTPUT buffer */
static	unsigned char *TOBEnd;		/* SYS$OUTPUT buffer end (+1) */
static	unsigned char *TOBPtr;		/* SYS$OUTPUT buffer pointer */
static	struct	FAB TIFab;		/* SYS$INPUT file access block */
static	struct	RAB TIRab;		/* SYS$INPUT record access block */
static	struct	FAB TOFab;		/* SYS$OUTPUT file access block */
static	struct	RAB TORab;		/* SYS$OUTPUT record access block */
static	short	TCChan = 0;		/* terminal command channel */
static	short	TIChan = 0;		/* terminal input channel */
static	short	TOChan = 0;		/* terminal output channel */
static  struct	tt_mode_characteristics_struct tt_chars;
static	char	WBfExp[NAM$C_MAXRSS];	/* wildcard expanded filename buf */
static	struct	FAB WFab;		/* wildcard file access block */
static	struct	NAM WNam;		/* wildcard name block */
/*****************************************************************************
	IFiles holds the file data blocks for input files.  There are three
static input streams:   the primary input stream,  the secondary input stream,
and the input stream used by the EQq command.  To access these three files,
identifiers defined in file tecoc.h are used to index into this array.
Other elements of this array are used to access input files for the EI
command.
*****************************************************************************/
static struct IFile_struct {
	struct	FAB	IFab;		/* file access block */
	struct	NAM	INam;		/* NAM block */
	struct	RAB	IRab;		/* record access block */
	BOOLEAN leftover_input_exists;	/* after-FF text saved? (ZRdLin) */
	char *leftover_input;		/* after-FF text (see ZRdLin) */
	size_t leftover_size;		/* after-FF text size (see ZRdLin) */
} IFiles[NIFDBS];
/*****************************************************************************
	OFiles holds the file data blocks for the output files.  There are
three output streams:   the primary output stream,  the secondary output
stream and the output stream used by the E%q command.  The array is indexed
using identifiers defined in file tecoc.h.
*****************************************************************************/
static struct {
	struct	FAB	OFab;		/* file access block */
	struct	NAM	ONam;		/* NAM block */
	struct	RAB	ORab;		/* record access block */
} OFiles[NOFDBS];
/*****************************************************************************
	ZErMsg()
	This function displays error message from the operating system on
the terminal screen.  The error message text is retrieved from the operating
system and imbedded in a TECO-style message with the SYS mnemonic.
*****************************************************************************/
static int v_action(	struct dsc$descriptor_s *msg_desc,
			struct dsc$descriptor_s *errstr_desc)
{
	MEMMOVE(	errstr_desc->dsc$a_pointer,	/* destination */
			msg_desc->dsc$a_pointer,	/* source */
			msg_desc->dsc$w_length);	/* size */
	errstr_desc->dsc$w_length = msg_desc->dsc$w_length;
	return SS$_ACCVIO;
}
static VVOID ZErMsg(	/* display an operating-system-supplied message */
	int stat1,
	int stat2)
{
	struct
		{
		short	argcnt;		/* argument count */
		short	msgflg;		/* message flags */
		int	msgcod;		/* message code */
		int	rmsstv;		/* RMS alternate status */
		} msgvec;
	unsigned int status;
	charptr tptr;
	struct	dsc$descriptor errstr_desc;
	char errstr[132];
	errstr_desc.dsc$w_length  = sizeof(errstr) - 1;
	errstr_desc.dsc$a_pointer = &errstr[1];
	msgvec.argcnt = 2;		/* number of longwords in msgvec */
	msgvec.msgflg = 15;		/* display all parts of message */
	msgvec.msgcod = stat1;		/* message code */
	msgvec.rmsstv = stat2;		/* RMS alternate status */
	status = sys$putmsg(	&msgvec,	/* message vector */
				v_action,	/* action routine */
				0,		/* facility name */
				&errstr_desc);	/* errstr descriptor */
	if (!$VMS_STATUS_SUCCESS(status)) {
		lib$stop(status);
	}
/*
 * If the message is a successful one,  then inform the user by surrounding
 * the text in brackets.  That way the user won't think an actual error has
 * occurred.  If it's not successful, just display it.
 */
	if ($VMS_STATUS_SUCCESS(stat1)) {
		errstr[0] = '[';
		tptr = &errstr[errstr_desc.dsc$w_length];
		tptr++;
		*tptr++ = ']';
		errstr[errstr_desc.dsc$w_length] = '\0';
		TypBuf(errstr, tptr);
	} else {
		errstr[errstr_desc.dsc$w_length+1] = '\0';
		ErrStr(ERR_SYS, &errstr[1]);
	}
}
/*****************************************************************************
	ZAlloc()
	This function allocates memory.  The single argument is the number of
bytes to allocate.  TECO-C uses the ZFree and ZRaloc functions to de-allocate
and re-allocate, respectively,  the memory allocated by this function.
*****************************************************************************/
voidptr ZAlloc(SIZE_T MemSize)		/* allocate memory */
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
VVOID ZBell(void)
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
	5.  If the NoWait argument is TRUE, don't wait
	6.  When the user hits the RETURN key,  TECO is supposed to see
	    a carriage return and then a line feed.  The function must
	    deal with this by returning a carriage return to the caller
	    and then "remembering" to send a line feed on the next call.
	7.  handle ET_BKSP_IS_DEL flag
*****************************************************************************/
DEFAULT ZChIn(BOOLEAN NoWait)	/* input a character from the terminal */
{
	unsigned char Charac;
	int	io_function =	IO$_READVBLK|
				IO$M_NOFILTR|
				IO$M_NOECHO;
	static	BOOLEAN	LastCR = FALSE;
	unsigned int status;
	struct	tt_rw_iosb_struct ttread_iosb;
	if (TIChan) {				/* if it's a terminal */
		if (LastCR) {			/* if last char was a <CR> */
			LastCR = FALSE;
			return (DEFAULT)LINEFD;
		}
		if (NoWait) {
			io_function |= IO$M_TIMED;
		} else {
			io_function &= ~IO$M_TIMED;
		}
		status = sys$qiow(
				TERM_IN_EFN,	/* event flag number */
				TIChan,		/* channel */
				io_function,	/* I/O function */
				&ttread_iosb,	/* I/O status block */
				0,		/* AST routine address */
				0,		/* AST parameter */
				&Charac,	/* input buffer */
				1,		/* input buffer size */
				0,		/* p3 */
				0,		/* p4 */
				0,		/* p5 */
				0);		/* p6 */
		if (!$VMS_STATUS_SUCCESS(status)) {
			ZErMsg(status, 0);
			ErrMsg(ERR_URC);
			exit(EXIT_FAILURE);
		}
		switch (ttread_iosb.io_status) {
		case SS$_NORMAL:
		case SS$_ABORT:
			break;
		case SS$_CONTROLC:
			return 3;
		case SS$_TIMEOUT:
			return -1;
		default:
			lib$stop(ttread_iosb.io_status);
		}
		if (Charac == CRETRN) {
			LastCR = TRUE;
		}
	} else {					/* else not term. */
		if (TIBPtr = TIBERc) {			/* if rec used up */
			status = sys$get(&TIRab);	/* get next record */
			if (status != RMS$_NORMAL) {
				ZErMsg(status, TIRab.rab$l_stv);
				ErrMsg(ERR_URC);
				exit(EXIT_FAILURE);
			}
			TIBERc = &TIBBeg[TIRab.rab$w_rsz];
			*TIBERc++ = CRETRN;
			*TIBERc++ = LINEFD;
			TIBPtr = TIBBeg;
		}
		Charac = *TIBPtr++;
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
	:EGSYM$		sets a DCL local symbol (only for VAX/VMS)
although more functions may be defined.
*****************************************************************************/
static SetSym(charptr TxtPtr)		/* set a DCL local symbol */
{
    struct dsc$descriptor_s symbol_desc;
    struct dsc$descriptor_s value_desc;
    unsigned int status;
    DBGFEN(1,"SetSym","");
    symbol_desc.dsc$b_dtype = value_desc.dsc$b_dtype = DSC$K_DTYPE_T;
    symbol_desc.dsc$b_class = value_desc.dsc$b_class = DSC$K_CLASS_S;
    while (*TxtPtr == ' ') {			/* skip extra spaces */
	if (TxtPtr == '\0') {
	    DBGFEX(1,DbgFNm,"1 (supported, but failed)");
	    return 1;				/* supported, but failed */
	}
	TxtPtr++;
    }
    symbol_desc.dsc$a_pointer = TxtPtr;		/* find delimiter */
    while (*TxtPtr != ' ') {
	if (TxtPtr == '\0') {
	    DBGFEX(1,DbgFNm,"1 (supported, but failed)");
	    return 1;				/* supported, but failed */
	}
	TxtPtr++;
    }
    symbol_desc.dsc$w_length = TxtPtr - symbol_desc.dsc$a_pointer;
    while (*TxtPtr == ' ') {			/* skip extra spaces */
	if (TxtPtr == '\0') {
	    DBGFEX(1,DbgFNm,"1 (supported, but failed)");
	    return 1;				/* supported, but failed */
	}
	TxtPtr++;
    }
    value_desc.dsc$w_length = strlen(TxtPtr);
    value_desc.dsc$a_pointer = TxtPtr;
    status = lib$set_symbol(
			&symbol_desc,		/* symbol name */
			&value_desc,		/* value string */
			&LIB$K_CLI_LOCAL_SYM);	/* local or global */
    if (!$VMS_STATUS_SUCCESS(status)) {
	DBGFEX(1,DbgFNm,"supported, but failed");
	return status;				/* return "failed" */
    }
    DBGFEX(1,DbgFNm,"-1 (success)");
    return -1;					/* return "success" */
}
LONG ZClnEG(			/* execute special :EG command */
    DEFAULT EGWhat,		/* what to get/set/clear: MEM, LIB, etc. */
    DEFAULT EGOper,		/* operation: get, set or clear */
    charptr TxtPtr)		/* if setting, null-terminated value to set */
{
    unsigned int status;
    struct dsc$descriptor_s *log_desc;
    static readonly $DESCRIPTOR(mem_desc,"TEC$MEMORY");
    static readonly $DESCRIPTOR(ini_desc,"TEC$INIT");
    static readonly $DESCRIPTOR(lib_desc,"TEC$LIBRARY");
    static readonly $DESCRIPTOR(vte_desc,"TEC$VTEDIT");
    DBGFEN(1,"ZClnEG","");
    switch (EGWhat) {
	case EG_INI:	log_desc = &ini_desc;	break;
	case EG_LIB:	log_desc = &lib_desc;	break;
	case EG_MEM:	log_desc = &mem_desc;	break;
	case EG_VTE:	log_desc = &vte_desc;	break;
	default:	if (To_Upper(*TxtPtr) == 'S') {
			    TxtPtr++;
			    if (To_Upper(*TxtPtr) == 'Y') {
				TxtPtr++;
				if (To_Upper(*TxtPtr) == 'M') {
				    TxtPtr++;
				    if (*TxtPtr == ' ') {
					return SetSym(++TxtPtr);
				    }
				}
			    }
			}
			DBGFEX(1,DbgFNm,"0 (unsupported)");
			return 0;
    }
    if (EGOper == GET_VAL) {
	short length;
	struct dsc$descriptor_s RSL_desc;
	RSL_desc.dsc$w_length  = NAM$C_MAXRSS;
	RSL_desc.dsc$b_dtype   = DSC$K_DTYPE_T;
	RSL_desc.dsc$b_class   = DSC$K_CLASS_S;
	RSL_desc.dsc$a_pointer = FBfBeg;
	status = sys$trnlog(	log_desc,	/* logical name */
				&length,	/* returned string length */
				&RSL_desc,	/* returned string buffer */
				0,		/* logical name table */
				0,		/* access mode */
				3);		/* table search mask */
	if (status == SS$_NORMAL) {		/* if it translated */
	    FBfPtr = FBfBeg + length;
	    *FBfPtr = '\0';			/* null-terminate it */
	    DBGFEX(1,DbgFNm,"-1 (success)");
	    return -1;				/* return "success" */
	}
	FBfPtr = FBfBeg;			/* nullify the "result" */
	if (!$VMS_STATUS_SUCCESS(status)) {	/* if problem */
	    DBGFEX(1,DbgFNm,"supported, but failed");
	    return status;			/* return "failed" */
	}
	DBGFEX(1,DbgFNm,"-1 (success)");
	return -1;				/* return "success" */
    }
    if (EGOper == CLEAR_VAL) {
	status = lib$delete_logical(
				log_desc,	/* logical name */
				0);		/* table name */
    } else {
	struct dsc$descriptor_s EQU_desc;
	EQU_desc.dsc$w_length  = strlen(TxtPtr);
	EQU_desc.dsc$b_dtype   = DSC$K_DTYPE_T;
	EQU_desc.dsc$b_class   = DSC$K_CLASS_S;
	EQU_desc.dsc$a_pointer = TxtPtr;
	status = lib$set_logical(
				log_desc,	/* logical name */
				&EQU_desc);	/* equivalence name */
    }
    if (!$VMS_STATUS_SUCCESS(status)) {		/* if not successful */
	DBGFEX(1,DbgFNm,"supported, but failed");
	return status;				/* return "failed" */
    }
    DBGFEX(1,DbgFNm,"-1 (success0");
    return -1;					/* return "success" */
}
/*****************************************************************************
	ZClnUp()
	This function cleans up in preparation for terminating TECO-C.
*****************************************************************************/
VVOID ZClnUp(void)			/* clean up for exit */
{
	unsigned int status;
	DBGFEN(3,"ZClnUp","closing terminal channels and exiting");
	if (TIChan) {				/* if it's a terminal */
		status = sys$dassgn(TIChan);	/* de-assign the channel */
		if (!$VMS_STATUS_SUCCESS(status))
			lib$stop(status);
	} else {				/* else process-perm file */
		status = sys$close(&TIFab);	/* close the file */
		if (status != RMS$_NORMAL) {
			lib$stop(status);
		}
	}
	if (TOChan) {				/* if it's a terminal */
		status = sys$dassgn(TOChan);	/* de-assign the channel */
		if (!$VMS_STATUS_SUCCESS(status)) {
			lib$stop(status);
		}
	} else {				/* else process-perm file */
		if (TOBPtr != TOBBeg) {
		    TORab.rab$w_rsz = TOBPtr - TOBBeg;
		    status = sys$put(&TORab);	/* output the record */
		    if (status != RMS$_NORMAL) {
			lib$stop(status, TORab.rab$l_stv);
		    }
		}
		status = sys$close(&TOFab);	/* close the file */
		if (status != RMS$_NORMAL) {
			lib$stop(status);
		}
	}
	if (TCChan) {				/* if it was assigned */
		status = sys$dassgn(TCChan);	/* de-assign the channel */
		if (!$VMS_STATUS_SUCCESS(status)) {
			lib$stop(status);
		}
	}
}
#if DEBUGGING
ULONG Zcp2ul(voidptr cp)		/* convert charptr to ULONG */
{
	return (ULONG)(cp);
}
#endif
/*****************************************************************************
	ZDoCmd()
	This function terminates TECO and feeds a command line to the
command line interpreter.  The command to be executed is passed to this
function in the file name buffer (FBf).
*****************************************************************************/
VVOID ZDoCmd(void)		/* die and pass command to OS */
{
	unsigned int status;
	struct	dsc$descriptor_s CS_descriptor;
	DBGFEN(1,"ZDoCmd","");
	CS_descriptor.dsc$w_length  = FBfPtr - FBfBeg;
	CS_descriptor.dsc$b_dtype   = DSC$K_DTYPE_T;
	CS_descriptor.dsc$b_class   = DSC$K_CLASS_S;
	CS_descriptor.dsc$a_pointer = FBfBeg;
	DBGFEX(1,DbgFNm,"calling lib$do_command");
	status = lib$do_command(&CS_descriptor);
	if (!$VMS_STATUS_SUCCESS(status)) {
		lib$stop(status);
	}
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
#define MAXTOUT 500
VVOID ZDspBf(			/* output a buffer to the terminal */
	charptr buffer,
	SIZE_T length)
{
	int	iolength;
	struct	tt_rw_iosb_struct ttwrite_iosb;
	unsigned int status;
	DEFAULT	TmpLng;
	if (TOChan) {				/* if it's a terminal */
		while ((length > 0) && !GotCtC) {
			iolength = (length > MAXTOUT) ? MAXTOUT : length;
			status = sys$qiow(
				TERM_OUT_EFN,	/* event flag number */
				TOChan,		/* channel */
				IO$_WRITEVBLK,	/* I/O func */
				&ttwrite_iosb,	/* I/O status block */
				0,		/* AST routine address */
				0,		/* AST parameter */
				buffer,		/* p1 */
				iolength,	/* p2 */
				0,		/* p3 */
				0,		/* p4 */
				0,		/* p5 */
				0);		/* p6 */
			if (!$VMS_STATUS_SUCCESS(status)) {
				lib$stop(status);
			}
			if ((!$VMS_STATUS_SUCCESS(ttwrite_iosb.io_status)) &&
			    (ttwrite_iosb.io_status != SS$_ABORT) &&
			    (ttwrite_iosb.io_status != SS$_CONTROLC) &&
			    (ttwrite_iosb.io_status != SS$_CONTROLO)) {
				lib$stop(ttwrite_iosb.io_status);
			}
			buffer += iolength;
			length -= iolength;
		}
	} else {				/* else it's not a terminal */
		for (TmpLng=1; TmpLng<=length; ++TmpLng) {
			ZDspCh(*buffer++);
		}
	}
}
/*****************************************************************************
	ZDspCh()
	This function outputs a single character to the terminal.
*****************************************************************************/
VVOID ZDspCh(char Charac)	/* output a character to the terminal */
{
	unsigned int status;
	struct	tt_rw_iosb_struct ttwrite_iosb;
	if (TOChan) {				/* if it's a terminal */
		status = sys$qiow(
				TERM_OUT_EFN,	/* event flag number */
				TOChan,		/* channel */
				IO$_WRITEVBLK,	/* I/O func */
				&ttwrite_iosb,	/* I/O status block */
				0,		/* AST routine address */
				0,		/* AST parameter */
				&Charac,	/* p1 */
				1,		/* p2 */
				0,		/* p3 */
				0,		/* p4 */
				0,		/* p5 */
				0);		/* p6 */
		if (!$VMS_STATUS_SUCCESS(status)) {
			lib$stop(status);
		}
		if ((!$VMS_STATUS_SUCCESS(ttwrite_iosb.io_status)) &&
		     (ttwrite_iosb.io_status != SS$_CONTROLO)) {
			lib$stop(ttwrite_iosb.io_status);
		}
	} else {				/* else it's not a terminal */
		if (IsEOL(Charac)) {
			if (Charac == LINEFD) {
				if (TOBPtr > TOBBeg) {
					--TOBPtr;
					if (*TOBPtr != CRETRN) {
						++TOBPtr;
					}
				}
			} else {
				*TOBPtr = Charac;
				if (++TOBPtr > TOBEnd) {
					lib$stop(SS$_NORMAL);
				}
			}
			TORab.rab$w_rsz = TOBPtr - TOBBeg;
			status = sys$put(&TORab);	/* output the record */
			if (status != RMS$_NORMAL) {	/* if it didn't work */
				lib$stop(status, TORab.rab$l_stv);
			}
			TOBPtr = TOBBeg;
		} else {
			*TOBPtr = Charac;
			if (++TOBPtr > TOBEnd) {
				lib$stop(SS$_NORMAL);
			}
		}
	}
}
/*****************************************************************************
	ZEgSym()
	This macro gets or sets or clears a CLI symbol.name.  This function
is called to implement the :EGSYM command under VAX/VMS.
*****************************************************************************/
DEFAULT ZEgSym(SFOpTp,TxtPtr)	/* get/set/clear DCL symbol value */
DEFAULT SFOpTp;
charptr TxtPtr;
{
	unsigned int status;
	struct dsc$descriptor_s Sym_desc = {
		0,				/* dsc$w_length */
		DSC$K_DTYPE_T,			/* dsc$b_dtype */
		DSC$K_CLASS_S,			/* dsc$b_class */
		0				/* dsc$a_pointer */
	};
	struct dsc$descriptor_s Val_desc = {
		NAM$C_MAXRSS,			/* dsc$w_length */
		DSC$K_DTYPE_T,			/* dsc$b_dtype */
		DSC$K_CLASS_S,			/* dsc$b_class */
		FBfBeg				/* dsc$a_pointer */
	};
	short	length;
	DBGFEN(1,"ZEgSym","");
	Sym_desc.dsc$w_length = FBfPtr-TxtPtr;
	Sym_desc.dsc$a_pointer = TxtPtr;
	if (SFOpTp == GET_VAL) {
		status = lib$get_symbol(
				&Sym_desc,	/* symbol */
				&Val_desc,	/* return string buffer */
				&length,	/* returned string length */
				0);		/* table indicator */
		if (!$VMS_STATUS_SUCCESS(status)) { /* if something is wrong */
			FBfPtr = FBfBeg;	/* nullify the "result" */
			DBGFEX(1,DbgFNm,"");
			return status;		/* return "failed" */
		}
		FBfPtr = FBfBeg + length;
		*FBfPtr = '\0';
		DBGFEX(1,DbgFNm,"-1");
		return -1;			/* return "success" */
	}
	if (SFOpTp == CLEAR_VAL) {
		Val_desc.dsc$w_length = 0;
	} else {
		Val_desc.dsc$w_length = FBfPtr-TxtPtr;
	}
	Val_desc.dsc$a_pointer = TxtPtr;
	status = lib$set_symbol(&Sym_desc,	/* symbol */
				&Val_desc,	/* value */
				0);		/* table indicator */
	if (!$VMS_STATUS_SUCCESS(status)) {
		DBGFEX(1,DbgFNm,"");
		return status;			/* return "failed" */
	}
	DBGFEX(1,DbgFNm,"-1");
	return -1;				/* return "success" */
}
/*****************************************************************************
	ZExCtB()
	This function implements the TECO ^B command,  which returns the
current date encoded in the following way:
		((year-1900)*16+month)*32+day
*****************************************************************************/
DEFAULT ZExCtB(void)			/* return current date */
{
	unsigned int status;
	short	system_time[7];
	LONG	teco_date;
	DBGFEN(1,"ZExCtB","");
	status = sys$numtim(	system_time,		/* returned time */
				0);			/* time to convert */
	if (!$VMS_STATUS_SUCCESS(status)) {
		lib$stop(status);
	}
	teco_date = (system_time[0] - 1900) << 4;	/* (year-1900) * 16 */
	teco_date += system_time[1];			/* month */
	teco_date = teco_date << 5;			/* multiply by 32 */
	teco_date += system_time[2];			/* day of month */
	DBGFEX(1,DbgFNm,"PushEx()");
	return PushEx(teco_date, OPERAND);
}
/*****************************************************************************
	ZExCtH()
	This function implements the TECO ^H command,  which returns the
current time encoded in the following way:
		(seconds since midnight) / 2
*****************************************************************************/
DEFAULT ZExCtH(void)			/* return current time */
{
	unsigned int status;
	short	system_time[7];
	LONG	teco_time;
	DBGFEN(1,"ZExCtH","");
	status = sys$numtim(	system_time,	/* returned time */
				0);		/* time to convert */
	if (!$VMS_STATUS_SUCCESS(status)) {
		lib$stop(status);
	}
	teco_time = system_time[3] * 60;	/* hours * 60 */
	teco_time += system_time[4];		/* minutes */
	teco_time *= 30;
	teco_time += system_time[5] >> 1;	/* seconds / 2 */
	DBGFEX(1,DbgFNm,"PushEx()");
	return PushEx(teco_time, OPERAND);
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
DEFAULT ZExeEJ(void)			/* execute an EJ command */
{
	LONG	RetVal;
	DBGFEN(1,"ZExeEJ","");
	if (EStTop == EStBot) {			/* if no numeric argument */
		NArgmt = 0;			/* default is 0EJ */
	} else {
		UMinus();			/* if -EJ, make it -1EJ */
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	if (NArgmt < 0) {			/* oper. system dependent */
		if (NArgmt == -1) {
			RetVal = 1024;		/* means "VAX running VMS" */
		} else {
			return ExeNYI();
		}
	} else if (NArgmt == 1) {		/* 1EJ (terminal unit) */
		RetVal = 0;
	} else {
		struct getxxx_iosb_struct getjpi_iosb;
		struct {
			short	buflen1;	/* buffer length */
			short	itmcode1;	/* item code */
			charptr	buffer1;	/* buffer address */
			short	*retlen1;	/* returned length */
			int	termin;		/* item list terminator */
		} item_list = {
				4,		/* buffer length */
				JPI$_UIC,	/* uic (2EJ) */
				&RetVal,	/* buffer address */
				0,		/* returned length */
				0		/* item list terminator */
		};
		unsigned int status;
		if (NArgmt == 0) {			/* 0EJ (process id) */
			item_list.itmcode1 = JPI$_PID;
		}
		status = sys$getjpiw(	0,		/* event flag */
					0,		/* pid address */
					0,		/* process name */
					&item_list,	/* item list */
					&getjpi_iosb,	/* i/o status block */
					0,		/* AST routine */
					0);		/* AST parameter */
		if (!$VMS_STATUS_SUCCESS(status)) {
			lib$stop(status);
		}
		if (!$VMS_STATUS_SUCCESS(getjpi_iosb.io_status)) {
			lib$stop(getjpi_iosb.io_status);
		}
	}
	DBGFEX(1,DbgFNm,"PushEx()");
	return PushEx(RetVal,OPERAND);
}
/*****************************************************************************
	ZExit()
	This function terminates TECO-C with a status value.
*****************************************************************************/
VVOID ZExit(DEFAULT estat)		/* terminate TECO-C */
{
/*
 * Exit with a success status if we're exiting normally.  If we're exiting
 * because something went wrong,  then exit with a VMS error status.  Since
 * we don't have a message file, just steal a system-defined code (ABORT)
 * which is somewhat meaningful.
 */
	sys$exit((estat == EXIT_SUCCESS) ? SS$_NORMAL : SS$_ABORT);
}
/*****************************************************************************
	ZFree()
	This function frees memory previously allocated by the ZAlloc
function.
*****************************************************************************/
VVOID ZFree(voidptr pointer)		/* free memory allocated by ZAlloc */
{
	free(pointer);
}
/*****************************************************************************
	ZHelp()
	This function accepts a help string and displays the corresponding
help text.
	it should be control-C interrupt-able.
*****************************************************************************/
/*****************************************************************************
	Help on the VAX is accessed through the help library routines.
*****************************************************************************/
VVOID ZHelp(			/* display a help message */
	charptr HlpBeg,		/* first char of help request */
	charptr HlpEnd,		/* last character of help request */
	BOOLEAN SysLib,		/* use default HELP library? */
	BOOLEAN	Prompt)		/* enter interactive help mode? */
{
	struct dsc$descriptor_s line_desc;
	struct dsc$descriptor_s lib_desc;
	unsigned int	flags;
	unsigned int	status;
	$DESCRIPTOR(syshelp_desc, "SYS$HELP:HELPLIB");
	$DESCRIPTOR(techelp_desc, "TEC$HELP");
#if DEBUGGING
	static char *DbgFNm = "ZHelp";
	sprintf(DbgSBf,"text = \"%.*s\"", (int)(HlpEnd-HlpBeg), HlpBeg);
	DbgFEn(2,DbgFNm,DbgSBf);
#endif
	line_desc.dsc$a_pointer = HlpBeg;
	line_desc.dsc$w_length = (HlpEnd - HlpBeg) + 1;
	lib_desc = (SysLib ? syshelp_desc : techelp_desc);
	flags = HLP$M_PROCESS | HLP$M_GROUP | HLP$M_SYSTEM | HLP$M_HELP;
	if (Prompt) {
		flags |= HLP$M_PROMPT;
	}
	status = lbr$output_help(	lib$put_output,	/* output routine */
					0,		/* output width */
					&line_desc,	/* line-desc */
					&lib_desc,	/* library name */
					&flags,		/* flags */
					lib$get_input);	/* input rotuine */
	if (status != SS$_NORMAL) {
		ZErMsg(status,0);
	}
	DBGFEX(2,DbgFNm,"");
}
/*****************************************************************************
	ZIClos()
	This function closes the current input file.  It must
	1.  if current input stream is not open,  simply return
	2.  close the input file
	3.  set open indicator to FALSE
*****************************************************************************/
VVOID ZIClos(DEFAULT IfIndx)		/* close input file */
{
	unsigned int status;
	DBGFEN(2,"ZIClos","");
	if (IsOpnI[IfIndx]) {			/* if it's open */
		status = sys$close(&IFiles[IfIndx].IFab);
		if (status != RMS$_NORMAL) {
			ZErMsg(status, IFiles[IfIndx].IFab.fab$l_stv);
			if (!$VMS_STATUS_SUCCESS(status)) {
				ErrMsg(ERR_UCI);
#if DEBUGGING
				DbgFMs(2,DbgFNm,"dying");
#endif
				exit(EXIT_FAILURE);
			}
		}
		IsOpnI[IfIndx] = FALSE;
	}
	DBGFEX(2,DbgFNm,"");
}
/*****************************************************************************
	ZOClDe()
	This function closes and deletes the current output stream.  It must
	1.  if no current output stream is defined,  simply return
	2.  close the output stream
	3.  delete the file just closed
*****************************************************************************/
VVOID ZOClDe(DEFAULT OfIndx)		/* close and delete output file */
{
	unsigned int status;
	DBGFEN(2,"ZOClDe","");
	if (IsOpnO[OfIndx]) {		/* if output stream is open */
		OFiles[OfIndx].OFab.fab$l_fop |= FAB$M_DLT;
		status = sys$close(&OFiles[OfIndx].OFab);
		if (status != RMS$_NORMAL) {
			ZErMsg(status, OFiles[OfIndx].OFab.fab$l_stv);
			if (!$VMS_STATUS_SUCCESS(status)) {
				ErrMsg(ERR_UCD);
				exit(EXIT_FAILURE);
			}
		}
		IsOpnO[OfIndx] = FALSE;
	}
	DBGFEX(2,DbgFNm,"");
}
/*****************************************************************************
	ZOClos()
	This function closes the current output stream.  It is only called
when an output stream is defined.  It must
	1.  flush output to the stream,  if neccessary
	2.  close the stream
	3.  set OFile to -1
*****************************************************************************/
VVOID ZOClos(DEFAULT OfIndx)	/* close output file */
{
	unsigned int status;
	DBGFEN(2,"ZOClos","");
	if (IsOpnO[OfIndx]) {			/* if it's open */
		status = sys$close(&OFiles[OfIndx].OFab);
		if (status != RMS$_NORMAL) {
			ZErMsg(status, OFiles[OfIndx].OFab.fab$l_stv);
			if (!$VMS_STATUS_SUCCESS(status)) {
				ErrMsg(ERR_UCO);
				exit(EXIT_FAILURE);
			}
		}
		IsOpnO[OfIndx] = FALSE;
	}
	DBGFEX(2,DbgFNm,"");
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
DEFAULT	ZOpInp(			/* open input file */
	DEFAULT IfIndx,		/* index into file data block array IFiles */
	BOOLEAN	EIFile,		/* is it a macro file? (hunt for it) */
	BOOLEAN RepFNF)		/* report "file not found" error? */
{
    char	ExpFBf[NAM$C_MAXRSS];	/* expanded filename buffer */
    struct	FAB	*IF;		/* pointer to FAB */
    struct	NAM	*IN;		/* pointer to NAM block */
    struct	RAB	*IR;		/* pointer to RAB */
    unsigned int status;		/* temporary status */
#if DEBUGGING
    static char *DbgFNm = "ZOpInp";
    sprintf(DbgSBf,", FBf = \"%.*s\"", (int)(FBfPtr-FBfBeg), FBfBeg);
    DbgFEn(2,DbgFNm,DbgSBf);
#endif
    IN = &IFiles[IfIndx].INam;
    *IN = cc$rms_nam;			/* initialize NAM defaults */
    IN->nam$l_esa = ExpFBf;		/* expanded file spec buffer addr. */
    IN->nam$b_ess = sizeof ExpFBf;	/* expanded file spec buffer size */
    IN->nam$l_rsa = FBfBeg;		/* resultant file spec buffer addr. */
    IN->nam$b_rss = NAM$C_MAXRSS;	/* resultant file spec buffer size */
    IF = &IFiles[IfIndx].IFab;
    *IF = cc$rms_fab;			/* initialize FAB defaults */
    IF->fab$l_nam = IN;			/* address of NAM block */
    IF->fab$b_fac = FAB$M_GET;		/* file access = read only */
    IF->fab$b_shr = FAB$M_SHRGET;	/* allow others to read the file */
    if (EIFile) {			/* if use default file type ".TEC" */
	IF->fab$l_dna = ".TEC";		/* default filename */
	IF->fab$b_dns = 4;		/* default filename size */
    }
    IF->fab$l_fna = FBfBeg;		/* set file name address */
    IF->fab$b_fns = FBfPtr - FBfBeg;	/* set file name size */
    IR = &IFiles[IfIndx].IRab;
    *IR = cc$rms_rab;			/* initialize RAB defaults */
    IR->rab$l_fab = IF;			/* address of associated FAB */
    IR->rab$b_rac = RAB$C_SEQ;		/* record access mode = sequential */
    status = sys$open(IF);		/* open the file */
    if (status != RMS$_NORMAL) {	/* if failed for some reason */
	if ((status == RMS$_FNF) && EIFile) {	/* if couldn't find EI file */
	    charptr dummyp = NULL;
	    char TmpBfr[NAM$C_MAXRSS];
	    ptrdiff_t TmpLen = FBfPtr-FBfBeg;
	    MEMMOVE(TmpBfr, FBfBeg, TmpLen);	/* save file name */
	    if (ZClnEG(EG_LIB, GET_VAL, dummyp) != -1) {  /* get dir spec */
		goto open_failed;
	    }
	    MEMMOVE(FBfPtr, TmpBfr, TmpLen);	/* append name to dir spec */
	    FBfPtr += TmpLen;
	    IF->fab$b_fns = FBfPtr - FBfBeg;	/* set file name size */
	    status = sys$open(IF);		/* open the file */
	    if (status != RMS$_NORMAL) {
		goto open_failed;
	    }
	} else {
open_failed:
	    if (!RepFNF && (status == RMS$_FNF)) {
	        DBGFEX(2,DbgFNm,"FILENF");
		return FILENF;
	    }
	    ZErMsg(status, IF->fab$l_stv);
	    if (!$VMS_STATUS_SUCCESS(status)) {
	        DBGFEX(2,DbgFNm,"FAILURE");
		return FAILURE;
	    }
	}
    }
    FBfPtr = FBfBeg + IN->nam$b_rsl;		/* resultant name length */
    status = sys$connect(IR);			/* connect RAB to FAB */
    if (status != RMS$_NORMAL) {
	ZErMsg(status, IR->rab$l_stv);
	if (!$VMS_STATUS_SUCCESS(status)) {
	    status = sys$close(IF);		/* close the file */
	    	DBGFEX(2,DbgFNm,"FAILURE");
	    	return FAILURE;
	}
    }
    IFiles[IfIndx].leftover_input_exists = FALSE;
    DBGFEX(2,DbgFNm,"SUCCESS");
    return SUCCESS;
}
/*****************************************************************************
	ZOpOut()
	This function creates (and opens) an output file.  The name of
the file to be created is pointed to by FBfBeg.  FBfPtr points to the
character following the last character of the file name.
*****************************************************************************/
DEFAULT ZOpOut(DEFAULT OfIndx, BOOLEAN RepErr)	/* open output file */
{
	char	ExpFBf[NAM$C_MAXRSS];	/* expanded filename buffer */
	struct	FAB	*OF;
	struct	NAM	*ON;
	struct	RAB	*OR;
	unsigned int status;
#if DEBUGGING
	static char *DbgFNm = "ZOpOut";
	sprintf(DbgSBf,", FBf = \"%.*s\"",(int)(FBfPtr-FBfBeg),FBfBeg);
	DbgFEn(2,DbgFNm,DbgSBf);
#endif
	ON = &OFiles[OfIndx].ONam;
	*ON = cc$rms_nam;		/* initialize NAM defaults */
	ON->nam$l_esa = ExpFBf;		/* expanded file spec buffer address */
	ON->nam$b_ess = sizeof ExpFBf;	/* expanded file spec buffer size */
	ON->nam$l_rsa = FBfBeg;		/* resultant file spec buffer address */
	ON->nam$b_rss = NAM$C_MAXRSS;	/* resultant file spec buffer size */
	OF = &OFiles[OfIndx].OFab;
	*OF = cc$rms_fab;		/* initialize FAB defaults */
	OF->fab$b_fac = FAB$M_PUT;	/* file access = write only */
	OF->fab$b_org = FAB$C_SEQ;	/* file organization = sequential */
	OF->fab$b_rat = FAB$M_CR;	/* carriage return record attribute */
	OF->fab$b_rfm = FAB$C_VAR;	/* variable length record format */
	OF->fab$b_shr = FAB$M_NIL;	/* no sharing */
	OF->fab$l_nam = ON;		/* address of NAM block */
	OF->fab$l_fna = FBfBeg;		/* file name address */
	OF->fab$b_fns = FBfPtr - FBfBeg;	/* file name size */
	OR = &OFiles[OfIndx].ORab;
	*OR = cc$rms_rab;		/* initialize RAB defaults */
	OR->rab$l_fab = OF;		/* address of associated FAB */
	OR->rab$b_rac = RAB$C_SEQ;	/* record access mode = sequential */
	status = sys$create(OF);		/* create the file */
	if (status != RMS$_NORMAL) {
		if (RepErr) {
			ZErMsg(status, OF->fab$l_stv);
		}
		if (!$VMS_STATUS_SUCCESS(status)) {
			DBGFEX(2,DbgFNm,"FAILURE, sys$create failed");
			return FAILURE;
		}
	}
	FBfPtr = FBfBeg + ON->nam$b_rsl;	/* resultant name length */
	status = sys$connect(OR);		/* connect RAB to FAB */
	if (status != RMS$_NORMAL) {
		if (RepErr) {
			ZErMsg(status, OR->rab$l_stv);
		}
		if (!$VMS_STATUS_SUCCESS(status)) {
			OF->fab$l_fop |= FAB$M_DLT;	/* delete on close */
			status = sys$close(OF);		/* close the file */
			DBGFEX(2,DbgFNm,"FAILURE, sys$connect failed");
			return FAILURE;
		}
	}
	DBGFEX(2,DbgFNm,"SUCCESS");
	return SUCCESS;
}
/*****************************************************************************
	ZPrsCL()
	This function parses the command line.  It does so using the classic
TECO-11 method:  use a TECO macro to do it!  The macros is stored in array
"clpars".  The macro is in file "clpars.h",  and is over 3k long.  The VMS C
compiler (version 3.0) can't handle a string literal that's that long, so
it loads a piece of memory 1 line at a time.
*****************************************************************************/
/*
  Under VAX/VMS,  the logic is:
	load q-register Z with the command line
	load q-register Y with a command-line parsing macro
	IF logical name "TECO" is defined THEN
		do an EITECO$$
	ELSEIF the file "SYS$LOGIN:TECO.TEC" exists, THEN
		do an EISYS$LOGIN:TECO.TEC$$
	ELSE
		do an MY$$
*/
VVOID ZPrsCL(			/* parse the command line */
	int argc,
	char *argv[])
{
	char	MngBeg[4+NAM$C_MAXRSS];	/* temporary command string buffer */
	charptr	MngPtr;			/* pointer into mung buffer */
	charptr	TmpPtr;			/* temporary pointer */
	BOOLEAN filefound;
	short	length;
	unsigned int status;
	char	CLPars_name[NAM$C_MAXRSS];
	$DESCRIPTOR(teco_desc,"TECO");
	$DESCRIPTOR(fil_desc,"SYS$LOGIN:TECO.TEC");
	$DESCRIPTOR(res_desc, CLPars_name);
	int	i;
	SIZE_T	line_len;
	char	cmd_line[1024];
	$DESCRIPTOR(cmd_line_desc, cmd_line);
	DBGFEN(2,"ZPrsCL","");
/*
 * If the command line contains arguments,  load them into Q-register Z.
 */
	status = lib$get_foreign(		/* get command line */
		&cmd_line_desc,			/* returned string */
		0,				/* user prompt */
		&cmd_line_desc.dsc$w_length,	/* returned string length */
		0);				/* force prompt */
	if (!$VMS_STATUS_SUCCESS(status)) {
		lib$stop(status);
	}
	if (cmd_line_desc.dsc$w_length != 0) {
		QR = &QRgstr[35];		/* 35 = q-register Z */
		if (MakRom((SIZE_T)cmd_line_desc.dsc$w_length) == FAILURE)
			exit(EXIT_FAILURE);
		MEMMOVE(QR->Start,			/* destination */
			cmd_line,			/* source */
			cmd_line_desc.dsc$w_length);	/* length */
		QR->End_P1 += cmd_line_desc.dsc$w_length;
	}
/*
 * load imbedded command-line parsing macro into q-register Y
 */
	QR = &QRgstr[34];			/* 34 = q-register Y */
	if (MakRom((SIZE_T)CLPARS_LEN) == FAILURE) {
		exit(EXIT_FAILURE);
	}
	for (i=0; i<CLPARS_LINES; i++) {
		line_len = strlen(clpars[i]);
		MEMMOVE(QR->End_P1, clpars[i], line_len);
		QR->End_P1 += line_len;		/* length of q-reg text */
	}
/*
 * If the logical name "TECO" is defined,  then do EITECO$$.  If not,  look
 * for file SYS$LOGIN:TECO.TEC, and "EI" it if it exists. If it also doesn't
 * exist,  just execute the macro in q-register Y.
 */
	status = sys$trnlog(	&teco_desc,	/* logical name */
				&length,	/* returned string length */
				&res_desc,	/* returned string buffer */
				0,		/* logical name table */
				0,		/* access mode */
				3);		/* table search mask */
	if (status == SS$_NORMAL) {		/* if logical exists */
		filefound = TRUE;		/* do EITECO$$ later*/
	} else if (status == SS$_NOTRAN) {	/* if logical doesn't exist */
		int	context;
		int	user_flags;
		context = 0;
		user_flags = 1;
		status = lib$find_file(	&fil_desc,	/* file spec */
					&res_desc,	/* resultant spec */
					&context,	/* context */
					0,		/* default spec */
					0,		/* related spec */
					0,		/* stv address */
					&user_flags);	/* user flags */
		if (status == RMS$_NORMAL) {
			length = res_desc.dsc$w_length;
			filefound = TRUE;
		} else {
			if (status != RMS$_FNF) {
				if (!$VMS_STATUS_SUCCESS(status)) {
					lib$stop(status);
				}
			}
			filefound = FALSE;
		}
		status = lib$find_file_end(&context);
		if (!$VMS_STATUS_SUCCESS(status)) {
			lib$stop(status);
		}
	} else {
		lib$stop(status);		/* terminate */
	}
/*
 * If there is a user-defined command-parsing macro file,  do an EI on it.
 * Otherwise,  execute q-register Y.
 */
	MngPtr = MngBeg;
	if (filefound) {
		*MngPtr++ = 'e';
		*MngPtr++ = 'i';
		TmpPtr = res_desc.dsc$a_pointer;
		for (;length>0;length--) {
			*MngPtr++ = *TmpPtr++;
		}
	} else {
		*MngPtr++ = 'm';
		*MngPtr++ = 'y';
	}
	*MngPtr++ = ESCAPE;
	*MngPtr = ESCAPE;
	CBfPtr = MngBeg;			/* command string start */
	CStEnd = MngPtr;			/* command string end */
	EStTop = EStBot;			/* clear expression stack */
	ExeCSt();				/* execute command string */
	DBGFEX(2,DbgFNm,"");
}
/*****************************************************************************
	ZPWild()
	This function presets the wildcard lookup filename.  It is called
when the user executes an ENfilename$ command.  Later executions of the EN$
command will cause the ZSWild function to be called to return successive
wildcard matches.
*****************************************************************************/
DEFAULT ZPWild(void)		/* preset the wildcard lookup filename */
{
	unsigned int status;
	static BOOLEAN first_time_called = TRUE;
#if DEBUGGING
	static char *DbgFNm = "ZPWild";
	sprintf(DbgSBf, ", FBf = \"%.*s\"", (int)(FBfPtr-FBfBeg), FBfBeg);
	DbgFEn(1,DbgFNm,DbgSBf);
#endif
	if (first_time_called) {
		first_time_called = FALSE;
		WNam = cc$rms_nam;		/* initialize NAM defaults */
		WNam.nam$l_esa = WBfExp;	/* expanded file spec buf */
		WNam.nam$b_ess = NAM$C_MAXRSS;	/* expanded f. s. buf size */
		WNam.nam$l_rsa = FBfBeg;	/* res file spec buf */
		WNam.nam$b_rss = NAM$C_MAXRSS;	/* res file spec buf size */
		WFab = cc$rms_fab;		/* initialize FAB defaults */
		WFab.fab$l_fna = FBfBeg;	/* addr. wildcard file spec */
		WFab.fab$l_nam = &WNam;		/* address of NAM block */
		WFab.fab$b_fac = FAB$M_GET;	/* file access = read only */
	}
	WFab.fab$b_fns = FBfPtr - FBfBeg;	/* file name size */
	status = sys$parse(&WFab);
	if (status != RMS$_NORMAL) {
		ZErMsg(status, WFab.fab$l_stv);
		if (!$VMS_STATUS_SUCCESS(status)) {
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
/*****************************************************************************
	ZRaloc()
	This function performs the standard C library function realloc.
*****************************************************************************/
voidptr	ZRaloc(voidptr OldBlk, SIZE_T NewSiz)
{
	return realloc(OldBlk, NewSiz);
}
/*****************************************************************************
	ZRdLin()
        This function reads a line from a file.  It is passed a buffer, the
size of the buffer, a file pointer and a pointer to a place to leave the
length of the line.
	What this function does may be confusing, so here's a description
of it from the Standard TECO manual, Appendix G (VAX/VMS Operating
Characteristics), section 11: "File Record Format".  TECO-C supports only
implied-carriage-control files under VMS,  so I removed the discussion of
other formats.
     Files-11 files are record structured, while TECO'S text buffer is
     ASCII stream.   Thus TECO must make format conversions when reading
     and writing files.  While reading a file, the records are packed
     into the buffer.  TECO inserts a carriage return and line feed
     after each record to make each record appear as a line of text in
     the buffer, unless the record ends with ESCAPE, carriage return,
     line feed, vertical tab, or form feed.  A record ending in form
     feed is interpreted as an end of page mark;  it stops the read
     operation and the form feed is not entered in the buffer. The
     portion of the record after the form feed, if any, is saved for
     the next input command.
File output is similarly screwy.  See the comment for the ZWrBfr function.
*****************************************************************************/
DEFAULT ZRdLin(			/* read a line */
	charptr ibuf,		/* where to put the line */
	ptrdiff_t ibuflen,	/* length of buf */
	int IfIndx,		/* index into IFiles[] */
	DEFAULT *retlen)	/* returned length of the line */
{
	unsigned int status;
	struct IFile_struct *ifile = &IFiles[IfIndx];
	char *ffptr;
	char lastchr;
	DBGFEN(3,"ZRdLin","");
/*
 * Get the next input line into ibuf,  and length into *retlen.
 */
	if (ifile->leftover_input_exists) {
		memcpy(ibuf,
		       ifile->leftover_input,
		       ifile->leftover_size);
		free(ifile->leftover_input);
		*retlen = ifile->leftover_size;
		ifile->leftover_input_exists = FALSE;
	} else {
		ifile->IRab.rab$l_ubf = ibuf;	/* input buffer */
		ifile->IRab.rab$w_usz = (ibuflen > 65535) ? 65535
							  : ibuflen;
		status = sys$get(&ifile->IRab);	/* get a record */
		if (status == RMS$_EOF) {		/* if end of file */
			DBGFEX(3,DbgFNm,"SUCCESS, hit end-of-file");
			IsEofI[IfIndx] = TRUE;
			return SUCCESS;
		}
		if (status != RMS$_NORMAL) {
			ZErMsg(status, IRab.rab$l_stv);
			DBGFEX(3,DbgFNm,"FAILURE");
			return FAILURE;
		}
		*retlen = ifile->IRab.rab$w_rsz;
	}
/*
 * Now do all the weird handling of special characters (see the main comment
 * for this function).
 */
	if (*retlen > 0) {			/* if not an empty line */
		if ((ffptr = memchr(ibuf, FORMFD, *retlen)) != NULL) {
			ifile->leftover_size = *retlen - (ffptr - ibuf + 1);
			if (ifile->leftover_size > 0) {
				if ((ifile->leftover_input =
				      malloc(ifile->leftover_size))==NULL) {
					ErrMsg(ERR_MEM);
					DBGFEX(3,DbgFNm,"FAILURE");
					return FAILURE;
				}
				memcpy(ifile->leftover_input,
				       ffptr+1,
				       ifile->leftover_size);
				ifile->leftover_input_exists = TRUE;
			}
			FFPage = -1;		/* set "form feed hit" flag */
			*retlen = ffptr - ibuf;
			DBGFEX(3,DbgFNm,"SUCCESS, hit formfeed");
			return SUCCESS;
		}
		lastchr = ibuf[(*retlen)-1];
		if ((lastchr == ESCAPE) ||
		    (lastchr == CRETRN) ||
		    (lastchr == LINEFD) ||
		    (lastchr == VRTTAB)) {
			DBGFEX(3,DbgFNm,"SUCCESS, hit ESC, CR, LF or VT");
			return SUCCESS;
		}
	}
	ibuf[*retlen] = CRETRN;		/* append carriage return */
	*retlen += 1;
	ibuf[*retlen] = LINEFD;		/* append line feed */
	*retlen += 1;
	DBGFEX(3,DbgFNm,"SUCCESS");
	return SUCCESS;
}
/*****************************************************************************
	ZScrOp()
	This function is called to perform special screen functions.
*****************************************************************************/
VVOID ZScrOp(DEFAULT OpCode)		/* do a screen operation */
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
	if (CrType == UNTERM)		/* if unknown terminal type */
		return;			/* can't do screen operations */
/*
 * The numbering used for CrType comes from TECO-11.  Convert it to get an
 * index into the string arrays.
 */
	index = map[CrType];
	switch (OpCode) {
	    case SCR_CUP:  ZDspBf(CUP[index].strt, CUP[index].len);  break;
	    case SCR_EEL:  ZDspBf(EEL[index].strt, EEL[index].len);  break;
	    case SCR_ROF:  ZDspBf(ROF[index].strt, ROF[index].len);  break;
	    case SCR_RON:  ZDspBf(RON[index].strt, RON[index].len);  break;
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
    unsigned int status;
    struct tt_mode_iosb_struct mode_iosb;
    if (!TOChan)				/* if it's not a terminal */
	return(SUCCESS);
/*
 * Modify the terminal characteristics.
 */
    switch (TTWhat) {
	case TT8BIT:	tt_chars.lw2.tt$v_eightbit = TTVal;  break;
	case TTWIDTH:	tt_chars.nbr_columns       = TTVal;  break;
	case TTHEIGHT:	tt_chars.lw2.tt$v_page     = TTVal;  break;
    }
/*
 * Set the new terminal characteristics.
 */
    status = sys$qiow(	0,			/* event flag number */
			TOChan,			/* channel */
			IO$_SETMODE,		/* I/O function */
			&mode_iosb,		/* I/O status block */
			0,			/* AST routine address */
			0,			/* AST parameter */
			&tt_chars,		/* p1 */
			0,			/* p2 */
			0,			/* p3 */
			0,			/* p4 */
			0,			/* p5 */
			0);			/* p6 */
    if (!$VMS_STATUS_SUCCESS(status)) {
	lib$stop(status);
    }
    if (!$VMS_STATUS_SUCCESS(mode_iosb.io_status)) {
	lib$stop(mode_iosb.io_status);
    }
    return SUCCESS;
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
DEFAULT ZSWild(void)		/* search for next wildcard filename */
{
	unsigned int status;
	DBGFEN(1,"ZSWild","");
	status = sys$search(&WFab);		/* search for file */
	if ((status == RMS$_FNF) ||		/* if file not found or */
	    (status == RMS$_NMF)) {		/* no more files */
		DBGFEX(1,DbgFNm,"FILENF");
		return FILENF;
	}
	if (status != RMS$_NORMAL) {
		ZErMsg(status, WFab.fab$l_stv);
		if (!$VMS_STATUS_SUCCESS(status)) {
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	FBfPtr = FBfBeg + WNam.nam$b_rsl;	/* resultant name length */
#if DEBUGGING
	sprintf(DbgSBf,"SUCCESS, FBf = \"%.*s\"",(int)(FBfPtr-FBfBeg),FBfBeg);
	DbgFEx(1,DbgFNm,DbgSBf);
#endif
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
*****************************************************************************/
/*****************************************************************************
	On the VAX,  TECO-C can be run under two environments: interactive or
non-interactive (like batch).  In an interactive session,  it uses the
terminal for input and output.  In non-interactive mode,  it goes through RMS.
The difference is the way the I/O completes.
	In interactive mode,  each character the user types is immediately
received by TECO-C (forget type-ahead for now).  This allows TECO-C to echo
the character immediately.
	In non-interactive mode,  input comes from a file and output goes to
another file,  so RMS is used.  RMS returns a bunch of characters to TECO-C
when the user types a "terminator" character, which is defined by VMS.
In non-interactive mode, it isn't important that TECO-C echo characters
immediately.
	Because both modes must be supported, there are two separate i/o
systems in the code.  TECO-C determines if it is in interactive mode when
this function is called.  It sets TIChan to be the a channel associated with
the terminal for interactive mode,  or leaves TIChan unset (zero) for
non-interactive mode.  Other functions (ZDSpCh, ZDspBf, ZChin) test TIChan and
use either QIOs or RMS to perform I/O.
	Under VMS,  input comes from SYS$INPUT,  output goes to SYS$OUTPUT,
and control-C's come from SYS$COMMAND.  Control-Y's are not explicitly
handled by TECOC.  If SYS$COMMAND is not a terminal device,  then control-C's
are not enabled.
*****************************************************************************/
int	devclass;			/* device class */
union	ttdef devdepend;		/* device dependent attributes */
union	tt2def devdepend2;		/* device dependent attributes */
readonly $DESCRIPTOR(ter_c_desc,"SYS$COMMAND");
readonly $DESCRIPTOR(ter_i_desc,"SYS$INPUT");
readonly $DESCRIPTOR(ter_o_desc,"SYS$OUTPUT");
struct {
	short	buflen1;		/* buffer length */
	short	itmcode1;		/* item code */
	charptr	buffer1;		/* buffer address */
	short	*retlen1;		/* returned length */
	short	buflen2;		/* buffer length */
	short	itmcode2;		/* item code */
	charptr	buffer2;		/* buffer address */
	short	*retlen2;		/* returned length */
	short	buflen3;		/* buffer length */
	short	itmcode3;		/* item code */
	charptr	buffer3;		/* buffer address */
	short	*retlen3;		/* returned length */
	short	buflen4;		/* buffer length */
	short	itmcode4;		/* item code */
	charptr	buffer4;		/* buffer address */
	short	*retlen4;		/* returned length */
	int	termin;			/* item list terminator */
} itmlst = {
		4,			/* buffer length */
		DVI$_DEVCLASS,		/* device class */
		&devclass,		/* buffer address */
		0,			/* no returned length */
		4,			/* buffer length */
		DVI$_DEVDEPEND,		/* device dependent data */
		&devdepend,		/* buffer address */
		0,			/* no returned length */
		4,			/* buffer length */
		DVI$_DEVDEPEND2,	/* device dependent data */
		&devdepend2,		/* buffer address */
		0,			/* no returned length */
		0			/* item list terminator */
};
unsigned short output_sys_vfc = 1;
unsigned int rms_status;
unsigned int status;
VVOID	CntrlC(void);			/* make the compiler happy */
static VVOID enable_ctrl_c_ast(void)
{
	unsigned int status;
	struct tt_mode_iosb_struct mode_iosb;
	status = sys$qiow(	0,		/* event flag number */
				TCChan,		/* channel */
				IO$_SETMODE|
				IO$M_CTRLCAST,	/* I/O function */
				&mode_iosb,	/* I/O status block */
				0,		/* AST routine address */
				0,		/* AST parameter */
				CntrlC,		/* control-C routine */
				0,		/* p2 */
				0,		/* p3 */
				0,		/* p4 */
				0,		/* p5 */
				0);		/* p6 */
	if (!$VMS_STATUS_SUCCESS(status)) {
		lib$stop(status);
	}
	if (!$VMS_STATUS_SUCCESS(mode_iosb.io_status)) {
		lib$stop(mode_iosb.io_status);
	}
}
/*****************************************************************************
	This function is called whenever a control-C is typed by the user.
It is called asynchronously.
*****************************************************************************/
static VVOID CntrlC(void)			/* control-C AST routine */
{
	unsigned int status;
	if (EtFlag & ET_TRAP_CTRL_C) {		/* if user wants it */
		EtFlag &= ~ET_TRAP_CTRL_C;	/* turn off bit */
	} else {
		if (EtFlag & ET_MUNG_MODE) {	/* if in MUNG mode */
			TAbort(EXIT_SUCCESS);
		}
		GotCtC = TRUE;			/* set "stop soon" flag */
	}
	status = sys$cancel((long)TOChan);	/* cancel current output */
	if (!$VMS_STATUS_SUCCESS(status)) {
		lib$stop(status);
	}
	enable_ctrl_c_ast();			/* re-enable the AST */
}
static VVOID open_terminal_input(void)
{
        struct getxxx_iosb_struct getdvi_iosb;
	status = sys$getdviw(	1,		/* event flag number */
				0,		/* channel */
				&ter_i_desc,	/* device name */
				&itmlst,	/* item list */
				&getdvi_iosb,	/* i/o status block */
				0,		/* AST routine address */
				0,		/* AST parameter */
				0);		/* reserved by DEC */
	if (!$VMS_STATUS_SUCCESS(status)) {
		devclass = DC$_MISC;
	} else {
		if ((!$VMS_STATUS_SUCCESS(getdvi_iosb.io_status)) &&
		    (getdvi_iosb.io_status != SS$_CONCEALED)) {
			lib$stop(getdvi_iosb.io_status);
		}
	}
	EtFlag = ET_READ_LOWER;
#if VIDEO
	EtFlag |= ET_WAT_SCOPE;
#endif
	if (devclass == DC$_TERM) {		    /* if it's a terminal */
		if (!devdepend.tt$v_lower) {	    /* if has no lowercase */
			EtFlag &= ~ET_READ_LOWER;   /* don't read lowercase */
		}
		status = sys$assign(	&ter_i_desc,	/* device name */
					&TIChan,	/* channel */
					0,		/* access mode */
					0);		/* mailbox name */
		if (!$VMS_STATUS_SUCCESS(status)) {
			lib$stop(status);
		}
		return;
	}
	TIBERc = TIBBeg = ZAlloc((SIZE_T)SOBSIZE);
	TIBEnd = (TIBBeg + SOBSIZE) - 1;
	TIBPtr = TIBERc;			/* causes the initial read */
	TIFab = cc$rms_fab;			/* initialize FAB defaults */
	TIFab.fab$b_fac = FAB$M_GET;		/* file access = read only */
	TIFab.fab$l_fna = ter_i_desc.dsc$a_pointer;
	TIFab.fab$b_fns = ter_i_desc.dsc$w_length;
	TIFab.fab$l_fop = FAB$M_INP |		/* this is SYS$INPUT and */
			  FAB$M_SQO;		/* sequential access only */
	rms_status = sys$open(&TIFab);		/* open terminal input */
	if (rms_status != RMS$_NORMAL) {
		lib$stop(rms_status, TIFab.fab$l_stv);
	}
	TIRab = cc$rms_rab;			/* initialize RAB defaults */
	TIRab.rab$l_fab = &TIFab;		/* address of associated FAB */
	TIRab.rab$b_rac = RAB$C_SEQ;		/* rec. access = sequential */
	TIRab.rab$l_rop = RAB$M_LOC |		/* use locate mode and */
			  RAB$M_RAH;		/* read ahead */
	TIRab.rab$l_ubf = TIBBeg;		/* input buffer */
	TIRab.rab$w_usz = SOBSIZE;		/* input buffer size */
	rms_status = sys$connect(&TIRab);	/* connect terminal input */
	if (rms_status != RMS$_NORMAL) {
		lib$stop(rms_status, TIRab.rab$l_stv);
	}
}
static VVOID open_terminal_output(void)
{
    struct getxxx_iosb_struct getdvi_iosb;
    struct tt_mode_iosb_struct mode_iosb;
    status = sys$getdviw(	1,		/* event flag number */
				0,		/* channel */
				&ter_o_desc,	/* device name */
				&itmlst,	/* item list */
				&getdvi_iosb,	/* i/o status block */
				0,		/* AST routine address */
				0,		/* AST parameter */
				0);		/* reserved by DEC */
    if (!$VMS_STATUS_SUCCESS(status)) {
	devclass = DC$_MISC;
    } else if ((!$VMS_STATUS_SUCCESS(getdvi_iosb.io_status)) &&
	       (getdvi_iosb.io_status != SS$_CONCEALED)) {
	lib$stop(getdvi_iosb.io_status);
    }
    if (devclass == DC$_TERM) {			/* if it's a terminal */
	if (!devdepend.tt$v_lower)		/* if won't show lowercase */
	    EuFlag = EU_LOWER;			/* set lowercase */
	if (devdepend.tt$v_scope)		/* if scope */
	    EtFlag |= ET_SCOPE;			/* set scope bit */
	if (devdepend.tt$v_eightbit)		/* can show 8-bit */
	    EtFlag |= ET_EIGHTBIT;
	if (devdepend2.tt2$v_deccrt2) {		/* VT200 compatible? */
	    CrType = VT200;
	    EtFlag |= ET_VT200;			/* VT200 mode */
	    EtFlag |= ET_ACCENT_GRAVE;		/* VT200 mode */
	} else if (devdepend2.tt2$v_deccrt)	/* VT100 compatible? */
	    CrType = VT100;
	else if (devdepend2.tt2$v_ansicrt)	/* ANSI compatible? */
	    CrType = VK100;
	else					/* default is VT52 */
	    CrType = VT52;
	status = sys$assign(	&ter_o_desc,	/* device name */
				&TOChan,	/* channel */
				0,		/* access mode */
				0);		/* mailbox name */
	if (!$VMS_STATUS_SUCCESS(status)) {
		lib$stop(status);
	}
/*
 * Get the current terminal characteristics so that later setmodes work.
 */
	status = sys$qiow(	0,		/* event flag number */
				TOChan,		/* channel */
				IO$_SENSEMODE,	/* I/O function */
				&mode_iosb,	/* I/O status block */
				0,		/* AST routine address */
				0,		/* AST parameter */
				&tt_chars,	/* p1 */
				0,		/* p2 */
				0,		/* p3 */
				0,		/* p4 */
				0,		/* p5 */
				0);		/* p6 */
	if (!$VMS_STATUS_SUCCESS(status)) {
		lib$stop(status);
	}
	if (!$VMS_STATUS_SUCCESS(mode_iosb.io_status)) {
		lib$stop(mode_iosb.io_status);
	}
#if VIDEO
	HtSize = tt_chars.nbr_columns;		/* horizontal size */
	VtSize = tt_chars.lw2.tt$v_page;	/* vertical size */
#endif
	return;
    } else {					/* else not a terminal */
	TOBEnd = &TOBBeg[SOBSIZE]-1;
	TOBPtr = &TOBBeg[0];
	TOFab = cc$rms_fab;			/* initialize FAB defaults */
	TOFab.fab$b_fac = FAB$M_PUT;		/* file access = write only */
	TOFab.fab$l_fna = ter_o_desc.dsc$a_pointer;
	TOFab.fab$b_fns = ter_o_desc.dsc$w_length;
	TOFab.fab$b_fsz = 2;			/* fixed size cntrl area=2 */
	TOFab.fab$b_org = FAB$C_SEQ;		/* organization=sequential */
	TOFab.fab$b_rat = FAB$M_PRN;		/* print file format */
	TOFab.fab$b_rfm = FAB$C_VFC;		/* variable w/fixed control */
	rms_status = sys$create(&TOFab);	/* open terminal output */
	if (rms_status != RMS$_NORMAL) {
	    lib$stop(rms_status, TOFab.fab$l_stv);
	}
	TORab = cc$rms_rab;			/* initialize RAB defaults */
	TORab.rab$l_fab = &TOFab;		/* addr of associated FAB */
	TORab.rab$l_rhb = &output_sys_vfc;	/* print control buffer */
	TORab.rab$l_rop = RAB$M_WBH;		/* write behind */
	TORab.rab$l_rbf = &TOBBeg[0];		/* output buffer */
	rms_status = sys$connect(&TORab); /* connect terminal output */
	if (rms_status != RMS$_NORMAL) {
	    lib$stop(rms_status, TORab.rab$l_stv);
	}
    }
}
static VVOID open_terminal_command(void)
{
        struct getxxx_iosb_struct getdvi_iosb;
	status = sys$getdviw(	1,		/* event flag number */
				0,		/* channel */
				&ter_c_desc,	/* device name */
				&itmlst,	/* item list */
				&getdvi_iosb,	/* i/o status block */
				0,		/* AST routine address */
				0,		/* AST parameter */
				0);		/* reserved by DEC */
	if (!$VMS_STATUS_SUCCESS(status)) {
		devclass = DC$_MISC;
	} else {
		if ((!$VMS_STATUS_SUCCESS(getdvi_iosb.io_status)) &&
		    (getdvi_iosb.io_status != SS$_CONCEALED)) {
			lib$stop(getdvi_iosb.io_status);
		}
	}
	if (devclass == DC$_TERM) {			/* if terminal */
		status = sys$assign(	&ter_c_desc,	/* device name */
					&TCChan,	/* channel */
					0,		/* access mode */
					0);		/* mailbox name */
		if (!$VMS_STATUS_SUCCESS(status)) {
			lib$stop(status);
		}
		signal(SIGINT,SIG_IGN);		/* ignore SIGINT */
		enable_ctrl_c_ast();		/* enable control-C */
	}
}
VVOID ZTrmnl(void)		/* set up I/O to the terminal */
{
	open_terminal_input();		/* open SYS$INPUT */
	open_terminal_output();		/* open SYS$OUTPUT */
	open_terminal_command();	/* open SYS$COMMAND */
}
/*****************************************************************************
	ZVrbos()
	This function displays the verbose form of error messages.
*****************************************************************************/
VVOID ZVrbos(WORD ErrNum, char *ErMnem)
{
	static char HlpStr[12] = " Errors xxx";
	MEMMOVE(&HlpStr[8], ErMnem, (SIZE_T)3);
	ZHelp(HlpStr, &HlpStr[10], FALSE, FALSE);
}
/*****************************************************************************
	ZWrBfr()
	This function writes a buffer to a file,  one line at a time.  It is
passed an output file index and pointers to the beginning and end of the
buffer to be output.
	On output, TECO scans the text buffer for carriage return,  line
feed, vertical tab and form feed characters.  Each such character delimits
the end of an output record.  If the line ends with exactly CR/LF, the CR/LF
are not output with the record,  otherwise the record is output in its
entirety.  If a record ends with a CR/LF preceded by an ESCAPE,  then the
ESCAPE and the CR/LF are output with the record.
*****************************************************************************/
DEFAULT ZWrBfr(
	DEFAULT OfIndx,		/* index into OFiles array */
        charptr BfrBeg,		/* address of output buffer beginning */
	charptr BfrEnd)		/* address of output buffer end */
{
	charptr	BfrPtr = BfrBeg;	/* output buffer pointer */
	ptrdiff_t line_len;		/* length of current output line */
#if DEBUGGING
	static char *DbgFNm = "ZWrBfr";
	sprintf(DbgSBf,"OfIndx = %d, BfrBeg = %ld, BfrEnd = %ld",
		OfIndx, Zcp2ul(BfrBeg), Zcp2ul(BfrEnd));
	DbgFEn(2,DbgFNm,DbgSBf);
#endif
	do {				/* do for each line */
/*
 * Set BfrBeg to the beginning of the line to be output. Find the end of the
 * line,  set BfrPtr to the character after it.
 */
		BfrBeg = BfrPtr;
		while (BfrPtr <= BfrEnd) {
		    if (*BfrPtr == CRETRN) {
			BfrPtr++;
			if ((BfrPtr <= BfrEnd) && (*BfrPtr == LINEFD)) {
			    line_len = BfrPtr - BfrBeg - 1;
			    BfrPtr ++;
			    break;
			} else {
			    BfrPtr--;
			}
		    } else if (*BfrPtr == ESCAPE) {
			if ((BfrPtr+2 <= BfrEnd) && (*(BfrPtr+1) == CRETRN) &&
						    (*(BfrPtr+2) == LINEFD)) {
			    BfrPtr += 3;
			    line_len = BfrPtr - BfrBeg;
			    break;
			}
		    } else if (IsEOL(*BfrPtr)) {	/* LF, VT or FF? */
			    BfrPtr++;
			    line_len = BfrPtr - BfrBeg;
			    break;
		    }
		    ++BfrPtr;
		}
		OFiles[OfIndx].ORab.rab$l_rbf = BfrBeg;		/* buffer */
		OFiles[OfIndx].ORab.rab$w_rsz = line_len;	/* size */
#if DEBUGGING
		sprintf(DbgSBf,"calling sys$put, line_len = %d", line_len);
		DbgFMs(2,DbgFNm,DbgSBf);
#endif
		status = sys$put(&OFiles[OfIndx].ORab);	/* output the record */
		if (status != RMS$_NORMAL) {		/* if it didn't work */
			ZErMsg(status, OFiles[OfIndx].ORab.rab$l_stv);
			ErrMsg(ERR_UWL);
			DBGFEX(2,DbgFNm,"FAILURE");
			return FAILURE;
		}
	} while (BfrPtr <= BfrEnd);
	DBGFEX(2,DbgFNm,"SUCCESS");
	return SUCCESS;
}
