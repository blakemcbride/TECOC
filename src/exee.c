/*****************************************************************************

	ExeE()

	This function executes a E command.

	EA		Select secondary output stream
	EBfilespec$	Open file for input and output
	:EBfilespec$	EB, return status
	EC		Close out (copy in to out and close)
	ED		Edit mode flag
	EE		Escape surrogate character
	EF		Close output file
	EG		Close out and exit, run last COMPIL class command
	:EGcmd args$	Performs operating system function
	:EGtext$	Close out and exit, run "text"
	EH		Help level flag
	EI		Close current indirect command file
	EIfilespec$	Open indirect command file
	m,nEJ		Set environment characteristics
	nEJ		Return environment characteristics
	EK		Kill output file
	ELfilespec$	Open log file
	mEM		Position mag tape
	EN		Wildcard lookup
	:EN		EN, return status
	ENfilespec$	Preset wildcard lookup
	EO		Version of TECO
	nEO		Set TECO to function as version n
	EP		Select secondary input stream
	EQqfilespec$	Read file into Q-register q
	ER		Select primary input stream
	ERfilespec$	Open input file
	:ERfilespec$	ER, return status
	ES		Search verification flag
	ET		Type out control flag
	EU		Case flagging flag
	EV		Edit verify flag
	EWfilespec$	Open output file
	EW		Select primary output stream
	EX		Close out and exit
	EY		Read without yank protection
	EZ		System-dependent mode control flags
	E%qfilespec$	Write Q-register q into a file
	nE_		Search without yank protection

*****************************************************************************/

#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "chmacs.h"		/* define character processing macros */
#include "dchars.h"		/* define identifiers for characters */
#include "deferr.h"		/* define identifiers for error messages */

#if USE_PROTOTYPES
static DEFAULT DoEI(DEFAULT IfIndx);
static DEFAULT ExeEA(void);
static DEFAULT ExeEB(void);
static DEFAULT ExeEC(void);
static DEFAULT ExeED(void);
static DEFAULT ExeEF(void);
static DEFAULT ExeEG(void);
static DEFAULT ExeEH(void);
static DEFAULT ExeEI(void);
static DEFAULT ExeEK(void);
static DEFAULT ExeEN(void);
static DEFAULT ExeEO(void);
static DEFAULT ExeEP(void);
static DEFAULT ExeEPr(void);
static DEFAULT ExeEQ(void);
static DEFAULT ExeER(void);
static DEFAULT ExeES(void);
static DEFAULT ExeET(void);
static DEFAULT ExeEU(void);
static DEFAULT ExeEUn(void);
static DEFAULT ExeEV(void);
static DEFAULT ExeEW(void);
static DEFAULT ExeEX(void);
static DEFAULT ExeEZ(void);
static DEFAULT GetWha(charptr TxtPtr, ptrdiff_t TxtLen);
static DEFAULT OpnEI(DEFAULT EInd);
static DEFAULT OpnInp(DEFAULT IfIndx, BOOLEAN EIFile, BOOLEAN RepFNF);
static DEFAULT OpnOut(DEFAULT OfIndx, BOOLEAN RepErr, BOOLEAN Backup);
static DEFAULT ReadEI(DEFAULT IfIndx, charptr *ZBfPtr, charptr *EscPtr);
#endif

charptr ZBfBeg;
charptr ZBfEnd;
charptr ZBfRdP;		/* pointer where to start reading into ZBf */

/*****************************************************************************

	OpnInp()

	This function opens an input file.  The name of the file is pointed
to by FBfBeg.  FBfPtr points to the character following the last character of
the file name.

*****************************************************************************/

static DEFAULT OpnInp(IfIndx, EIFile, RepFNF)
DEFAULT	IfIndx;				/* output file indicator */
BOOLEAN	EIFile;				/* is it a macro file? */
BOOLEAN RepFNF;				/* report "file not found" error? */
{
    DEFAULT status;

    DBGFEN(2,"OpnInp",NULL);

    ZIClos(IfIndx);			/* close input file, if any */

    status = ZOpInp(IfIndx, EIFile, RepFNF);
    if (status != SUCCESS) {
	if (!RepFNF && (status == FILENF)) {
	    DBGFEX(2,DbgFNm,"FILENF");
	    return FILENF;
	}
	ErrPSt(ERR_UFI, FBfBeg, FBfPtr);
	DBGFEX(2,DbgFNm,"status");
	return status;
    }

    IsOpnI[IfIndx] = TRUE;		/* mark the file as open */
    IsEofI[IfIndx] = FALSE;		/* end-of-file indicator */

    DBGFEX(2,DbgFNm,"SUCCESS");
    return SUCCESS;
}

/*****************************************************************************

	OpnOut()

	This function creates and opens an output file.  The name of the file
to be created is pointed to by FBfBeg.  FBfPtr points to the character
following the last character of the file name.

*****************************************************************************/


static DEFAULT OpnOut(OfIndx, RepErr, Backup)
DEFAULT	OfIndx;				/* output file indicator */
BOOLEAN RepErr;				/* report errors? */
BOOLEAN Backup;				/* create backup? TAA Added */
{
	DBGFEN(2,"OpnOut",NULL);

	if (IsOpnO[OfIndx]) {		/* if output file is open */
		ErrMsg(ERR_OFO);	/* OFO = output file already open */
		DBGFEX(2,DbgFNm,"FAILURE");
		return FAILURE;
	}

	if (ZOpOut(OfIndx, RepErr, Backup) == FAILURE) {
		if (RepErr) {
			ErrPSt(ERR_UFO, FBfBeg, FBfPtr);
		}
		DBGFEX(2,DbgFNm,"FAILURE");
		return FAILURE;
	}

	IsOpnO[OfIndx] = TRUE;

	DBGFEX(2,DbgFNm,"SUCCESS");
	return SUCCESS;
}

/*****************************************************************************

	ExeEA()

	This function executes an EA command.

	EA	Select secondary output stream

*****************************************************************************/

static DEFAULT ExeEA()			/* execute an EA command */
{
	DBGFEN(1,"ExeEA",NULL);

	CurOut = SOUTFL;
	CmdMod = '\0';			/* clear modifiers flags */
	EStTop = EStBot;		/* clear expression stack */

	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}

/*****************************************************************************

	ExeEB()

	This function executes an EB command.

	EBfilespec$	Open file for input and output
	:EBfilespec$	EB, return status

*****************************************************************************/

static DEFAULT ExeEB()			/* execute an EB command */
{
	BOOLEAN	RepFNF;			/* report "file not found" ? */
	DEFAULT	status;			/* OpnInp() status */

	DBGFEN(1,"ExeEB",NULL);

	if (BldStr(FBfBeg, FBfEnd, &FBfPtr) == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, BldStr() failed");
		return FAILURE;
	}

	RepFNF = !(CmdMod & COLON);		/* report file not found? */
	CmdMod = '\0';				/* clear modifiers flags */
	status = OpnInp(CurInp, FALSE, RepFNF);	/* open input file */
	if (status != SUCCESS) {
		if (status == FILENF) {
			if (!RepFNF) {		/* if it's :EB */
				DBGFEX(1,DbgFNm,"PushEx(0)");
				return PushEx(0L,OPERAND);
			}
		}
		DBGFEX(1,DbgFNm,"FAILURE, OpnInp() failed");
		return FAILURE;
	}

	ZMKOFN();				/* kludge for VMS filenames */

	if (OpnOut(CurOut, RepFNF, TRUE) == FAILURE) {/* open output file */
#if DEBUGGING
		sprintf(DbgSBf,"%s, OpnOut() failed",
			(RepFNF) ? "FAILURE" : "PushEx(0)");
		DbgFEx(1,DbgFNm,DbgSBf);
#endif
		return (RepFNF) ? FAILURE : PushEx(0L,OPERAND);
	}

	if (!RepFNF) {				/* if it's :EB */
		DBGFEX(1,DbgFNm,"PushEx(-1)");
		return PushEx(-1L,OPERAND);
	}

	EStTop = EStBot;			/* clear expression stack */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}

/*****************************************************************************

	ExeEC()

	This function executes an EC command.

	EC	Close out (copy in to out and close)

*****************************************************************************/

static DEFAULT ExeEC()			/* execute an EC command */
{
	DBGFEN(1,"ExeEC",NULL);

	if (EStTop > EStBot) {			/* if numeric argument */
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE, GetNmA() failed");
			return FAILURE;
		}

/* ??? */	ZDspBf("memory expand as per page 84\r\n", 29);
		CmdMod = '\0';			/* clear modifiers flags */

		DBGFEX(1,DbgFNm,"SUCCESS");
		return SUCCESS;
	}

    if (IsOpnO[CurOut]) {

/*
 * if there's text in the edit buffer,  or if the form feed flag indicates
 * that the current page needs to be output,  call WrPage to write the page,
 * then clear the edit buffer.
 */

	if ((GapBeg > EBfBeg) || (GapEnd < EBfEnd) || FFPage) {
		if (WrPage(CurOut, EBfBeg, EBfEnd, FFPage) == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE, WrPage() failed");
			return FAILURE;
		}
		GapBeg = EBfBeg;		/* clear the... */
		GapEnd = EBfEnd;		/*   ...edit buffer */
	}

/*
 * Move the remainder of the current input file, if any, to the currenti
 * output file.
 */

	if (IsOpnI[CurInp]) {			/* if input file is open */
		while (!IsEofI[CurInp]) {	/* while not end-of-file */
			if (RdPage() == FAILURE) {	/* read a page */
				DBGFEX(1,DbgFNm,"FAILURE, RdPage() failed");
				return FAILURE;
			}
			if (WrPage(CurOut, EBfBeg, EBfEnd, FFPage) == FAILURE) {
				DBGFEX(1,DbgFNm,"FAILURE, WrPage() failed");
				return FAILURE;
			}
			GapBeg = EBfBeg;	/* clear the... */
			GapEnd = EBfEnd;	/*   ...edit buffer */
		}
	}
    }

	ZIClos(PINPFL);		/* close primary input file */
	ZIClos(SINPFL);		/* close secondary input file */
	ZOClos(POUTFL);		/* close primary output file */
	ZOClos(SOUTFL);		/* close secondary output file */

	CmdMod = '\0';		/* clear modifiers flag */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}

/*****************************************************************************

	ExeED()

	This function executes an ED command.

	ED		Edit mode flag

*****************************************************************************/

static DEFAULT ExeED()			/* execute an ED command */
{
	DBGFEN(1,"ExeED",NULL);
	DBGFEX(1,DbgFNm,"DoFlag(&EdFlag)");
	return DoFlag(&EdFlag);
}

/*****************************************************************************

	ExeEF()

	This function executes an EF command.

	EF		Close output file

*****************************************************************************/

static DEFAULT ExeEF()			/* execute an EF command */
{
	DBGFEN(1,"ExeEF",NULL);

	ZOClos(CurOut);			/* close output file */
	CmdMod = '\0';			/* clear modifiers flags */
	EStTop = EStBot;		/* clear expression stack */

	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}

/*****************************************************************************

	ExeEG()

	This function implements the EG command.  If the EG command is not
colon modified,  it terminates TECO and passes a command line to the command
line interpreter.

	EG		Close out and exit
	EGtext$		Close out and exit, execute "text" command

If the EG command is colon modified,  then it implements one of the "special
functions" as follows:

	:EGINI$		gets, sets or clears the initialization file name
	:EGLIB$		gets, sets or clears the macro library directory
	:EGMEM$		gets, sets or clears the file name memory
	:EGVTE$		gets, sets or clears the video macro file name

other, system-dependent functions may be defined.  See the ZClnEG function.

*****************************************************************************/

static DEFAULT GetWha(TxtPtr, TxtLen)	/* get "what to operate on" */
charptr TxtPtr;				/* string to parse */
ptrdiff_t TxtLen;			/* length of string */
{
	if (TxtLen >= 3) {			/* it's long enough */
		switch(To_Upper(*TxtPtr)) {
		case 'I':			/* INI (init macro) */
			TxtPtr++;
			if (To_Upper(*TxtPtr) == 'N') {
				++TxtPtr;
				if (To_Upper(*TxtPtr) == 'I') {
					return EG_INI;
				}
			}
			break;

		case 'L':			/* LIB (macro directory) */
			TxtPtr++;
			if (To_Upper(*TxtPtr) == 'I') {
				TxtPtr++;
				if (To_Upper(*TxtPtr) == 'B') {
					return EG_LIB;
				}
			}
			break;

		case 'M':			/* MEM (filename memory) */
			TxtPtr++;
			if (To_Upper(*TxtPtr) == 'E') {
				TxtPtr++;
				if (To_Upper(*TxtPtr) == 'M') {
					return EG_MEM;
				}
			}
			break;

		case 'V':			/* VTE (vtedit macro) */
			TxtPtr++;
			if (To_Upper(*TxtPtr) == 'T') {
				TxtPtr++;
				if (To_Upper(*TxtPtr) == 'E') {
					return EG_VTE;
				}
			}
			break;

		default:
			break;
		}
	}

	return EG_OTHER;
}

#define GBFSIZE 256

static DEFAULT ExeEG()		/* perform operating system function */
{
    DEFAULT EGWhat;		/* what to operate on: INI, VTE, etc. */
    unsigned char GBfBeg[GBFSIZE];
    charptr GBfPtr;
    ptrdiff_t GBfLen;

    DBGFEN(1,"ExeEG",NULL);

    if (BldStr(GBfBeg, GBfBeg+GBFSIZE-2, &GBfPtr) == FAILURE) {
	DBGFEX(1,DbgFNm,"FAILURE, BldStr() failed");
	return FAILURE;
    }

    GBfLen = GBfPtr - GBfBeg;
    if (CmdMod & COLON) {			/* if it's :EG */
        DEFAULT EGOper = GET_VAL;		/* get, set or clear */
	charptr TmpPtr;
	CmdMod &= ~COLON;			/* clear colon flag */
	EGWhat = GetWha(GBfBeg,GBfLen);
	TmpPtr = (charptr)GBfBeg;
	if (EGWhat != EG_OTHER) {
	    if (GBfLen == 4) {			/* if it's "clear" */
	        if (*(TmpPtr+3) != ' ') {
		    DBGFEX(1,DbgFNm,"PushEx(0)");
		    return PushEx(0L,OPERAND);
		}
		EGOper = CLEAR_VAL;
	    } else if (GBfLen != 3) {		/* it it's not "get" */
	        EGOper = SET_VAL;
		TmpPtr += 4;
	    }
        }
	*GBfPtr = '\0';
	DBGFEX(1,DbgFNm,"PushEx(ZClnEG())");
	return PushEx(ZClnEG(EGWhat,EGOper,TmpPtr),OPERAND);
    }

    if (ExeEC() == FAILURE) {			/* do an EC command */
	DBGFEX(1,DbgFNm,"FAILURE, ExeEC() failed");
	return FAILURE;
    }

    if (GBfLen == 0) {				/* if it was EC$ */
	TAbort(EXIT_SUCCESS);			/* just exit TECO-C */
    }

    ZDoCmd(GBfBeg, GBfPtr);			/* exit with a command */
    DBGFEX(1,DbgFNm,"SUCCESS");		/* we should never reach this */
    return SUCCESS;
}

/*****************************************************************************

	ExeEH()

	This function executes an EH command.

	EH		Help level flag

*****************************************************************************/

static DEFAULT ExeEH()			/* execute an EH command */
{
	DBGFEN(1,"ExeEH",NULL);
	DBGFEX(1,DbgFNm,"DoFlag()");
	return DoFlag(&EhFlag);
}

/*****************************************************************************

	ReadEI()

	This function reads an EI file.  It is passed an index for the file
data block of the open EI file,  and a buffer.  It reads the contents of the
file into the buffer,  stopping when a double ESCAPE or end-of-file is
encountered.

*****************************************************************************/

static DEFAULT ReadEI(IfIndx, ZBfPtr, EscPtr)
DEFAULT	IfIndx;		/* index into IFiles array */
charptr	*ZBfPtr;	/* returned ptr to end of EI string (+1) */
charptr *EscPtr;	/* returned ptr to $$ in EI string  (+1) */
{
	DEFAULT		line_len;
	charptr		NewBuf;
	SIZE_T		NewSiz;
	BOOLEAN		previous_char_was_escape;
	charptr		TmpPtr;

#if DEBUGGING
	static char *DbgFNm = "ReadEI";
	sprintf(DbgSBf,"ZBfBeg = %ld, ZBfEnd = %ld",
		Zcp2ul(ZBfBeg), Zcp2ul(ZBfEnd));
	DbgFEn(1,DbgFNm,DbgSBf);
#endif

	FOREVER {

/*
 * potentially reallocate ZBf
 */

		if ((ZBfEnd - ZBfRdP) < ZBFMIN) {
			NewSiz = (ZBfEnd - ZBfBeg) + ZBFEXP;
			NewBuf = (charptr)ZRaloc(ZBfBeg, NewSiz);
			if (NewBuf == NULL) {
				ErrMsg(ERR_MEM); /* MEM = memory overflow */
				DBGFEX(1,DbgFNm,"FAILURE, ZRaloc() failed");
				return (FAILURE);
			}
			ZBfRdP = NewBuf + (ZBfRdP - ZBfBeg);
			ZBfEnd = NewBuf + NewSiz;
			ZBfBeg = NewBuf;
		}
/*
 * append a line from the EI file to ZBf
 */

		if (ZRdLin(ZBfRdP,ZBfEnd-ZBfRdP,IfIndx,&line_len) == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE, ZRdLin() failed");
			return (FAILURE);
		}

/*
 * break out of FOREVER loop if we tried reading past end of file.
 */

		if (IsEofI[IfIndx]) {
			DBGFMS(1,DbgFNm,"ran into EOF");
			break;
		}

#if DEBUGGING
		sprintf(DbgSBf,"read %d bytes: \"%.*s\"",
			(int)line_len, (int)line_len, ZBfRdP);
		DbgFMs(1,DbgFNm,DbgSBf);
#endif

/*
 * go through the line we've read, looking for $$.
 */

		TmpPtr = ZBfRdP;
		ZBfRdP += line_len;
		previous_char_was_escape = FALSE;
		for (; TmpPtr < ZBfRdP; ++TmpPtr) {
			if (*TmpPtr == ESCAPE) {
				if (previous_char_was_escape) {
					*ZBfPtr = ZBfRdP;
					*EscPtr = ++TmpPtr;
					DBGFEX(1,DbgFNm,
					       "SUCCESS, ran into $$");
					return (SUCCESS);
				}
				previous_char_was_escape = TRUE;
			} else {
				previous_char_was_escape = FALSE;
			}
		}
	}

	*ZBfPtr = ZBfRdP;
	*EscPtr = NULL;

	DBGFEX(1,DbgFNm,"SUCCESS");

	return SUCCESS;
}

/*****************************************************************************

	OpnEI()

	This function opens an EI file.  The file name given by the user may
find the file one of the following ways:

	1. as given (current directory)
	2. as given, but with a ".TEC" type (current directory)
	3. as given, in the default directory, with a ".TEC" type
	3. as given, in the default directory

As soon as one of these forms is found and opened,  this function returns.

*****************************************************************************/

static DEFAULT OpnEI(EInd)	/* open an EI file */
DEFAULT EInd;
{
#ifdef OPNEI_FINISHED
	if (OpnInp(EInd, FALSE, TRUE) == SUCCESS)
		return(SUCCESS);
	if (OpnInp(EInd, TRUE, TRUE) == SUCCESS)
		return(SUCCESS);
	save contents of FBf
	use :EGDEF command to fill FBf with default directory (how?)
	append saved FBf to actual FBf
	if (OpnInp(EInd, FALSE, TRUE) == SUCCESS)
		return(SUCCESS);
	if (OpnInp(EInd, TRUE, TRUE) == SUCCESS)
		return(SUCCESS);
#else
	return OpnInp(EInd, TRUE, TRUE);
#endif
}

/*****************************************************************************

	DoEI()

	This function performs the execution part of an EI command.  It is
called by the ExeEI function.  It is passed an index to a file data block in
the IFiles array that has been opened.  It is also passed a memory buffer.
This function reads text from the file into the buffer and calls ExeCSt to
execute the text.

*****************************************************************************/

static DEFAULT DoEI(IfIndx)	/* do an EI command */
DEFAULT	IfIndx;			/* index into IFiles array */
{
    charptr ZBfPtr;		/* pointer to end of command string in ZBf */
    charptr EscPtr;		/* ptr into ZBf past second $ of $$ pair */
    DEFAULT status;		/* return status of ExeCSt() */

    DBGFEN(1,"DoEI",NULL);

    status = SUCCESS;		/* in case we try doing an empty EI file */
    ZBfRdP = ZBfBeg;		/* start reading into beginning of ZBf */
    do {
/*
 * Read EI file into ZBf until $$ or EOF, whichever comes first
 */
	if (ReadEI(IfIndx, &ZBfPtr, &EscPtr) == FAILURE) {
	    ErrMsg(ERR_URE);		/* Unable to Read EI file */
	    DBGFEX(1,DbgFNm,"FAILURE, ReadEI() failed");
	    return FAILURE;
	}
/*
 * save the current execution state and make ZBf the current command string
 * to execute
 */
	if (PshMac(ZBfBeg, (EscPtr) ? EscPtr : ZBfPtr) == FAILURE) {
	    DBGFEX(1,DbgFNm,"FAILURE, PshMac() failed");
	    return FAILURE;
	}
/*
 * execute macro in ZBf
 */
	status = ExeCSt();

#if DEBUGGING
	if (status == FAILURE) {
	    DbgFMs(1,DbgFNm,"ExeCSt() failed");
	}
#endif
/*
 * restore old execution state
 */
	if (PopMac() == FAILURE) {
	    DBGFEX(1,DbgFNm,"FAILURE, PopMac() failed");
	    return FAILURE;
	}
/*
 * if we ran into an $$ that was in the middle of a line,  then we want to
 * move it to the beginning of ZBf and append the rest of the EI file to
 * it; otherwise, we'll just start reading into the beginning of ZBf.
 */
	if (EscPtr) {
	    ptrdiff_t LeftOver;

	    LeftOver = ZBfPtr - EscPtr;
	    MEMMOVE(ZBfBeg, EscPtr, LeftOver);
	    ZBfRdP = ZBfBeg + LeftOver;
	} else {
	    ZBfRdP = ZBfBeg;
	}

    } while (!IsEofI[IfIndx] && !GotCtC && status != FAILURE);

    DBGFEX(1,DbgFNm,(status == FAILURE) ? "FAILURE" : "SUCCESS");

    return status;		/* return ExeCSt() status */
}

/*****************************************************************************

	ExeEI()

	This function executes an EI command.  This function does only part of
the work;  the DoEI function does the rest.  This function opens/closes the
file and allocates/deallocates an input buffer.  The code executes such that
no matter what happens,  an open file will always be closed and allocated
memory will always be freed.

	EI		Close current indirect command file
	EIfilespec$	Open indirect command file

*****************************************************************************/

/*
 * EIIndx is an index into the IFiles array.  It is incremented each time an
 * EI command is executed and decremented each time an EI command completes.
 * It is the index of the file data block of the currently open EI input file.
 * When EIIndx equals EIFL-1,  no EI files are open.
 */

static DEFAULT EIIndx = EIFL-1;

static DEFAULT ExeEI()			/* execute an EI command */
{
    int	status;

    DBGFEN(1,"ExeEI",NULL);

    if (BldStr(FBfBeg, FBfEnd, &FBfPtr) == FAILURE) {
	DBGFEX(1,DbgFNm,"FAILURE, BldStr() failed");
	return FAILURE;
    }

    if (FBfPtr == FBfBeg) {			/* if it's EI$ */
	if (EIIndx >= EIFL) {			/* if EI file is open */
	    CmdMod = '\0';			/* clear modifiers flags */
	}
	EStTop = EStBot;			/* clear expression stack */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
    }

    if (++EIIndx > NIFDBS) {			/* If EI nesting too deep */
	EIIndx--;
	ErrMsg(ERR_MEM);			/* ERR_MEM=memory overflow */
	DBGFEX(1,DbgFNm,"FAILURE, EI nested too deep");
	return FAILURE;
    }

    if (OpnEI(EIIndx) != SUCCESS) {		/* open EI file */
	status = FAILURE;
    } else {
	ZBfBeg = (charptr)ZAlloc(ZBFINIT);	/* allocate input buffer */
	if (ZBfBeg == NULL) {			/* if allocation failed */
	    ErrMsg(ERR_MEM);			/* MEM = memory overflow */
	    status = FAILURE;
	} else {				/* else allocation ok */
	    ZBfEnd = ZBfBeg + ZBFINIT;
	    status = DoEI(EIIndx);
	    ZFree((voidptr)ZBfBeg);		/* free allocated memory */
	}
	ZIClos(EIIndx);
    }
    EIIndx--;					/* decrement nesting count */
    CmdMod = '\0';				/* clear modifiers flags */
    EStTop = EStBot;				/* clear expression stack */

    DBGFEX(1,DbgFNm,"SUCCESS");

    return status;
}

/*****************************************************************************

	ExeEK()

	This function executes an EK command.

	EK		Kill output file

*****************************************************************************/

static DEFAULT ExeEK()			/* execute an EK command */
{
	DBGFEN(1,"ExeEK",NULL);

	ZOClDe(CurOut);			/* close and delete output file */
	CmdMod = '\0';			/* clear modifiers flags */
	EStTop = EStBot;		/* clear expression stack */

	DBGFEX(1,DbgFNm,"SUCCESS");

	return SUCCESS;
}

/*****************************************************************************

	ExeEN()

	This function executes an EN command.

	EN		Wildcard lookup
	:EN		EN, return status
	ENfilespec$	Preset wildcard lookup

*****************************************************************************/

static DEFAULT ExeEN()			/* execute an EN command */
{
	DBGFEN(1,"ExeEN",NULL);

	if (BldStr(FBfBeg, FBfEnd, &FBfPtr) == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, BldStr() failed");
		return FAILURE;
	}

	if (FBfPtr == FBfBeg) {		/* if it's EN$ or :EN$ */
		switch (ZSWild()) {
		case FAILURE:
			DBGFEX(1,DbgFNm,"FAILURE, ZSWild() failed");
			return FAILURE;

		case FILENF:
			if (CmdMod & COLON) {	/* if it's :EN */
				CmdMod = '\0';	/* clear modifiers flags */
				EStTop = EStBot;/* clear expression stack */
				DBGFEX(1,DbgFNm,"PushEx(0)");
				return PushEx(0L,OPERAND);
			}
			ErrStr(ERR_FNF, "");	/* file not found */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;

		case SUCCESS:
			if (CmdMod & COLON) {	/* if it's :EN */
				CmdMod = '\0';	/* clear modifiers flags */
				DBGFEX(1,DbgFNm,"PushEx(-1)");
				return PushEx(-1L,OPERAND);
			}
			EStTop = EStBot;/* clear expression stack */
			CmdMod = '\0';	/* clear modifiers flags */
			DBGFEX(1,DbgFNm,"SUCCESS");
			return SUCCESS;
		}
	}

	if (ZPWild() == FAILURE) {		/* preset wildcard filepsec */
		DBGFEX(1,DbgFNm,"FAILURE, ZPWild() failed");
		return FAILURE;
	}

	EStTop = EStBot;			/* clear expression stack */
	CmdMod = '\0';				/* clear modifiers flags */

	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}

/*****************************************************************************

	ExeEO()

	This function executes an EO command.  TECO-C's version numbers
started at 100 and went up.  Other implementations of TECO have version
numbers less than 100.

	EO		Version of TECO
	nEO		Set TECO to function as version n

*****************************************************************************/

static DEFAULT ExeEO()			/* execute an EO command */
{
	return ((EStTop > EStBot)		/* is it nEO ? */
	    ? ExeNYI()				/* ...yes, not supported */
	    : PushEx((LONG)TVERSION,OPERAND));	/* ...no, return version */
}

/*****************************************************************************

	ExeEP()

	This function executes an EP command.

	EP		Select secondary input stream

*****************************************************************************/

static DEFAULT ExeEP()			/* execute an EP command */
{
	DBGFEN(1,"ExeEP",NULL);

	CurInp = SINPFL;		/* select secondary input stream */
	CmdMod = '\0';			/* clear modifiers flags */
	EStTop = EStBot;		/* clear expression stack */

	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}

/*****************************************************************************

	ExeEPr()

	This function executes an E% command.

	E%qfilespec$	Write Q-register q into a file

*****************************************************************************/

static DEFAULT ExeEPr()			/* execute an E% command */
{
	BOOLEAN RepErr;			/* report errors? */

	DBGFEN(1,"ExeEPr",NULL);

	if (IncCBP() == FAILURE) {		/* move to char after % */
		DBGFEX(1,DbgFNm,"FAILURE, IncCBP() failed");
		return FAILURE;
	}

	if (FindQR() == FAILURE) {		/* find q-register */
		DBGFEX(1,DbgFNm,"FAILURE, FindQR() failed");
		return FAILURE;
	}

	if (BldStr(FBfBeg, FBfEnd, &FBfPtr) == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, BldStr() failed");
		return FAILURE;
	}

	if (FBfPtr == FBfBeg) {			/* if it's E%q$ */
		ErrUTC();			/* unterminated command */
		DBGFEX(1,DbgFNm,"FAILURE, unterminated command");
		return FAILURE;
	}

	RepErr = !(CmdMod & COLON);
	CmdMod = '\0';
	if (OpnOut(EPRCFL, RepErr, FALSE) == FAILURE) {
#if DEBUGGING
		sprintf(DbgSBf,"%s, OpnOut() failed",
			(RepErr) ? "FAILURE" : "PushEx(0)");
		DbgFEx(1,DbgFNm,DbgSBf);
#endif
		return ((RepErr) ? FAILURE : PushEx(0L,OPERAND));
	}

	if (QR->Start != NULL) {		/* if it's not empty */
		if (ZWrBfr(EPRCFL, QR->Start, QR->End_P1-1) == FAILURE) {
#if DEBUGGING
			sprintf(DbgSBf,"%s, ZWrBfr() failed",
				(RepErr) ? "FAILURE" : "PushEx(0)");
			DbgFEx(1,DbgFNm,DbgSBf);
#endif
			return ((RepErr) ? FAILURE : PushEx(0L,OPERAND));
		}
	}

	ZOClos(EPRCFL);				/* close the file */

	if (!RepErr) {
		DBGFEX(1,DbgFNm,"PushEx(-1)");
		return PushEx(-1L,OPERAND);
	}

	EStTop = EStBot;			/* clear expression stack */

	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}

/*****************************************************************************

	ExeEQ()

	This function executes an EQ command.  The EQ command reads a file
directly into a q-register.  This command uses the RdPage function to
temporarily append the contents of the file to the end of the edit buffer.
It then copies the text to a q-register.

	EQqfilespec$	Read file into Q-register q

*****************************************************************************/

static DEFAULT ExeEQ()			/* execute an EQ command */
{
	ptrdiff_t	FSize;		/* file size */
	LONG		SvFFPg;		/* saved FFPage value */
	DEFAULT		SvCrIn;		/* saved CurInp value */
	ptrdiff_t	SvEbSz;		/* saved edit buffer size */
	DEFAULT		Status;

	DBGFEN(1,"ExeEQ",NULL);

	if (IncCBP() == FAILURE) {		/* move to char after Q */
		DBGFEX(1,DbgFNm,"FAILURE, IncCBP() failed");
		return FAILURE;
	}

	if (FindQR() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, FindQR() failed");
		return FAILURE;
	}

	if (BldStr(FBfBeg, FBfEnd, &FBfPtr) == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, BldStr() failed");
		return FAILURE;
	}

	if (FBfPtr == FBfBeg) {			/* if it's EQq$ */
		ErrUTC();			/* unterminated command */
		DBGFEX(1,DbgFNm,"FAILURE, unterminated command");
		return FAILURE;
	}

	if (OpnInp(EQFL, FALSE, TRUE) != SUCCESS) {	/* open the file */
		DBGFEX(1,DbgFNm,"FAILURE, OpnInp() failed");
		return FAILURE;
	}

/*
 * We borrow RdPage's ability to read files correctly.  RdPage reads file
 * CurInp,  so we need to temporarily set CurInp to the EQ file data block.
 * The data is appended to the edit buffer,  so we need to save the pointer
 * to the current end of the edit buffer.
 */
	SvCrIn = CurInp;				/* save CurInp */
	SvEbSz = (EBfEnd - EBfBeg) - (GapEnd - GapBeg);	/* save EBfEnd */
	SvFFPg = FFPage;				/* save FFPage */
	CurInp = EQFL;
	do {
		if (RdPage() == FAILURE) {
			Status = FAILURE;
			goto exeeqdone;
		}
	} while (!IsEofI[EQFL]);

/*
 * Make room in the q-register.
 */
	FSize = EBfEnd - (EBfBeg + SvEbSz + (GapEnd - GapBeg));
	if (MakRom((SIZE_T)FSize) == FAILURE) {
		Status = FAILURE;
		goto exeeqdone;
	}

/*
 * Copy the text that we just read from the file into the q-register.
 */
	MEMMOVE(QR->Start,
		EBfBeg + SvEbSz + 1 + (GapEnd - GapBeg),
		(SIZE_T)FSize);
	QR->End_P1 = QR->Start + FSize;

	Status = SUCCESS;
exeeqdone:
	FFPage = SvFFPg;			/* restore FFpage */
	EBfEnd = EBfBeg + SvEbSz +
		 (GapEnd - GapBeg);		/* restore EBfEnd */
	CurInp = SvCrIn;			/* restore CurInp */

	ZIClos(EQFL);				/* close the file */

	CmdMod = '\0';				/* clear modifiers flags */
	EStTop = EStBot;			/* clear expression stack */

	DBGFEX(1,DbgFNm,(Status == SUCCESS) ? "SUCCESS" : "FAILURE");
	return Status;
}

/*****************************************************************************

	ExeER()

	This function executes an ER command.

	ER		Select primary input stream
	ERfilespec$	Open input file
	:ERfilespec$	ER, return status
*****************************************************************************/

static DEFAULT ExeER()			/* execute an ER command */
{
	BOOLEAN	RepFNF;			/* report "file not found" errors? */
	DEFAULT	status;

	DBGFEN(1,"ExeER",NULL);

	if (BldStr(FBfBeg, FBfEnd, &FBfPtr) == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, BldStr() failed");
		return FAILURE;
	}

	if (FBfPtr == FBfBeg) {			/* if it's ER$ */
		CurInp = PINPFL;		/* primary input stream */
		CmdMod = '\0';			/* clear modifiers flags */
		EStTop = EStBot;		/* clear expression stack */
		DBGFEX(1,DbgFNm,"SUCCESS");
		return SUCCESS;
	}

	RepFNF = !(CmdMod & COLON);		/* report file not found? */
	CmdMod = '\0';				/* clear modifiers flags */
	status = OpnInp(CurInp, FALSE, RepFNF);	/* open input file */
	if (status != SUCCESS) {
		if (status == FILENF) {
			if (!RepFNF) {		/* if it's :ER */
				DBGFEX(1,DbgFNm,"PushEx(0)");
				return PushEx(0L,OPERAND);
			}
		}
		DBGFEX(1,DbgFNm,"FAILURE, OpnInp() failed");
		return FAILURE;
	}

	if (!RepFNF) {				/* if it's :ER */
		DBGFEX(1,DbgFNm,"PushEx(-1)");
		return PushEx(-1L,OPERAND);
	}

	EStTop = EStBot;			/* clear expression stack */

	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}

/*****************************************************************************

	ExeES()

	This function executes an ES command.

	ES		Search verification flag

*****************************************************************************/

static DEFAULT ExeES()			/* execute an ES command */
{
	DBGFEN(1,"ExeES",NULL);
	DBGFEX(1,DbgFNm,"DoFlag(&EsFlag)");
	return DoFlag(&EsFlag);
}

/*****************************************************************************

	ExeET()

	This function executes an ET command.

	ET		Type out control flag

*****************************************************************************/

static DEFAULT ExeET()			/* execute an ET command */
{
        DEFAULT status;
        WORD TmpET;

	DBGFEN(1,"ExeET",NULL);

	TmpET = EtFlag;
	status = DoFlag(&EtFlag);

/*
 * If DoFlag set or cleared the 8-bit flag,  we have to tell the operating
 * system.
 */
	if (status != FAILURE) {
	    if ((TmpET & ET_EIGHTBIT) &&	  /* eightbit flag cleared? */
		!(EtFlag & ET_EIGHTBIT)) {
		status = ZSetTT(TT8BIT, 0);
	    } else if (!(TmpET & ET_EIGHTBIT) &&  /* eightbit flag set? */
		(EtFlag & ET_EIGHTBIT)) {
		status = ZSetTT(TT8BIT, 1);
	    }
	}

/*
 * If one of the read-only flags was changed,  set it back.
 */
	if ((TmpET & ET_WAT_SCOPE) != (EtFlag & ET_WAT_SCOPE)) {
	    if (TmpET & ET_WAT_SCOPE)		/* if it was cleared */
		EtFlag |= ET_WAT_SCOPE;		/* set it */
	    else
		EtFlag &= ~ET_WAT_SCOPE;	/* clear it */
	}
	if ((TmpET & ET_WAT_REFRESH) != (EtFlag & ET_WAT_REFRESH)) {
	    if (TmpET & ET_WAT_REFRESH)		/* if it was cleared */
		EtFlag |= ET_WAT_REFRESH;	/* set it */
	    else
		EtFlag &= ~ET_WAT_REFRESH;	/* clear it */
	}

	DBGFEX(1,DbgFNm,(status==FAILURE) ? "FAILURE" : "SUCCESS");
	return status;
}

/*****************************************************************************

	ExeEU()

	This function executes an EU command.

	EU		Case flagging flag

*****************************************************************************/

static DEFAULT ExeEU()			/* execute an EU command */
{
	DBGFEN(1,"ExeEU",NULL);
	DBGFEX(1,DbgFNm,"DoFlag(&EuFlag)");
	return DoFlag(&EuFlag);
}

/*****************************************************************************

	ExeEUn()

	This function executes an E_ command.

	nE_		Search without yank protection

*****************************************************************************/

static DEFAULT ExeEUn()			/* execute an E_ command */
{
	DBGFEN(1,"ExeEUn",NULL);

/*
 * The command m,nE_ is illegal: the user should use m,nFB
 */

	if (CmdMod & MARGIS) {			/* if it's m,nE_ */
		ErrStr(ERR_ILL, "m,nE_");	/* illegal command "m,nE_" */
		DBGFEX(1,DbgFNm,"FAILURE, m,nE is illegal");
		return FAILURE;
	}

	if (CmdMod & DCOLON) {			/* if it's ::E_ */
		ErrStr(ERR_ILL, "::E_");	/* illegal command "::E_" */
		DBGFEX(1,DbgFNm,"FAILURE, ::E_ is illegal");
		return FAILURE;
	}

	SrcTyp = E_SEARCH;
	if (Search(FALSE) == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, Search() failed");
		return FAILURE;
	}

	CmdMod = '\0';				/* clear modifiers flags */

	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}

/*****************************************************************************

	ExeEV()

	This function executes an EV command.

	EV		Edit verify flag

*****************************************************************************/

static DEFAULT ExeEV()			/* execute an EV command */
{
	DBGFEN(1,"ExeEV",NULL);
	DBGFEX(1,DbgFNm,"DoFlag()");
	return DoFlag(&EvFlag);
}

/*****************************************************************************

	ExeEW()

	This function executes an EW command.

	EWfilespec$	Open output file
	EW		Select primary output stream

*****************************************************************************/

static DEFAULT ExeEW()			/* execute an EW command */
{
	BOOLEAN RepErr;			/* report errors? */

	DBGFEN(1,"ExeEW",NULL);

	if (BldStr(FBfBeg, FBfEnd, &FBfPtr) == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, BldStr() failed");
		return FAILURE;
	}

	if (FBfPtr == FBfBeg) {			/* if it's EW$ */
		CmdMod = '\0';			/* clear modifiers flags */
		CurOut = POUTFL;		/* primary output stream */
		EStTop = EStBot;		/* clear expression stack */
		DBGFEX(1,DbgFNm,"SUCCESS");
		return SUCCESS;
	}

	RepErr = !(CmdMod & COLON);
	CmdMod = '\0';				/* clear modifiers flags */
	if (OpnOut(CurOut, RepErr, FALSE) == FAILURE) {
#if DEBUGGING
		sprintf(DbgSBf,"%s, OpnOut() failed",
			(RepErr) ? "FAILURE" : "PushEx(0)");
		DbgFEx(1,DbgFNm,DbgSBf);
#endif
		return ((RepErr) ? FAILURE : PushEx(0L,OPERAND));
	}

	if (!RepErr) {
		DBGFEX(1,DbgFNm,"PushEx(-1)");
		return PushEx(-1L,OPERAND);
	}

	EStTop = EStBot;		/* clear expression stack */

	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}

/*****************************************************************************

	ExeEX()

	This function executes an EX command.

	EX		Close out and exit

*****************************************************************************/

static DEFAULT ExeEX()			/* execute an EX command */
{
	DBGFEN(1,"ExeEX",NULL);

	if (ExeEC() != FAILURE) {		/* execute an EC command */
		DBGFEX(1,DbgFNm,"exiting successfully!");
		TAbort(EXIT_SUCCESS);		/* exit TECO-C */
	}

	DBGFEX(1,DbgFNm,"FAILURE, ExeEX() failed");
	return FAILURE;
}

/*****************************************************************************

	ExeEZ()

	This function executes an EZ command.  EZ is a mode control flag
that contains bits specific to the operating system we're using.

*****************************************************************************/

static DEFAULT ExeEZ()			/* execute an EZ command */
{
	DBGFEN(1,"ExeEZ",NULL);
	DBGFEX(1,DbgFNm,"DoFlag(&EZFlag)");
	return DoFlag(&EzFlag);
}

/*****************************************************************************

	ExeE()

	This function executes two-letter commands that start with "E".
It uses the character following the "E" to index into a table of functions.

*****************************************************************************/

DEFAULT	ExeE()				/* execute an E command */
{
	unsigned char TmpChr;
	static DEFAULT (*FEAray[])(VVOID) = {
		/* A */ ExeEA,    /* B */ ExeEB,
		/* C */ ExeEC,    /* D */ ExeED,
		/* E */ ExeIll,   /* F */ ExeEF,
		/* G */ ExeEG,    /* H */ ExeEH,
		/* I */ ExeEI,    /* J */ ZExeEJ,
		/* K */ ExeEK,    /* L */ ExeNYI,
		/* M */ ExeNYI,   /* N */ ExeEN,
		/* O */ ExeEO,    /* P */ ExeEP,
		/* Q */ ExeEQ,    /* R */ ExeER,
		/* S */ ExeES,    /* T */ ExeET,
		/* U */ ExeEU,    /* V */ ExeEV,
		/* W */ ExeEW,    /* X */ ExeEX,
		/* Y */ ExeEY,    /* Z */ ExeEZ
	};

	if (IncCBP() == FAILURE) {
		return FAILURE;
	}

	TmpChr = To_Upper (*CBfPtr);

	if (TmpChr == '%') {			/* we have E%q */
		return ExeEPr();
	}

	if (TmpChr == '_') {			/* we have nE_ */
		return ExeEUn();
	}

	if (!Is_Upper(TmpChr) || (TmpChr == 'E')) {
		ErrChr(ERR_IEC, TmpChr);
		return FAILURE;
	}

	return (*FEAray[TmpChr-'A'])();
}
