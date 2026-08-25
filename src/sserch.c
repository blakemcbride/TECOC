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
			if (++SStPtr == SBfPtr) { /* if no more to check */
				RefLen = (EBPtr1 - EBPtr2) - 1;
				if ((SIncrm == -1) && (EBPtr2 >= GapEnd)) {
					RefLen += GapEnd-GapBeg+1;
				}
				Matchd = TRUE;
				DBGFEX(3,DbgFNm,"SUCCESS, Matchd = TRUE");
				return SUCCESS;
			}
			if (++EBPtr2 > RhtSid) { /* inc and check for eob */
			    if (SIncrm == -1) {
/*
 * For a backwards search RhtSid is the last character BEFORE the edit buffer
 * gap,  so passing it just means EBPtr2 has walked into the gap and has to be
 * stepped over it.  RhtSid says nothing about the end of the edit buffer in
 * that case,  so the end has to be tested separately.  Without this test the
 * match walked off the end of the edit buffer and could "succeed" against
 * memory outside it,  after which SrcLop moved the gap to a position beyond
 * Z and wrecked the buffer.
 *	Running out of buffer only rules out THIS candidate,  not the whole
 * search:  a backwards search carries on from the next character to the left,
 * which has more room in front of it,  not less.  So break out of the match
 * loop and let the FOREVER loop try again.  (The forward case can give up
 * outright,  since every later candidate would run out even sooner.)
 */
				if (EBPtr2 < GapEnd) {
					EBPtr2 = GapEnd+1;
				}
				if (EBPtr2 > EBfEnd) {
					break;	/* this one can't match */
				}
			    } else {
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
