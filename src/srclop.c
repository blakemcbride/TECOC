/*****************************************************************************

    SrcLop()

*****************************************************************************/

#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */

/*****************************************************************************

    DoFBFC()

	This function is called only by the SrcLop function,  after the
Search function has set up the SBfBeg, SBfEnd, SBfPtr and NArgmt variables.
It is called to handle the special processing needed by the FB and FC
search commands.  The FB and FC commands provide TECO's bounded search
capability.

*****************************************************************************/

#if USE_PROTOTYPES
static DEFAULT DoFBFC(void);
#endif

static DEFAULT DoFBFC()		/* search loop */
{
    ptrdiff_t HowFar;
    ptrdiff_t Shuffl;		/* how far buffer gap is shuffled */

#if DEBUGGING
    static char *DbgFNm = "DoFBFC";
    sprintf(DbgSBf,"MArgmt = %ld, NArgmt = %ld", MArgmt, NArgmt);
    DbgFEn(2,DbgFNm,DbgSBf);
#endif

    Shuffl = 0;
    if ((CmdMod & MARGIS) && (MArgmt == NArgmt)) {

/*
 * An ANCHORED search.  The manual:  "A special case of the bounded search
 * occurs when the upper and lower bound limits are the same.  In such a case,
 * the search command is called an anchored search,  and is used to compare the
 * search argument against the character string immediately following the text
 * buffer pointer",  and it defines ::Stext` as "identical to .,.:FBtext`".
 *	It has to be taken before the general path below,  which can never
 * match when m == n:  GetAra reads m,n as the half-open range [m,n) and so
 * hands back AraEnd == AraBeg-1,  the direction test picks BACKWARDS because
 * m is not less than n,  and SSerch's first bound test (EBPtr1 < EndSAr) fires
 * before it looks at a single character.  Worse,  m == n == Z made GetAra
 * itself report ?POP,  because it computes the address of character n-1.
 *	Only a match that starts exactly at m counts,  so EBPtr1 and EndSAr
 * are both the address of m;  the matched string may run past m,  which is
 * what RhtSid allows.  (TECO-C's own ::S builds .,.+1:FB to dodge all this;
 * that still works,  since [.,.+1) is the single position dot.)
 */
	if (MArgmt < 0) {			/* if negative m */
	    ErrMsg(ERR_NCA);			/* negative argument to , */
	    DBGFEX(2,DbgFNm,"FAILURE, negative m");
	    return FAILURE;
	}
	if (MArgmt > ((GapBeg-EBfBeg) + (EBfEnd-GapEnd))) {	/* if past Z */
	    ErrStr(ERR_POP, ErrTxt);		/* POP = pointer off page */
	    DBGFEX(2,DbgFNm,"FAILURE, m too large");
	    return FAILURE;
	}
	AraBeg = EBfBeg + MArgmt;		/* compute area beginning */
	if (AraBeg > GapBeg-1) {		/* if past start of gap */
	    AraBeg += (GapEnd-GapBeg) + 1;	/* correct for gap */
	}
	AraEnd = AraBeg;
	if (AraBeg > EBfEnd) {			/* if m is the end of the buffer */
	    Matchd = FALSE;			/* there is nothing to compare */
	    DBGFEX(2,DbgFNm,"SUCCESS, Matchd = FALSE");
	    return SUCCESS;
	}
	SIncrm = 1;				/* compare forwards */
	EBPtr1 = EndSAr = AraBeg;		/* only m may start a match */
	RhtSid = (AraBeg < GapBeg) ? GapBeg - 1 : EBfEnd;

    } else if (CmdMod & MARGIS) {		/* if it's m,nFB or m,nFC */
/*
 * Call GetAra to convert m and n into addresses.  A side effect of the call
 * to GetAra is that MArgmt and NArgmt are sorted so that MArgmt is less than
 * NArgmt.  We need to know the original order,  so set an indicator variable
 * before calling GetAra.
 */

	SIncrm = (MArgmt < NArgmt) ? 1 : -1;
	if (GetAra() == FAILURE) {
	    DBGFEX(2,DbgFNm,"FAILURE, GetAra() failed");
	    return FAILURE;
	}

/*
 * If the edit buffer gap falls between m and n,  we need to shuffle part of
 * the edit buffer to make the m,n area contiguous.  The gap is shuffled so
 * that it is before the m,n area,  so that if part of the match string lies
 * beyond the n boundary,  it can still be matched.
 */

/*
 * The gap has to end up immediately in front of the m,n area,  so the distance
 * to move it is the number of area characters that lie BEFORE the gap,  which is
 * GapBeg-AraBeg.  This used to use AraEnd-GapEnd -- the number that lie AFTER it
 * -- which is a different number unless the area happens to be symmetric about
 * dot.  Too large and GapBeg was driven below EBfBeg (heap underflow) and the
 * search started before m;  too small and the search started after m and missed
 * matches inside the requested range.
 */
	if ((AraBeg < GapBeg) && (AraEnd > GapEnd)) {
	    Shuffl = GapBeg - AraBeg;
	    GapBeg -= Shuffl;
	    GapEnd -= Shuffl;
	    MEMMOVE(GapEnd+1, GapBeg, (SIZE_T)Shuffl);
	}

/*
 * set the variables for the call to SSerch
 */

	if (SIncrm == 1) {		/* forward search */
	    EBPtr1 = (Shuffl!=0 ? GapEnd + 1 : AraBeg);
	    EndSAr = AraEnd;
	} else {			/* else backward search */
	    EBPtr1 = AraEnd;
	    EndSAr = (Shuffl!=0 ? GapEnd + 1 : AraBeg);
	}
/*
 * EndSAr is the last place a match may START;  RhtSid is the last byte that may
 * be EXAMINED,  which is how a match is allowed to run past the n boundary.
 * "RhtSid = EBfEnd" is right when the gap has been shuffled out of the way,
 * and right when the m,n area lies after the gap.  It was wrong in the
 * remaining case -- the area lies entirely BEFORE the gap -- because then a
 * partial match that reached AraEnd carried straight on into the gap,  which
 * is not text at all but whatever the last gap movement left there.  The match
 * could "succeed" against that stale text,  and the code below then computed a
 * NEGATIVE HowFar and handed MEMMOVE a length of (SIZE_T)-1.
 *	Stopping at GapBeg-1 is what the non-FB paths do (see the backwards
 * branch of SrcLop below);  SSerch notices that bound and steps EBPtr2 over
 * the gap,  so the match still carries on into the text beyond it.
 */
	RhtSid = (AraEnd < GapBeg) ? GapBeg - 1 : EBfEnd;
    } else {				/* else it's nFB or nFC */
	if (NArgmt > 0) {		/* forward search */
	    SIncrm = 1;
	    EBPtr1 = GapEnd + 1;
	    EndSAr = GapEnd + Ln2Chr(NArgmt);
	    RhtSid = EBfEnd;
	} else {			/* else backward search */
	    SIncrm = -1;
	    EBPtr1 = GapBeg - 1;
	    EndSAr = GapBeg + Ln2Chr(NArgmt);
	    RhtSid = GapBeg - 1;
	}
    }


    if (SSerch() == FAILURE) {
	DBGFEX(2,DbgFNm,"FAILURE, SSerch() failed");
	return FAILURE;
    }

/*
 * If the search succeeded,  position the gap to the end of the found string.
 */

    if (Matchd) {				/* if search succeeded */
	if (EBPtr2 >= GapEnd) {			/* if match after gap */
	    HowFar = EBPtr2 - GapEnd;
	    MEMMOVE(GapBeg, GapEnd+1, (SIZE_T)HowFar);
	    GapBeg += HowFar;
	    GapEnd += HowFar;
	} else {
	    HowFar = (GapBeg-1) - EBPtr2;
	    GapBeg -= HowFar;
	    GapEnd -= HowFar;
	    MEMMOVE(GapEnd+1, GapBeg, (SIZE_T)HowFar);
	}
    } else {					/* else search failed */

/*
 * If the edit buffer gap was shuffled earlier (because the gap fell into the
 * m,n area),  then we need to reverse shuffle it to leave things as they
 * were.
 */

	if (Shuffl != 0) {		/* if buffer was shuffled */
	    MEMMOVE(GapBeg, GapEnd+1, (SIZE_T)Shuffl);
	    GapBeg += Shuffl;
	    GapEnd += Shuffl;
	}
    }
#if DEBUGGING
    sprintf(DbgSBf,"SUCCESS, Matchd = %s", (Matchd) ? "TRUE" : "FALSE");
    DbgFEx(2,DbgFNm,DbgSBf);
#endif
    return SUCCESS;
}



/*****************************************************************************

    SrcLop()

	This function is called by the Search function only,  after the
Search function has set up the search string buffer (SBfBeg, SBfEnd, SBfPtr)
and the iteration count (NArgmt).  It sets up the edit buffer bounds for the
search (EBPtr1, EndSAr) and direction indicator (SIncrm) and then calls
the SSerch function.  One way to describe this function is that it handles
the numeric arguments of all search commands by handling m,n arguments or
actually doing the loop for n arguments.

*****************************************************************************/

DEFAULT SrcLop()		/* search loop */
{
    ptrdiff_t	HowFar;

#if DEBUGGING
    static char *DbgFNm = "SrcLop";
    sprintf(DbgSBf,"search string SBf = \"%.*s\"",
		(int)(SBfPtr-SBfBeg), SBfBeg);
    DbgFEn(2,DbgFNm,DbgSBf);
#endif

    if (SrcTyp == FB_SEARCH) {		/* if it's FB or FC */
	DBGFEX(2,DbgFNm,"returning DoFBFC()");
	return DoFBFC();
    }

/*
 * It's not an FB or FC search.  The FB and FC commands are the only ones
 * which can be preceded by an m,n argument pair.  Other search commands
 * can be preceded only by an iteration count.
 */

    if (NArgmt > 0) {			/* if forwards search */
	SIncrm = 1;
	EBPtr1 = GapEnd;
	EndSAr = RhtSid = EBfEnd;
	do {
	    EBPtr1++;
	    if (SSerch() == FAILURE) {
		DBGFEX(2,DbgFNm,"FAILURE, SSerch() failed");
		return FAILURE;
	    }

	    if (Matchd) {			/* if search succeeded */
		--NArgmt;
		if ((EdFlag & ED_DOT_BY_ONE) == 0)
		    EBPtr1 = EBPtr2;		/* skip found string */
	    } else {				/* else search failed */
		switch (SrcTyp) {
		    case S_SEARCH:
		    case FK_SEARCH:
			NArgmt = 0;		/* exit loop */
			break;

		    case N_SEARCH:
			if (IsEofI[CurInp]) {
			    if (WrPage(CurOut,EBfBeg,
					EBfEnd,FFPage) == FAILURE) {
				DBGFEX(2,DbgFNm,"FAILURE, WrPage() failed");
				return FAILURE;
			    }
			    GapBeg = EBfBeg;	/* clear the.. */
			    GapEnd = EBfEnd;	/* ..edit buffer */
			    NArgmt = 0;		/* exit loop */
			} else {		/* else not end-of-file */
			    if (SinglP() == FAILURE) {
				DBGFEX(2,DbgFNm,"FAILURE, SinglP() failed");
				return FAILURE;
			    }
/*
 * GapEnd,  not GapEnd+1.  The loop advances the candidate position with the
 * EBPtr1++ at the TOP of its body,  which is why the set-up above it says
 * "EBPtr1 = GapEnd" too.  Re-arming with GapEnd+1 after a page turn made the
 * first character SSerch saw GapEnd+2,  so a match beginning at the very first
 * character of any page after the first could never be found.
 */
			    EBPtr1 = GapEnd;
			    EndSAr = RhtSid = EBfEnd;
			}
			break;
		    case U_SEARCH:
			if (((EdFlag & ED_YANK_OK) == 0) &&
			    (IsOpnO[CurOut]) &&
			    ((GapBeg != EBfBeg) || (GapEnd != EBfEnd))) {
				ErrMsg(ERR_YCA);
				DBGFEX(2,DbgFNm,"FAILURE");
				return FAILURE;
			}
			/* FALL THROUGH */
		    case E_SEARCH:
			GapBeg = EBfBeg;		/* clear the... */
			GapEnd = EBfEnd;		/* ...edit buffer */
			if (IsEofI[CurInp]) {		/* if end-of-file */
			    NArgmt = 0;			/* exit loop */
			} else {
			   if (RdPage() == FAILURE) {
				DBGFEX(2,DbgFNm,"FAILURE");
				return FAILURE;
			    }
			    EBPtr1 = GapEnd;	/* see the N_SEARCH comment above */
			    EndSAr = RhtSid = EBfEnd;
			}
		}				/* end of switch */
	    }
	} while (NArgmt > 0);
    } else {					/* else backwards search */
	SIncrm = -1;
	EBPtr1 = RhtSid = GapBeg;		/* start point */
	RhtSid--;
	EndSAr = EBfBeg;			/* end point */
	do {
	    EBPtr1--;
	    if (SSerch() == FAILURE) {
		DBGFEX(2,DbgFNm,"FAILURE, SSerch() failed");
		return FAILURE;
	    }

	    if (Matchd) {
		++NArgmt;
	    } else {
		if ((SrcTyp==S_SEARCH) || (SrcTyp==FK_SEARCH)) {
		    break;
		} else {
		    DBGFEX(2,DbgFNm,"ExeNYI()");
		    return ExeNYI();
		}
	    }
	} while (NArgmt < 0);
    }

/* The next block of code deals with what happens to the edit buffer after
 * the search.  There are three possibilities:
 *
 *	if the search was successful
 *		if it's an FK command
 *			delete spanned characters
 *		else
 *			move to after the found string
 *	else
 *		move to the start of the edit buffer
 *
 */

    if (Matchd) {				/* if search succeeded */
	if (SrcTyp == FK_SEARCH) {		/* if FK command */
	    if (SIncrm == 1) {			/* if forward search */
		GapEnd = EBPtr2;		/* delete */
	    } else {				/* else backward */
		GapBeg = EBPtr1;		/* delete */
		if (EBPtr2 > GapEnd)
		    GapEnd = EBPtr2;
	    }
	} else {				/* else not FK */
	    if ((SIncrm == 1) || (EBPtr2 > GapEnd)) {
		HowFar = EBPtr2 - GapEnd;
		MEMMOVE(GapBeg, GapEnd+1, (SIZE_T)HowFar);
		GapBeg += HowFar;
		GapEnd += HowFar;
	    } else {
		HowFar = (GapBeg-1) - EBPtr2;
		GapBeg -= HowFar;
		GapEnd -= HowFar;
		MEMMOVE(GapEnd+1, GapBeg, (SIZE_T)HowFar);
	    }
	}
    } else {					/* else search failed */
	if ((EdFlag & ED_PRES_DOT) == 0) {	/* if don't preserve dot */
	    HowFar = GapBeg - EBfBeg;
	    GapBeg -= HowFar;
	    GapEnd -= HowFar;
	    MEMMOVE(GapEnd+1, GapBeg, (SIZE_T)HowFar);
	}
    }

#if DEBUGGING
    sprintf(DbgSBf,"SUCCESS, Matchd = %s", (Matchd) ? "TRUE" : "FALSE");
    DbgFEx(2,DbgFNm,DbgSBf);
#endif
    return SUCCESS;
}
