/*****************************************************************************
	BakSrc()
	This recursive function does a backward search in the edit buffer,
looking for a string which will match the first match construct in the search
buffer.  Basically,  it is the high-speed part of the search algorithm: it
scans the edit buffer looking for the first character in the search string.
	On entry, SStPtr points to the first match construct in the search
buffer.  On exit,  SStPtr points to the last character of the first match
construct in the search buffer.
	SBfPtr points to the character following the last character in
the search string.  This function does not modify SBfPtr.
	On entry,  EBPtr1 points to the place in the edit buffer where
the search starts.  On exit,  if the search was successful,  EBPtr1 will
point to the found character.  If the search was unsuccessful,  EBPtr1 will
be less than EndSAr.
	On entry,  EBPtr2 is undefined.  On exit,  if the search was
successful,  EBPtr2 points to the last character of the found string.  If
the search was unsuccessful,  EBPtr2 is undefined.
	EndSAr points to the end of the search area (where the search ends).
Note that for backwards searches,  EndSAr is less than EBPtr1.  This function
does not modify EndSAr.
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
DEFAULT BakSrc()		/* forward search for 1st search char */
{
    unsigned char Charac;	/* holds a character */
    charptr	LstBeg;		/* beginning of ^E[x1,x2,...] list */
    char	OtCase;		/* "other" case character */
    charptr	QRPtr;		/* pointer into q-register text */
    BOOLEAN	SamChr;		/* same character indicator */
    charptr	SavEP2;		/* temporary holder of EBPtr2 */
    charptr	SavSSP;		/* temporary holder of SStPtr */
    DEFAULT	Status;		/* FindQR() status for ^EGq */
    charptr	TCBfPt;		/* temporary holder of CBfPtr */
    charptr	TCStEn;		/* temporary holder of CStEnd */
    LONG	TmpLng;		/* holds octal number for ^E<nnn> */
#if DEBUGGING
    static char *DbgFNm = "BakSrc";
    sprintf(DbgSBf,"*SStPtr = '%c', *EBPtr1 = '%c'", *SStPtr, *EBPtr1);
    DbgFEn(3,DbgFNm,DbgSBf);
#endif
    switch (*SStPtr) {
	case CTRL_X:	/* ^X match any char */
    	    break;
	case CTRL_S:	/* ^S match separator char */
	    for (; EBPtr1 >= EndSAr; --EBPtr1) {
	        if (!Is_Alnum(*EBPtr1)) {
		    break;
		}
	    }
	    break;
	case CTRL_N:	/* ^Nx match any BUT next match construct */
	    if (++SStPtr == SBfPtr) {
	        ErrMsg(ERR_ISS);		/* illegal search string */
		DBGFEX(3,DbgFNm,"FAILURE, no ^N arg");
		return FAILURE;
	    }
	    SavSSP = SStPtr;
	    for (;EBPtr1>=EndSAr;--EBPtr1) {
	        EBPtr2 = EBPtr1;
		SStPtr = SavSSP;
		if (CMatch(&SamChr) == FAILURE) {
		    DBGFEX(3,DbgFNm,"FAILURE, CMatch failed");
		    return FAILURE;
		}
		if (!SamChr) {
		    break;
		}
	    }
	    break;
	case CTRL_E:	/* ^E match construct */
	    if (++SStPtr == SBfPtr) {
	        ErrMsg(ERR_ICE);	/* ICE = illegal ^E */
		DBGFEX(3,DbgFNm,"FAILURE, no ^E arg");
		return FAILURE;
	    }
	    switch (To_Upper(*SStPtr)) {
	        case 'A':	/* ^EA match any alphabetic */
		    for (; EBPtr1 >= EndSAr; --EBPtr1) {
		        if (Is_Alpha(*EBPtr1)) {
			    break;
			}
		    }
		    break;
		case 'B':	/* ^EB match any separator (==^S) */
		    for (; EBPtr1 >= EndSAr; --EBPtr1) {
		        if (!Is_Alnum(*EBPtr1)) {
			    break;
			}
		    }
		    break;
		case 'C':	/* ^EC match symbol constitient */
		    for (; EBPtr1 >= EndSAr; --EBPtr1) {
		        if (Is_SyCon(*EBPtr1)) {
			    break;
			}
		    }
		    break;
		case 'D':	/* ^ED match any digit */
		    for (; EBPtr1 >= EndSAr; --EBPtr1) {
		        if (Is_Digit(*EBPtr1)) {
			    break;
			}
		    }
		    break;
		case 'G':	/* ^EGq match any char in Q-reg q */
		    if (++SStPtr == SBfPtr) {
		        ErrMsg(ERR_ICE);
			DBGFEX(3,DbgFNm,"FAILURE, no ^EG arg");
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
		        DBGFEX(3,DbgFNm,"FAILURE, ^EG FindQR failed");
			    return FAILURE;
		    }	
		    for (; EBPtr1 >= EndSAr; --EBPtr1) {
		        QRPtr = QR->Start;
			while (QRPtr < QR->End_P1) {
			    if (*QRPtr++ == *EBPtr1) {
			        goto kludge;
			    }
			}
		    }
kludge:		    break;
		case 'L':	/* ^EL match line terminator */
		    for (; EBPtr1 >= EndSAr; --EBPtr1) {
		        if (IsEOL(*EBPtr1)) {
			    break;
			}
		    }
		    break;
		case 'M':	/* ^EM match multiple next constructs */
		    if (++SStPtr == SBfPtr) {
		        ErrMsg(ERR_ICE);
			DBGFEX(3,DbgFNm,"FAILURE, no ^EM arg");
			return FAILURE;
		    }
		    SavSSP = SStPtr;
		    if (BakSrc() == FAILURE) {
		        DBGFEX(3,DbgFNm,"FAILURE");
			return FAILURE;
		    }
		    if (EBPtr1 < EndSAr) {	/* if not found */
		        break;
		    }
		    SavEP2 = EBPtr2;
		    while (EBPtr1 > EndSAr) {
		        EBPtr1--;
			EBPtr2 = EBPtr1;
			SStPtr = SavSSP;
			if (CMatch(&SamChr) == FAILURE) {
			    DBGFEX(3,DbgFNm,"FAILURE");
			    return FAILURE;
			}
			if (!SamChr) {
			    EBPtr1++;
			    EBPtr2 = SavEP2;
			    break;
			}
		    }
		    DBGFEX(3,DbgFNm,"SUCCESS");
		    return SUCCESS;
		case 'R':	/* ^ER match any alphanumeric */
		    for (; EBPtr1 >= EndSAr; --EBPtr1) {
		        if (Is_Alnum(*EBPtr1)) {
			    break;
			}
		    }
		    break;
		case 'S':	/* ^ES match any spaces/tabs */
		    for (; EBPtr1 >= EndSAr; --EBPtr1) {
		        if ((*EBPtr1 == SPACE) || (*EBPtr1 == TABCHR)) {
			    EBPtr2 = EBPtr1;
			    while (EBPtr1 > EndSAr) {
			        EBPtr1--;
				if ((*EBPtr1 != SPACE) &&
				    (*EBPtr1 != TABCHR)) {
				    EBPtr1++;
				    break;
				}
			    }
			    DBGFEX(3,DbgFNm,"SUCCESS");
			    return SUCCESS;
			}
		    }
		    break;
		case 'V':	/* ^EV match any lowercase char */
		    for (; EBPtr1 >= EndSAr; --EBPtr1) {
		        if (Is_Lower(*EBPtr1)) {
			    break;
			}
		    }
		    break;
		case 'W':	/* ^EW match any uppercase character */
		    for (; EBPtr1 >= EndSAr; --EBPtr1) {
		        if (Is_Upper(*EBPtr1)) {
		            break;
			}
		    }
		    break;
		case 'X':	/* ^EX match any char (==^X) */
		    break;
		case '<':	/* ^E<n> match char w/ASCII octal code n */
		    if (++SStPtr == SBfPtr) {
		        ErrMsg(ERR_ICE);
			DBGFEX(3,DbgFNm,"FAILURE, no ^E<> arg");
			return FAILURE;
		    }
		    TmpLng = 0;
		    while (Is_Digit(*SStPtr)) {
		        TmpLng = (TmpLng * 8) + (*SStPtr - '0');
			if (TmpLng > 127) {
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
		    Charac = (unsigned char)TmpLng;
		    for (; EBPtr1 >= EndSAr; --EBPtr1) {
		        if (*EBPtr1 == Charac) {
			    break;
			}
		    }
		    break;
		case '[':	/* ^E[x1,x2,...] match any one of x1,x2,... */
		    if (++SStPtr == SBfPtr) {
		        ErrMsg(ERR_ICE);
			DBGFEX(3,DbgFNm,"FAILURE, no ^E[] arg");
			return FAILURE;
		    }
		    LstBeg = SStPtr;
		    for (; EBPtr1 >= EndSAr; --EBPtr1) {
			while (*SStPtr != ']') {
		            if (*SStPtr == ',') {
			        if (++SStPtr == SBfPtr) {
			            ErrMsg(ERR_ICE);
				    DBGFEX(3,DbgFNm,"FAILURE");
				    return FAILURE;
				}
			    } else {
			        EBPtr2 = EBPtr1;
				if (CMatch(&SamChr) == FAILURE) {
			            DBGFEX(3,DbgFNm,"FAILURE");
				    return FAILURE;
				}
				if (SamChr) {
			            while (*SStPtr != ']') {
				        if (++SStPtr == SBfPtr) {
				            ErrMsg(ERR_ICE);
					    DBGFEX(3,DbgFNm,"FAILURE");
					    return FAILURE;
					}
				    }
				    EBPtr2 = EBPtr1;
				    DBGFEX(3,DbgFNm,"SUCCESS");
				    return SUCCESS;
				}
				if (++SStPtr == SBfPtr) {
			            ErrMsg(ERR_ICE);
				    DBGFEX(3,DbgFNm,"FAILURE");
				    return FAILURE;
				}
			    }
			}
			SStPtr = LstBeg;
		    }
		    break;
		default:
		    ErrMsg(ERR_ICE);
		    DBGFEX(3,DbgFNm,"FAILURE, bad ^E arg");
		    return FAILURE;
	    } /* end of ^E switch */
	    break;
	default:	/* edit buffer char must match search string char */
	    if (SMFlag) {			/* if case dependence */
	        for (; EBPtr1 >= EndSAr; --EBPtr1) {
		    if (*EBPtr1 == *SStPtr) {
		        break;
		    }
		}
	    } else {			/* else case independence */
	        OtCase = Is_Upper(*SStPtr)
		             ? To_Lower(*SStPtr)
			     : To_Upper(*SStPtr);
		for (; EBPtr1 >= EndSAr; --EBPtr1) {
		    if ((*EBPtr1 == *SStPtr) || (*EBPtr1 == OtCase)) {
		        break;
		    }
		}
	    }
    } /* end of switch */
    EBPtr2 = EBPtr1;
    DBGFEX(3,DbgFNm,"SUCCESS");
    return SUCCESS;
}
