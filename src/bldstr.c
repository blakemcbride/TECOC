/*****************************************************************************
	BldStr()
	This function "builds" a string.  This means converting string
build constructs in the input string into their intended equivalents
in the output string.  The string build constructs are as follows:
	^Q	use next character literally,  not as a string build char
	^R	use next character literally,  not as a string build char
	^V	lowercase the next character
	^V^V	lowercase all following characters
	^W	uppercase the next character
	^W^W	uppercase all following characters
	^EQq	use string in q-register q here
	^EUq	use ASCII char for number in q-register q here
	When this function is called,  CBfPtr points to the first character
of the input string.  It is assumed that the string is terminated by an ESCAPE
character (or something else if the calling command was @-modified).  If the
string is not properly terminated this function will die with "unterminated
command" when it encounters CStEnd while looking for the terminator character.
	When this function returns,  CBfPtr points to the ESCAPE which
terminates the string, the built string is in the buffer pointed to by
XBfBeg,  and XBfPtr points to the character after the last character in
the built string.
	The commands which contain a filename (EB, EI, EN, ER and
EW) use this function.  The EG command, which exits with an operating system
command line, uses this function.  The O command, which jumps to a tag, uses
this function.  The search commands (E_, FK, FN, FS, F_, N, S and _) use this
function.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
#include "dchars.h"		/* define identifiers for characters */
#include "chmacs.h"		/* define character processing macros */
#if USE_PROTOTYPES
static DEFAULT DoCtVW(charptr EndArg, unsigned char TmpChr);
static DEFAULT DoCtE(charptr EndArg, charptr XBfEnd);
#endif
static charptr BBfPtr;			/* pointer into XBf */
static int CaseCv;			/* case conversion */
static unsigned char WVFlag;		/* ^W or ^V flag */
static DEFAULT DoCtVW(EndArg, TmpChr)	/* do a control-V or control-W */
charptr EndArg;				/* ptr to end of string argument */
unsigned char TmpChr;			/* temporary character */
{
    DBGFEN(3,"DoCtVW",NULL);
    WVFlag = TmpChr;
    if (++CBfPtr == EndArg) {		/* move past ^W or ^V,  too far? */
	ErrMsg(ERR_ISS);		/* yes, illegal search string */
	DBGFEX(2,DbgFNm,"FAILURE");
	return FAILURE;
    }
    if ((*CBfPtr == '^') && ((EdFlag & ED_CARET_OK) == 0)) {
	if (++CBfPtr == EndArg) {
	    ErrMsg(ERR_ISS);
	    DBGFEX(2,DbgFNm,"FAILURE");
	    return FAILURE;
	}
	TmpChr = To_Upper(*CBfPtr);
	if ((TmpChr < '@') || (TmpChr > '_')) {
	    ErrChr(ERR_IUC, *CBfPtr);
	    DBGFEX(2,DbgFNm,"FAILURE");
	    return FAILURE;
	}
	TmpChr &= '\077';
    } else {
	TmpChr = *CBfPtr;
    }
    if (WVFlag == CTRL_V) {
	if (TmpChr == CTRL_V) {
	    CaseCv = LOWER;
	} else {
	    *BBfPtr++ = To_Lower(TmpChr);
	}
    } else {
	if (TmpChr == CTRL_W) {
	    CaseCv = UPPER;
	} else {
	    *BBfPtr++ = To_Upper(TmpChr);
	}
    }
    WVFlag = '\0';
    DBGFEX(2,DbgFNm,"SUCCESS");
    return SUCCESS;
}
static DEFAULT DoCtE(EndArg, XBfEnd)	/* do a control-E */
charptr EndArg;				/* ptr to end of string argument */
charptr XBfEnd;				/* end of build-string buffer */
{
    DEFAULT	Status;			/* returned from FindQR */
    charptr	TCStEn;			/* temporary holder of CStEnd */
    DBGFEN(3,"DoCtE",NULL);
    if (++CBfPtr == EndArg) {		/* move past ^E,  too far? */
	ErrMsg(ERR_ICE);		/* yes, illegal ^E command */
	DBGFEX(2,DbgFNm,"FAILURE");
	return FAILURE;
    }
    if ((*CBfPtr == 'Q') || (*CBfPtr == 'q')) {
	if (++CBfPtr == EndArg) {
	    ErrMsg(ERR_ISS);
	    DBGFEX(2,DbgFNm,"FAILURE");
	    return FAILURE;
	}
/*
 * handle filespec buffer and search string buffer
 */
	if (*CBfPtr=='*' || *CBfPtr=='_') {
	    charptr  BufPtr, BufBeg;
	    if (*CBfPtr=='*') {
		BufPtr=FBfPtr;
		BufBeg=FBfBeg;
	    } else {
		BufPtr=SBfPtr;
		BufBeg=SBfBeg;
	    }
	    if ((BufPtr-BufBeg) > (XBfEnd-BBfPtr)) {
		ErrMsg(ERR_STL);
		DBGFEX(2,DbgFNm,"FAILURE");
		return FAILURE;
	    }
	    MEMMOVE(BBfPtr, BufBeg, (SIZE_T)(BufPtr - BufBeg));
	    BBfPtr += BufPtr-BufBeg;
	} else {
/*
 * it really must be a Q reg reference after all
 */
	    TCStEn = CStEnd;			/* save CStEnd */
	    CStEnd = EndArg;
	    Status = FindQR();
	    CStEnd = TCStEn;			/* restore CStEnd */
	    if (Status == FAILURE) {
		DBGFEX(2,DbgFNm,"FAILURE");
		return FAILURE;
	    }
	    if ((QR->End_P1-QR->Start) > (XBfEnd-BBfPtr)) {
		ErrMsg(ERR_STL);
		DBGFEX(2,DbgFNm,"FAILURE");
		return FAILURE;
	    }
	    MEMMOVE(BBfPtr, QR->Start, (SIZE_T)(QR->End_P1 - QR->Start));
	    BBfPtr += QR->End_P1 - QR->Start;
	}
    } else if ((*CBfPtr == 'U') || (*CBfPtr == 'u')) {
	if (++CBfPtr == EndArg) {
	    ErrMsg(ERR_ISS);
	    DBGFEX(2,DbgFNm,"FAILURE");
	    return FAILURE;
	}
	TCStEn = CStEnd;			/* save CStEnd */
	CStEnd = EndArg;
	Status = FindQR();
	CStEnd = TCStEn;			/* restore CStEnd */
	if (Status == FAILURE) {
	    DBGFEX(2,DbgFNm,"FAILURE");
	    return FAILURE;
	}
	*BBfPtr++ = (char)QR->Number;
    } else {
	*BBfPtr++ = CTRL_E;
	*BBfPtr++ = *CBfPtr;
    }
    DBGFEX(2,DbgFNm,"SUCCESS");
    return SUCCESS;
}
DEFAULT BldStr(XBfBeg, XBfEnd, XBfPtr)		/* build a string */
charptr XBfBeg;			/* beginning of build-string buffer */
charptr XBfEnd;			/* end of build-string buffer */
charptr (*XBfPtr);		/* pointer into build-string buffer */
{
    charptr	EndArg;		/* end of input string, plus 1 */
    unsigned char TmpChr;	/* temporary character */
    DBGFEN(2,"BldStr",NULL);
    if (FindES(ESCAPE) == FAILURE) {	/* move CBfPtr to end of argument */
	DBGFEX(2,DbgFNm,"FAILURE, FindES(ESCAPE) failed");
	return FAILURE;
    }
    WVFlag = '\0';		/* initialize ^W and ^V flag */
    CaseCv = IniSrM;		/* initialize internal search mode */
    BBfPtr = XBfBeg;		/* initialize ptr into build-string buffer */
    EndArg = CBfPtr;		/* save pointer to end of argument */
    CBfPtr = ArgPtr;		/* reset to beginning of argument */
    while (CBfPtr < EndArg) {
	if ((*CBfPtr == '^') && ((EdFlag & ED_CARET_OK) == 0)) {
	    if (++CBfPtr == EndArg) {
		ErrMsg(ERR_ISS);
		DBGFEX(2,DbgFNm,"FAILURE, no char after ^");
		return FAILURE;
	    }
	    TmpChr = To_Upper(*CBfPtr);
	    if ((TmpChr < '@') || (TmpChr > '_')) {
		ErrChr(ERR_IUC, *CBfPtr);
		DBGFEX(2,DbgFNm,"FAILURE, bad char after ^");
		return FAILURE;
	    }
	    TmpChr &= '\077';
	} else {
	    TmpChr = *CBfPtr;
	}
	switch (TmpChr) {
	    case CTRL_R:
	    case CTRL_Q:
		if (++CBfPtr == EndArg) {
		    ErrMsg(ERR_ISS);
		    DBGFEX(2,DbgFNm,"FAILURE");
		    return FAILURE;
		}
		*BBfPtr++ = *CBfPtr;
		break;
	    case CTRL_V:
	    case CTRL_W:
		if (DoCtVW(EndArg, TmpChr) == FAILURE) {
		    DBGFEX(2,DbgFNm,"FAILURE, DoCtVW failed");
		    return FAILURE;
		}
		break;
	    case CTRL_E:
		if (DoCtE(EndArg, XBfEnd) == FAILURE) {
		    DBGFEX(2,DbgFNm,"FAILURE, DoCtE failed");
		    return FAILURE;
		}
		break;
	    default:
		if (CaseCv == LOWER) {
		    TmpChr = To_Lower(TmpChr);
		} else if (CaseCv == UPPER) {
		    TmpChr = To_Upper(TmpChr);
		}
		*BBfPtr++ = TmpChr;
	}
	if (BBfPtr > XBfEnd) {
	    ErrMsg(ERR_STL);	/* string too long */
	    DBGFEX(2,DbgFNm,"FAILURE, string too long");
	    return FAILURE;
	}
	++CBfPtr;
    }
    *XBfPtr = BBfPtr;
#if DEBUGGING
    sprintf(DbgSBf,"string = \"%.*s\"", (int)(BBfPtr-XBfBeg), XBfBeg);
    DbgFEx(2,DbgFNm,DbgSBf);
#endif
    return SUCCESS;
}
