/*****************************************************************************
	SSerch()
	This function does a "simple" search.  It is passed a direction
and the starting and ending addresses of the contiguous area to be searched.
It sets Matchd, which indicates whether the search was successful or not.
If it was successful, EBPtr2 is left pointing to the last character of the
found string.  Note that if the direction is backwards, the address of the
beginning of the search area will be greater than the address of the end of
the search area.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT SSerch()		/* search for 1 occurrence of a string */
{
	BOOLEAN	SamChr = FALSE;	/* same character indicator */
#if DEBUGGING
	static char *DbgFNm = "SSerch";
	sprintf(DbgSBf,"SIncrm = %ld, EBPtr1 = %ld, EndSAr = %ld",
		SIncrm, Zcp2ul(EBPtr1), Zcp2ul(EndSAr));
	DbgFEn(3,DbgFNm,DbgSBf);
	sprintf(DbgSBf,"search string SBf = \"%.*s\"",
		(int)(SBfPtr-SBfBeg), SBfBeg);
	DbgFMs(3,DbgFNm,DbgSBf);
#endif
	if (SBfPtr == SBfBeg) {			/* if null search string */
		Matchd = FALSE;
		DBGFEX(3,DbgFNm,"SUCCESS, Matchd = FALSE");
		return SUCCESS;
	}
	FOREVER {
		SStPtr = SBfBeg;
/*
 * find a character which matches the first
 * character of the search string
 */
		if (SIncrm == 1) {		/* if forward search */
			if (ZFrSrc() == FAILURE) {
				DBGFEX(3,DbgFNm,"FAILURE, ZFrSrc() failed");
				return FAILURE;
			}
			if (EBPtr1 > EndSAr) {		/* if not found */
				Matchd = FALSE;
				DBGFEX(3,DbgFNm,"SUCCESS, Matchd = FALSE");
				return SUCCESS;
			}
		} else {
			if (BakSrc() == FAILURE) {
				DBGFEX(3,DbgFNm,"FAILURE, BakSrc() failed");
				return FAILURE;
			}
			if (EBPtr1 < EndSAr) {		/* if not found */
				Matchd = FALSE;
				DBGFEX(3,DbgFNm,"SUCCESS, Matchd = FALSE");
				return SUCCESS;
			}
		}
/*
 * first char is matched,  now check rest of search string
 */
		do {
/*
 * ">=" rather than "==":  a ^EGq whose Q-register name is missing used to be
 * able to leave SStPtr one PAST SBfPtr,  after which an equality test could
 * never fire again and the match loop walked off the end of the search string.
 * The call sites no longer let that happen,  but the test costs nothing.
 */
			if (++SStPtr >= SBfPtr) { /* if no more to check */
				RefLen = (EBPtr1 - EBPtr2) - 1;
/*
 * EBPtr1 is the first character of the match and EBPtr2 the last,  so the
 * pointer difference counts the gap too whenever the match spans it.  The test
 * used to be "SIncrm == -1 && EBPtr2 >= GapEnd",  which both missed the forward
 * span -- an m,nFB whose area lies before the gap can now match across it --
 * and fired for a backwards m,nFB whose area lies wholly AFTER the gap,  where
 * nothing was spanned at all.
 */
				if ((EBPtr1 < GapBeg) && (EBPtr2 > GapEnd)) {
					RefLen += GapEnd-GapBeg+1;
				}
				Matchd = TRUE;
				DBGFEX(3,DbgFNm,"SUCCESS, Matchd = TRUE");
				return SUCCESS;
			}
			if (++EBPtr2 > RhtSid) { /* inc and check for eob */
/*
 * RhtSid is the last character BEFORE the edit buffer gap whenever the search
 * area stops short of the gap -- that is every backwards search,  and the m,n
 * form of FB/FC when the area lies before the gap.  Passing it then just means
 * EBPtr2 has walked into the gap and has to be stepped over it.  RhtSid says
 * nothing about the end of the edit buffer in that case,  so the end has to be
 * tested separately.  Without this test the match walked off the end of the
 * edit buffer and could "succeed" against memory outside it,  after which
 * SrcLop moved the gap to a position beyond Z and wrecked the buffer.
 *	The gap runs from GapBeg to GapEnd INCLUSIVE,  so "inside the gap" is
 * ">= GapBeg && <= GapEnd",  which is what CMatch and ZFrSrc both write.  This
 * used to test "EBPtr2 < GapEnd",  which misses the one address GapEnd itself:
 * with a gap exactly one byte wide (GapBeg == GapEnd) EBPtr2 was left pointing
 * AT that byte,  CMatch compared the search string against a byte that is not
 * part of the document,  and a match there left EBPtr2 == GapEnd,  from which
 * SrcLop computed a negative HowFar and handed MEMMOVE a length of (SIZE_T)-1.
 *	Running out of buffer only rules out THIS candidate of a BACKWARDS
 * search,  not the whole search:  it carries on from the next character to the
 * left,  which has more room in front of it,  not less.  So break out of the
 * match loop and let the FOREVER loop try again.  (The forward case can give
 * up outright,  since every later candidate would run out even sooner.)
 */
			    if ((EBPtr2 >= GapBeg) && (EBPtr2 <= GapEnd)) {
				EBPtr2 = GapEnd+1;	/* step over the gap */
			    }
			    if (EBPtr2 > EBfEnd) {	/* no more text to match */
				if (SIncrm == -1) {
					break;	/* this one can't match */
				}
				Matchd = FALSE;
				DBGFEX(3,DbgFNm,"SUCCESS, Matchd = FALSE");
				return SUCCESS;
			    }
			}
			if (CMatch(&SamChr) == FAILURE) {
				DBGFEX(3,DbgFNm,"FAILURE, CMatch() failed");
				return FAILURE;
			}
		} while (SamChr);
/*
 * string only partially matched,  so keep looking
 */
		if (SIncrm == 1) {		/* if forward search */
			if (++EBPtr1 > EndSAr) {
				Matchd = FALSE;
				DBGFEX(3,DbgFNm,"SUCCESS, Matchd = FALSE");
				return SUCCESS;
			}
		} else {
			if (--EBPtr1 < EndSAr) {
				Matchd = FALSE;
				DBGFEX(3,DbgFNm,"SUCCESS, Matchd = FALSE");
				return SUCCESS;
			}
		}
	} /* end of FOREVER loop */
}
