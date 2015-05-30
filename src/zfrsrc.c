/*****************************************************************************
	ZFrSrc()
	This recursive function does a forward search in the edit buffer,
looking for a string which will match the first match construct in the search
buffer.  Basically,  it is the high-speed part of the search algorithm: it
scans the edit buffer looking for the first character in the search string.
	The VAX SCANC instruction implements most of the meat of this
function so well that I couldn't resist using it.  For environments other
than VMS,  the generic code exists to do the search.  That's why this has to
be a system-dependent piece of code.  Note that I did not use SCANC for some
of the match constructs,  including the "default" case.  I started to,  but
the resulting code was real clumsy and in timing tests it didn't seem worth
the confusion.
	On entry, SStPtr points to the first match construct in the search
buffer.  On exit,  SStPtr points to the last character of the first match
construct in the search buffer.
	SBfPtr points to the character following the last character in
the search string.  This function does not modify SBfPtr.
	On entry,  EBPtr1 points to the place in the edit buffer where
the search starts.  On exit,  if the search was successful,  EBPtr1 will
point to the found character.  If the search was unsuccessful,  EBPtr1 will
be greater than EndSAr.
	On entry,  EBPtr2 is undefined.  On exit,  if the search was
successful,  EBPtr2 points to the last character of the found string.  If
the search was unsuccessful,  EBPtr2 is undefined.
	EndSAr points to the end of the search area (where the search ends).
This function does not modify EndSAr.
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
#if defined(VAX11C)
#include descrip		/* define "$descriptor" macro */
int	lib$scanc();		/* scan for characters */
DEFAULT ZFrSrc()		/* forward search for 1st search charptr */
{
	char	Charac;		/* holds a character */
	int	indx;		/* returned by lib$scanc */
	charptr	LstBeg;		/* beginning of ^E[x,x,x] list */
	char	mask;
	char	OtCase;		/* "other" case character */
	charptr	QTPtr;		/* pointer into q-register text */
	int	real_length;	/* length of area to be searched */
	BOOLEAN	SamChr;		/* same character indicator */
	charptr	SavEP2;		/* temporary holder of EBPtr2 */
	charptr	SavSSP;		/* temporary holder of SStPtr */
	struct	dsc$descriptor_s src_str = {
		0,		/* dsc$w_length */
		DSC$K_DTYPE_T,	/* dsc$b_dtype */
		DSC$K_CLASS_S,	/* dsc$b_class */
		0		/* dsc$a_pointer */
	};
	DEFAULT	Status;
	charptr	TCBfPt;		/* temporary holder of CBfPtr */
	charptr	TCStEn;		/* temporary holder of CStEnd */
	LONG	TmpLng;
#define SEPARATOR	'\001'	/* separators (not a letter or digit) */
#define ALPHA		'\002'	/* alphabetics */
#define SYMBOL		'\004'	/* symbol constituents */
#define DIGIT		'\010'	/* digits */
#define LINE_TERMINATOR	'\020'	/* line terminators (lf, vt, ff) */
#define ALPHANUMERIC	'\040'	/* alphanumerics */
#define LOWER		'\100'	/* lowercase */
#define UPPER		'\200'	/* uppercase */
static readonly char table_1[256] =
		{
		SEPARATOR,				/* null */
		SEPARATOR,				/* ^A */
		SEPARATOR,				/* ^B */
		SEPARATOR,				/* ^C */
		SEPARATOR,				/* ^D */
		SEPARATOR,				/* ^E */
		SEPARATOR,				/* ^F */
		SEPARATOR,				/* ^G (bell) */
		SEPARATOR,				/* ^H (bs) */
		SEPARATOR,				/* ^I (tab) */
		LINE_TERMINATOR | SEPARATOR,		/* ^J (lf) */
		LINE_TERMINATOR | SEPARATOR,		/* ^K (vt) */
		LINE_TERMINATOR | SEPARATOR,		/* ^L (ff) */
		SEPARATOR,				/* ^M (cr) */
		SEPARATOR,				/* ^N */
		SEPARATOR,				/* ^O */
		SEPARATOR,				/* ^P */
		SEPARATOR,				/* ^Q */
		SEPARATOR,				/* ^R */
		SEPARATOR,				/* ^S */
		SEPARATOR,				/* ^T */
		SEPARATOR,				/* ^U */
		SEPARATOR,				/* ^V */
		SEPARATOR,				/* ^W */
		SEPARATOR,				/* ^X */
		SEPARATOR,				/* ^Y */
		SEPARATOR,				/* ^Z */
		SEPARATOR,				/* escape */
		SEPARATOR,				/* FS */
		SEPARATOR,				/* GS */
		SEPARATOR,				/* RS */
		SEPARATOR,				/* US */
		SEPARATOR,				/* space */
		SEPARATOR,				/* ! */
		SEPARATOR,				/* " */
		SEPARATOR,				/* # */
		SYMBOL & SEPARATOR,			/* $ */
		SEPARATOR,				/* % */
		SEPARATOR,				/* | */
		SEPARATOR,				/* ' */
		SEPARATOR,				/* ( */
		SEPARATOR,				/* ) */
		SEPARATOR,				/* * */
		SEPARATOR,				/* + */
		SEPARATOR,				/* , */
		SEPARATOR,				/* - */
		SYMBOL | SEPARATOR,			/* . */
		SEPARATOR,				/* / */
		ALPHANUMERIC | DIGIT | SYMBOL,		/* 0 */
		ALPHANUMERIC | DIGIT | SYMBOL,		/* 1 */
		ALPHANUMERIC | DIGIT | SYMBOL,		/* 2 */
		ALPHANUMERIC | DIGIT | SYMBOL,		/* 3 */
		ALPHANUMERIC | DIGIT | SYMBOL,		/* 4 */
		ALPHANUMERIC | DIGIT | SYMBOL,		/* 5 */
		ALPHANUMERIC | DIGIT | SYMBOL,		/* 6 */
		ALPHANUMERIC | DIGIT | SYMBOL,		/* 7 */
		ALPHANUMERIC | DIGIT | SYMBOL,		/* 8 */
		ALPHANUMERIC | DIGIT | SYMBOL,		/* 9 */
		SEPARATOR,				/* : */
		SEPARATOR,				/* ; */
		SEPARATOR,				/* < */
		SEPARATOR,				/* = */
		SEPARATOR,				/* > */
		SEPARATOR,				/* ? */
		SEPARATOR,				/* @ */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* A */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* B */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* C */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* D */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* E */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* F */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* G */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* H */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* I */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* J */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* K */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* L */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* M */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* N */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* O */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* P */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* Q */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* R */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* S */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* T */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* U */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* V */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* W */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* X */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* Y */
		UPPER | ALPHANUMERIC | SYMBOL | ALPHA,	/* Z */
		SEPARATOR,				/* [ */
		SEPARATOR,				/* \ */
		SEPARATOR,				/* ] */
		SEPARATOR,				/* ^ */
		SYMBOL | SEPARATOR,			/* _ */
		SEPARATOR,				/* ` */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* a */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* b */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* c */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* d */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* e */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* f */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* g */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* h */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* i */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* j */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* k */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* l */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* m */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* n */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* o */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* p */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* q */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* r */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* s */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* t */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* u */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* v */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* w */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* x */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* y */
		LOWER | ALPHANUMERIC | SYMBOL | ALPHA,	/* z */
		SEPARATOR,				/* { */
		SEPARATOR,				/* | */
		SEPARATOR,				/* } */
		SEPARATOR,				/* ~ */
		SEPARATOR,				/* delete */
		'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
		'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
		'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
		'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
		'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
		'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
		'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
		'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'
		};
#if DEBUGGING
	static char *DbgFNm = "ZFrSrc";
	sprintf(DbgSBf,"*SStPtr = '%c', *EBPtr1 = '%c'", *SStPtr, *EBPtr1);
	DbgFEn(3,DbgFNm,DbgSBf);
#endif
	switch (*SStPtr) {
	case CTRL_X:
		break;
	case CTRL_S:
		mask = SEPARATOR;
		goto scanc;
	case CTRL_N:
		if (++SStPtr == SBfPtr) {
			ErrMsg(ERR_ISS);	/* ill. search str. */
			DBGFEX(3,DbgFNm,"FAILURE");
			return FAILURE;
		}
		SavSSP = SStPtr;
		for (; EBPtr1 <= EndSAr; ++EBPtr1) {
			EBPtr2 = EBPtr1;
			SStPtr = SavSSP;
			if (CMatch(&SamChr) == FAILURE) {
				DBGFEX(3,DbgFNm,"FAILURE, CMatch() failed");
				return FAILURE;
			}
			if (!SamChr)
				break;
		}
		break;
	case CTRL_E:
		if (++SStPtr == SBfPtr) {
			ErrMsg(ERR_ICE);	/* ICE = illegal ^E */
			DBGFEX(3,DbgFNm,"FAILURE");
			return FAILURE;
		}
		switch (To_Upper(*SStPtr)) {
		case 'A':
			mask = ALPHA;
			goto scanc;
		case 'B':
			mask = SEPARATOR;
			goto scanc;
		case 'C':
			mask = SYMBOL;
			goto scanc;
		case 'D':
			mask = DIGIT;
			goto scanc;
		case 'G':
			if (++SStPtr == SBfPtr) {
				ErrMsg(ERR_ICE);
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
			for (;EBPtr1<=EndSAr;++EBPtr1) {
				QTPtr = QR->Start;
				while (QTPtr < QR->End_P1)
					if (*QTPtr++ == *EBPtr1)
						goto kludge;
			}
kludge:			break;
		case 'L':
			mask = LINE_TERMINATOR;
			goto scanc;
		case 'M':
			if (++SStPtr == SBfPtr) {
				ErrMsg(ERR_ICE);
				DBGFEX(3,DbgFNm,"FAILURE");
				return FAILURE;
			}
			SavSSP = SStPtr;
			if (ZFrSrc() == FAILURE) {
				DBGFEX(3,DbgFNm,"FAILURE");
				return FAILURE;
			}
			if (EBPtr1 > EndSAr) {	/* if not found */
				break;
			}
			while (EBPtr2 < EndSAr) {
				SavEP2 = EBPtr2++;
				SStPtr = SavSSP;
				if (CMatch(&SamChr) == FAILURE) {
					DBGFEX(3,DbgFNm,"FAILURE");
					return FAILURE;
				}
				if (!SamChr) {
					EBPtr2 = SavEP2;
					break;
				}
			}
			DBGFEX(3,DbgFNm,"SUCCESS");
			return SUCCESS;
		case 'R':
			mask = ALPHANUMERIC;
			goto scanc;
		case 'S':
			for (;EBPtr1<=EndSAr;++EBPtr1) {
				if ((*EBPtr1 == SPACE) || (*EBPtr1 == TABCHR)) {
					EBPtr2 = EBPtr1;
					while (EBPtr2 < EndSAr) {
						++EBPtr2;
						if ((*EBPtr2 != SPACE) && (*EBPtr2 != TABCHR)) {
							EBPtr2--;
							break;
						}
					}
					DBGFEX(3,DbgFNm,"SUCCESS");
					return SUCCESS;
				}
			}
			break;
		case 'V':
			mask = LOWER;
			goto scanc;
		case 'W':
			mask = UPPER;
			goto scanc;
		case 'X':
			break;
		case '<':
			if (++SStPtr == SBfPtr) {
				ErrMsg(ERR_ICE);
				DBGFEX(3,DbgFNm,"FAILURE");
				return FAILURE;
			}
			TmpLng = 0;
			while (Is_Digit(*SStPtr)) {
				TmpLng *= 8;
				TmpLng += *SStPtr - '0';
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
			Charac = (char)TmpLng;
			for (; EBPtr1 <= EndSAr; ++EBPtr1) {
				if (*EBPtr1 == Charac) {
					break;
				}
			}
			break;
		case '[':
			if (++SStPtr == SBfPtr) {
				ErrMsg(ERR_ICE);
				DBGFEX(3,DbgFNm,"FAILURE");
				return FAILURE;
			}
			LstBeg = SStPtr;
			for (;EBPtr1<=EndSAr;++EBPtr1) {
				while (*SStPtr != ']') {
					if (*SStPtr == ', ') {
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
					}
					if (++SStPtr == SBfPtr) {
						ErrMsg(ERR_ISS);
						DBGFEX(3,DbgFNm,"FAILURE");
						return FAILURE;
					}
				}
				SStPtr = LstBeg;
			}
			break;
		default:
			ErrMsg(ERR_ICE);
			DBGFEX(3,DbgFNm,"FAILURE");
			return FAILURE;
		}
		break;
	default:
		if (SMFlag) {		/* if case dependence */
			for (;EBPtr1<=EndSAr;++EBPtr1) {
				if (*EBPtr1 == *SStPtr) {
					break;
				}
			}
		} else {			/* else case independence */
			OtCase = (Is_Upper(*SStPtr)) ?
					To_Lower(*SStPtr) :
					To_Upper(*SStPtr);
			for (;EBPtr1<=EndSAr;++EBPtr1) {
				if ((*EBPtr1 == *SStPtr) ||
				    (*EBPtr1 == OtCase)) {
					break;
				}
			}
		}
		break;
	} /* end of switch */
	EBPtr2 = EBPtr1;
#if DEBUGGING
	sprintf(DbgSBf,"SUCCESS, EBPtr1 = %ld, EndSAr = %ld",
		Zcp2ul(EBPtr1), Zcp2ul(EndSAr));
	DbgFEx(3,DbgFNm,DbgSBf);
#endif
	return SUCCESS;
scanc:	real_length = EndSAr - EBPtr1;
	real_length++;
	src_str.dsc$a_pointer = EBPtr1;
	while (real_length > 0) {
		src_str.dsc$w_length = (real_length > 65535) ?
						65535 :
						real_length;
		indx = lib$scanc(&src_str, table_1, &mask);
		if (indx) {			/* if scan succeeded */
			EBPtr1 += indx;
			EBPtr1--;
			EBPtr2 = EBPtr1;
			DBGFEX(3,DbgFNm,"SUCCESS");
			return SUCCESS;
		}
		real_length -= 65535;
		src_str.dsc$a_pointer += 65535;
	}
	EBPtr1 = EndSAr;
	EBPtr1++;
	DBGFEX(3,DbgFNm,"SUCCESS");
	return SUCCESS;
}
#else
DEFAULT ZFrSrc()		/* forward search for 1st search charptr */
{
	char	Charac;		/* holds a character */
	charptr	LstBeg;		/* beginning of ^E[x,x,x] list */
	char	OtCase;		/* "other" case character */
	charptr	QTPtr;		/* pointer into q-register text */
	BOOLEAN	SamChr;		/* same character indicator */
	charptr	SavEP2;		/* temporary holder of EBPtr2 */
	charptr	SavSSP;		/* temporary holder of SStPtr */
	DEFAULT	Status;
	charptr	TCBfPt;		/* temporary holder of CBfPtr */
	charptr	TCStEn;		/* temporary holder of CStEnd */
	LONG	TmpLng;
#if DEBUGGING
	static char *DbgFNm = "ZFrSrc";
	sprintf(DbgSBf,"*SStPtr = '%c', *EBPtr1 = '%c'", *SStPtr, *EBPtr1);
	DbgFEn(3,DbgFNm,DbgSBf);
#endif
	switch (*SStPtr) {
		case CTRL_X:
			break;
		case CTRL_S:
			for (;EBPtr1<=EndSAr;++EBPtr1)
				if (!Is_Alnum(*EBPtr1))
					break;
			break;
		case CTRL_N:
			if (++SStPtr == SBfPtr)
				{
				ErrMsg(ERR_ISS);	/* ill. search str. */
				DBGFEX(3,DbgFNm,"FAILURE");
				return FAILURE;
				}
			SavSSP = SStPtr;
			for (;EBPtr1<=EndSAr;++EBPtr1)
				{
				EBPtr2 = EBPtr1;
				SStPtr = SavSSP;
				if (CMatch(&SamChr) == FAILURE) {
					DBGFEX(3,DbgFNm,"FAILURE");
					return FAILURE;
				}
				if (!SamChr)
					break;
				}
			break;
		case CTRL_E:
			if (++SStPtr == SBfPtr)
				{
				ErrMsg(ERR_ICE);	/* ICE = illegal ^E */
				DBGFEX(3,DbgFNm,"FAILURE");
				return FAILURE;
				}
			switch (To_Upper(*SStPtr)) {
			case 'A':
				for (;EBPtr1<=EndSAr;++EBPtr1)
					if (Is_Alpha(*EBPtr1))
						break;
				break;
			case 'B':
				for (;EBPtr1<=EndSAr;++EBPtr1)
					if (!Is_Alnum(*EBPtr1))
						break;
				break;
			case 'C':
				for (;EBPtr1<=EndSAr;++EBPtr1)
					if (Is_SyCon(*EBPtr1))
						break;
				break;
			case 'D':
				for (;EBPtr1<=EndSAr;++EBPtr1)
					if (Is_Digit(*EBPtr1))
						break;
				break;
			case 'G':
				if (++SStPtr == SBfPtr)
					{
					ErrMsg(ERR_ICE);
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
				for (;EBPtr1<=EndSAr;++EBPtr1)
					{
					QTPtr = QR->Start;
					while (QTPtr < QR->End_P1)
						if (*QTPtr++ == *EBPtr1)
							goto kludge;
					}
kludge:				break;
			case 'L':
				for (;EBPtr1<=EndSAr;++EBPtr1)
					if (IsEOL(*EBPtr1))
						break;
				break;
			case 'M':
				if (++SStPtr == SBfPtr)
					{
					ErrMsg(ERR_ICE);
					DBGFEX(3,DbgFNm,"FAILURE");
					return FAILURE;
					}
				SavSSP = SStPtr;
				if (ZFrSrc() == FAILURE) {
					DBGFEX(3,DbgFNm,"FAILURE");
					return FAILURE;
				}
				if (EBPtr1 > EndSAr)	/* if not found */
					break;
				while (EBPtr2 < EndSAr)
					{
					SavEP2 = EBPtr2++;
					SStPtr = SavSSP;
					if (CMatch(&SamChr) == FAILURE) {
						DBGFEX(3,DbgFNm,"FAILURE");
						return FAILURE;
					}
					if (!SamChr)
						{
						EBPtr2 = SavEP2;
						break;
						}
					}
				DBGFEX(3,DbgFNm,"SUCCESS");
				return SUCCESS;
			case 'R':
				for (;EBPtr1<=EndSAr;++EBPtr1)
					if (Is_Alnum(*EBPtr1))
						break;
				break;
			case 'S':
				for (;EBPtr1<=EndSAr;++EBPtr1)
					if ((*EBPtr1 == SPACE) ||
					    (*EBPtr1 == TABCHR))
						{
						EBPtr2 = EBPtr1;
						while (EBPtr2 < EndSAr)
						    {
						    ++EBPtr2;
						    if ((*EBPtr2 != SPACE) &&
							(*EBPtr2 != TABCHR))
							    {
							    EBPtr2--;
							    break;
							    }
						    }
						DBGFEX(3,DbgFNm,"SUCCESS");
						return SUCCESS;
						}
				break;
			case 'V':
				for (;EBPtr1<=EndSAr;++EBPtr1)
					if (Is_Lower(*EBPtr1))
						break;
				break;
			case 'W':
				for (;EBPtr1<=EndSAr;++EBPtr1)
					if (Is_Upper(*EBPtr1))
						break;
				break;
			case 'X':
				break;
			case '<':
				if (++SStPtr == SBfPtr)
					{
					ErrMsg(ERR_ICE);
					DBGFEX(3,DbgFNm,"FAILURE");
					return FAILURE;
					}
				TmpLng = 0;
				while (Is_Digit(*SStPtr))
					{
					TmpLng *= 8;
					TmpLng += *SStPtr - '0';
					if (TmpLng > 127)
						{
						ErrMsg(ERR_ICE);
						DBGFEX(3,DbgFNm,"FAILURE");
						return FAILURE;
						}
					if (++SStPtr == SBfPtr)
						{
						ErrMsg(ERR_ICE);
						DBGFEX(3,DbgFNm,"FAILURE");
						return FAILURE;
						}
					}
				if (*SStPtr != '>')
					{
					ErrMsg(ERR_ICE);
					DBGFEX(3,DbgFNm,"SUCCESS");
					return FAILURE;
					}
				Charac = (char)TmpLng;
				for (;EBPtr1<=EndSAr;++EBPtr1)
					if (*EBPtr1 == Charac)
						break;
				break;
			case '[':
				if (++SStPtr == SBfPtr)
					{
					ErrMsg(ERR_ICE);
					DBGFEX(3,DbgFNm,"FAILURE");
					return FAILURE;
					}
				LstBeg = SStPtr;
				for (;EBPtr1<=EndSAr;++EBPtr1)
					{
					while (*SStPtr != ']')
						if (*SStPtr == ',')
							{
							if (++SStPtr == SBfPtr)
								{
								ErrMsg(ERR_ICE);
								DBGFEX(3,DbgFNm,"FAILURE");
								return FAILURE;
								}
							}
						else
							{
						EBPtr2 = EBPtr1;
						if (CMatch(&SamChr) == FAILURE) {
							DBGFEX(3,DbgFNm,"FAILURE");
							return FAILURE;
						}
						if (SamChr)
							{
							while (*SStPtr != ']')
							if (++SStPtr == SBfPtr)
								{
								ErrMsg(ERR_ICE);
								DBGFEX(3,DbgFNm,"FAILURE");
								return FAILURE;
								}
							EBPtr2 = EBPtr1;
							DBGFEX(3,DbgFNm,"SUCCESS");
							return SUCCESS;
							}
						if (++SStPtr == SBfPtr)
							{
							ErrMsg(ERR_ISS);
							DBGFEX(3,DbgFNm,"FAILURE");
							return FAILURE;
							}
						}
					SStPtr = LstBeg;
					}
				break;
			default:
				ErrMsg(ERR_ICE);
				DBGFEX(3,DbgFNm,"FAILURE");
				return FAILURE;
			}
			break;
		default:
			if (SMFlag)		/* if case dependence */
				{
				for (;EBPtr1<=EndSAr;++EBPtr1)
					if (*EBPtr1 == *SStPtr)
						break;
				}
			else			/* else case independence */
				{
				OtCase = (Is_Upper(*SStPtr)) ?
						To_Lower(*SStPtr) :
						To_Upper(*SStPtr);
				for (;EBPtr1<=EndSAr;++EBPtr1)
					if ((*EBPtr1 == *SStPtr) ||
					    (*EBPtr1 == OtCase))
						break;
				}
	}	/* end of switch */
	EBPtr2 = EBPtr1;
#if DEBUGGING
	sprintf(DbgSBf,"SUCCESS, EBPtr1 = %ld, EndSAr = %ld",
		Zcp2ul(EBPtr1), Zcp2ul(EndSAr));
	DbgFEx(3,DbgFNm,DbgSBf);
#endif
	return SUCCESS;
}
#endif
