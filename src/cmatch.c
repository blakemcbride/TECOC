/*****************************************************************************
	CMatch()
	This recursive function trys to match one match construct with the
character pointed to by EBPtr2.  It sets SamChr, which indicates whether the
match was successful or not.  SStPtr is left pointing to the last character
of the match construct.
	If the match construct is ^Em or ^S,  then EBPtr2 will be updated to
point to the last matched character.  In this case,  RhtSid is needed to
indicate the limit past which EBPtr2 should not be incremented.
	Match constructs are:
	^X		match any character
	^S		match a separator character (not letter or digit)
	^N		match anything but following match construct
	^EA		match any alphabetic
	^EB		match a separator character (not letter or digit)
	^EC		match symbol constituent
	^ED		match any digit
	^EGq		match any character in q-register q
	^EL		match any line terminator (LF, VT, FF)
	^EM		match non-null string of following match construct
	^ER		match any alphanumeric
	^ES		match non-null string of spaces and/or tabs
	^EV		match lowercase alphabetic
	^EW		match uppercase alphabetic
	^EX		match any character
	^E<nnn>		match character with ASCII code nnn (octal)
	^E[x1,x2,...xn]	match any of the match constructs x1, x2, etc.
	else		match the character itself
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
#include "chmacs.h"		/* define character processing macros */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT CMatch(SamChr)		/* match a character */
BOOLEAN *SamChr;		/* returned match indicator */
{
    BOOLEAN	ChrMat;		/* character match indicator */
    char	OtCase;		/* "other" case character */
    charptr	QRPtr;		/* pointer into q-register text */
    charptr	SavEP2;		/* temporary holder of EBPtr2 */
    charptr	SavSSP;		/* temporary holder of SStPtr */
    DEFAULT	Status;		/* FindQR() status for ^EGq */
    charptr	TCBfPt;		/* temporary holder of CBfPtr */
    charptr	TCStEn;		/* temporary holder of CStEnd */
    unsigned int TmpChr;	/* accumulator for ^E<nnn> */
#if DEBUGGING
    static char *DbgFNm = "CMatch";
    sprintf(DbgSBf,"*SStPtr = '%c', *EBPtr2 = '%c'", *SStPtr, *EBPtr2);
    DbgFEn(3,DbgFNm,DbgSBf);
#endif
    switch (*SStPtr) {
    case CTRL_X:
	*SamChr = TRUE;
	break;
    case CTRL_S:
	*SamChr = !Is_Alnum(*EBPtr2);
	break;
    case CTRL_N:
	if (++SStPtr == SBfPtr) {
	    ErrMsg(ERR_ISS);		/* ill. search str. */
	    DBGFEX(3,DbgFNm,"FAILURE");
	    return FAILURE;
	}
	if (CMatch(&ChrMat) == FAILURE) {
	    DBGFEX(3,DbgFNm,"FAILURE");
	    return FAILURE;
	}
	*SamChr = !ChrMat;
#if DEBUGGING
	sprintf(DbgSBf,"SUCCESS, *SamChr = %s",
		(*SamChr == TRUE) ? "TRUE" : "FALSE");
	DbgFEx(3,DbgFNm,DbgSBf);
#endif
	return SUCCESS;
    case CTRL_E:
	if (++SStPtr == SBfPtr) {
	    ErrMsg(ERR_ICE);		/* ICE = illegal ^E */
	    DBGFEX(3,DbgFNm,"FAILURE");
	    return FAILURE;
	}
	switch (To_Upper(*SStPtr)) {
	case 'A':
	    *SamChr = Is_Alpha(*EBPtr2);
	    break;
	case 'B':
	    *SamChr = !Is_Alnum(*EBPtr2);
	    break;
	case 'C':
	    *SamChr = Is_SyCon(*EBPtr2);
	    break;
	case 'D':
	    *SamChr = Is_Digit(*EBPtr2);
	    break;
	case 'G':
	    if (++SStPtr == SBfPtr) {
		ErrMsg(ERR_ICE); /* ill. ^E command */
		DBGFEX(3,DbgFNm,"FAILURE");
		return FAILURE;
	    }
	    TCBfPt = CBfPtr;	/* save CBfPtr */
	    TCStEn = CStEnd;	/* save CStEnd */
	    CBfPtr = SStPtr;
	    CStEnd = SBfPtr;
	    Status = FindQR();
	    SStPtr = CBfPtr;
	    SBfPtr = CStEnd;
	    CBfPtr = TCBfPt;	/* restore CBfPtr */
	    CStEnd = TCStEn;	/* restore CStEnd */
	    if (Status == FAILURE) {
		DBGFEX(3,DbgFNm,"FAILURE");
		return FAILURE;
	    }
	    QRPtr = QR->Start;
	    while (QRPtr < QR->End_P1) {
		if (*QRPtr++ == *EBPtr2) {
		    *SamChr = TRUE;
#if DEBUGGING
		    sprintf(DbgSBf,"SUCCESS, *SamChr = %s", (*SamChr)?"TRUE":"FALSE");
		    DbgFEx(3,DbgFNm,DbgSBf);
#endif
		    return SUCCESS;
		}
	    }
	    *SamChr = FALSE;
	    break;
	case 'L':
	    *SamChr = IsEOL(*EBPtr2);
	    break;
	case 'M':
	    if (++SStPtr == SBfPtr) {
		ErrMsg(ERR_ICE);
		DBGFEX(3,DbgFNm,"FAILURE");
		return FAILURE;
	    }
	    SavSSP = SStPtr;
	    if (CMatch(&ChrMat) == FAILURE) {
		DBGFEX(3,DbgFNm,"FAILURE");
		return FAILURE;
	    }
	    *SamChr = ChrMat;
	    if (ChrMat) {
		while (EBPtr2 <= RhtSid) {
		    SavEP2 = EBPtr2;
		    ++EBPtr2;
		    SStPtr = SavSSP;
		    if (CMatch(&ChrMat) == FAILURE) {
			DBGFEX(3,DbgFNm,"FAILURE");
			return FAILURE;
		    }
		    if (!ChrMat) {
			EBPtr2 = SavEP2;
			break;
		    }
		}
	    }
#if DEBUGGING
	    sprintf(DbgSBf,"SUCCESS, *SamChr = %s",(*SamChr)?"TRUE":"FALSE");
	    DbgFEx(3,DbgFNm,DbgSBf);
#endif
	    return SUCCESS;
	case 'R':
	    *SamChr = Is_Alnum(*EBPtr2);
	    break;
	case 'S':
	    if ((*EBPtr2 != SPACE) && (*EBPtr2 != TABCHR)) {
		*SamChr = FALSE;
	    } else {
		*SamChr = TRUE;
		while (EBPtr2 <= RhtSid) {
		    ++EBPtr2;
		    if ((*EBPtr2 != SPACE) && (*EBPtr2 != TABCHR)) {
			EBPtr2--;
			break;
		    }
		}
	    }
	    break;
	case 'V':
	    *SamChr = Is_Lower(*EBPtr2);
	    break;
	case 'W':
	    *SamChr = Is_Upper(*EBPtr2);
	    break;
	case 'X':
	    *SamChr = TRUE;
	    break;
	case '<':
	    if (++SStPtr == SBfPtr) {
		ErrMsg(ERR_ICE);
		DBGFEX(3, DbgFNm,"FAILURE");
		return FAILURE;
	    }
	    TmpChr = 0;
	    while (Is_Digit(*SStPtr)) {
		TmpChr *= 8;
		TmpChr += *SStPtr - '0';
		if (TmpChr > 255) {
		    ErrMsg(ERR_ICE);
		    DBGFEX(3,DbgFNm,"FAILURE");
		    return FAILURE;
		}
		if (++SStPtr == SBfPtr) {
		    ErrMsg(ERR_ICE);
		    DBGFEX(3,DbgFNm,"FAILURE");
		    return FAILURE;
		}
	    }
	    if (*SStPtr != '>') {
		ErrMsg(ERR_ICE);
		DBGFEX(3,DbgFNm,"FAILURE");
		return FAILURE;
	    }
	    *SamChr = (*EBPtr2 == (unsigned char)TmpChr);
	    break;
	case '[':
	    if (++SStPtr == SBfPtr) {
		ErrMsg(ERR_ICE);
		DBGFEX(3,DbgFNm,"FAILURE");
		return FAILURE;
	    }
	    while (*SStPtr != ']') {
		if (*SStPtr == ',') {
		    if (++SStPtr == SBfPtr) {
			ErrMsg(ERR_ICE);
			DBGFEX(3,DbgFNm,"FAILURE");
			return FAILURE;
		    }
		} else {
		    if (CMatch(&ChrMat) == FAILURE) {
			DBGFEX(3,DbgFNm,"FAILURE");
			return FAILURE;
		    }
		    if (ChrMat) {
			while (*SStPtr != ']') {
			    if (++SStPtr == SBfPtr) {
				ErrMsg(ERR_ICE);
				DBGFEX(3,DbgFNm,"FAILURE");
				return FAILURE;
			    }
			}
			*SamChr = TRUE;
#if DEBUGGING
			sprintf(DbgSBf,"SUCCESS, *SamChr = %s",
					(*SamChr) ? "TRUE" : "FALSE");
			DbgFEx(3,DbgFNm,DbgSBf);
#endif
			return SUCCESS;
		    }
		    if (++SStPtr == SBfPtr) {
			ErrMsg(ERR_ICE);
			DBGFEX(3,DbgFNm,"FAILURE");
			return FAILURE;
		    }
		}
	    }
	    break;
	default:
	    ErrMsg(ERR_ICE);
	    DBGFEX(3,DbgFNm,"FAILURE");
	    return FAILURE;
	} /* end of ^E switch */
	break;
    default:
	if (SMFlag == 0) {	/* if case independence */
	    OtCase = Is_Upper(*SStPtr) ? To_Lower(*SStPtr)
				       : To_Upper(*SStPtr);
	    *SamChr = ((*EBPtr2 == *SStPtr) || (*EBPtr2 == OtCase));
	} else {
	    *SamChr = (*EBPtr2 == *SStPtr);
	}
    } /* end of switch */
#if DEBUGGING
    sprintf(DbgSBf,"SUCCESS, *SamChr = %s", (*SamChr) ? "TRUE" : "FALSE");
    DbgFEx(3,DbgFNm,DbgSBf);
#endif
    return SUCCESS;
}
