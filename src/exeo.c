/*****************************************************************************
	ExeO()
	This function executes an O command.
	Otag		Goto label
	nOtag0,tag1	Computed goto
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeO()			/* execute an O command */
{
    BOOLEAN Found;		/* has the tag been found? */
    unsigned char TBfBeg[TBFINIT]; /* tag buffer */
    charptr TBfEnd;		/* end of tag buffer */
    charptr TBfPtr;		/* end of tag in tag buffer */
    charptr TagPtr;		/* pointer into tag buffer */
    charptr TmpPtr;		/* temp pointer for shuffling in Tbf */
    DBGFEN(1,"ExeO",NULL);
    /*
     * ??? Pete,
     * given "O^EQq$" we simply use BldStr to build the string
     * before looking for the tag in q-reg q.  What do we do if
     * given "nOtag0,^EQq,tag2$", especially if q-reg q contains a
     * comma?  if we BldStr first, the comma in the q-reg will
     * change the number of tags, maybe changing the nth tag.
     * If we BldStr after finding the tag we want, we don't take
     * into account the list of tags which may be in q-reg q.
     *
     * After implementing it the second way (search/Bldstr), I find
     * it is much easier to do it the first way (BldStr/search).  One
     * disadvantage to doing it this way is BldStr is probably building
     * much more than it needs to, potentially overrunning TBf?
     */
    TBfEnd = TBfBeg + TBFINIT;
    if (BldStr(TBfBeg, TBfEnd, &TBfPtr) == FAILURE) {
	DBGFEX(1,DbgFNm,"FAILURE, BldStr() failed");
	return FAILURE;
    }
/*
 * If there's something on the expression stack, then we have a computed
 * goto, so figure out which tag in TBf they want.
 *
 *		TBf:	"TAG0,TAG1,TAG2........"
 *			 ^             ^       ^
 *			 TBfBeg        TBfPtr  TBfEnd
 */
    if (EStTop > EStBot) {
	if (GetNmA () == FAILURE) {	/* get NArgmt */
	    DBGFEX(1,DbgFNm,"FAILURE, GetNmA() failed");
	    return FAILURE;
	}
	if (NArgmt < 0) {		/* n is out of range */
	    TagPtr = TBfPtr;		/* say we ran into end of tag list */
	} else {			/* find the nth tag */
	    TagPtr = TBfBeg;
	    if (NArgmt > 0) {		/* n==0 is already done */
		while (TagPtr < TBfPtr) {
		    if (*TagPtr++ == ',') {
			if (--NArgmt == 0) {
			    break;
			}
		    }
		}
	    }
	}
/*
 * If we ran into the end of the tag list, or the tag we found is null
 * (",,"), then flow past nO command.
 */
	if (TagPtr == TBfPtr || *TagPtr == ',') {
	    DBGFEX(1,DbgFNm,"SUCCESS");
	    CmdMod = '\0';			/* clear modifiers flags */
	    return SUCCESS;
	}
/*
 * shuffle tag we found to beginning of TBf for searching.
 */
	TmpPtr = TBfBeg;
	while (TagPtr < TBfPtr && *TagPtr != ',') {
	    *TmpPtr++ = *TagPtr++;
	}
	TBfPtr = TmpPtr;
    }
#if DEBUGGING
    *TBfPtr = '\0';
    sprintf(DbgSBf,"searching for tag \"%s\"", TBfBeg);
    DbgFMs(1,DbgFNm,DbgSBf);
#endif
/*
 * reset CBfPtr to point to the front of the command string,  or the front
 * of the loop if we're in a loop, then search for the tag in TBf, keeping
 * track of nested <> loops.
 */
    CBfPtr = (LStTop == LStBot) ? CStBeg : LStack[LStTop].LAddr + 1;
    Found = FALSE;
    CmdMod = '\0';				/* clear modifiers flags */
    while (!Found) {
	while (*CBfPtr != '!') {
	    if (CBfPtr == CStEnd) {
		ErrPSt(ERR_TAG, TBfBeg, TBfPtr);
		DBGFEX(1,DbgFNm,"FAILURE, missing tag");
		return FAILURE;
	    }
	    switch (*CBfPtr) {
	    case '<':
		if (++LStTop >= LPS_SIZE) {
		    ErrMsg(ERR_PDO);
		    DBGFEX(1,DbgFNm,"FAILURE");
		    return FAILURE;
		}
		LStack[LStTop].LIndex = INFINITE;
		LStack[LStTop].LAddr = CBfPtr;
		CmdMod = '\0';
		break;
	    case '>':
		if (LStTop == LStBot) {
		    ErrMsg(ERR_BNI);
		    DBGFEX(1,DbgFNm,"FAILURE");
		    return FAILURE;
		}
		--LStTop;
		CmdMod = '\0';
		break;
	    default:
		if (SkpCmd() == FAILURE) {
		    DBGFEX(1,DbgFNm,"FAILURE, SkpCmd() failed");
		    return FAILURE;
		}
	    } /* end of switch */
	    ++CBfPtr;
	} /* end of while (!='!')*/
/*
 * At this point, we've found an '!'.  if tracing is on,  display exclamation
 * point
 */
	if (TraceM) {				/* if trace mode is on */
	    if (CmdMod & ATSIGN) {		/* if it was @O */
		ZDspCh('@');
	    }
	    ZDspCh('!');
	}
	if (FindES('!') == FAILURE) {	/* find end of tag */
	    DBGFEX(1,DbgFNm,"FAILURE, FindES() failed");
	    return FAILURE;
	}
#if DEBUGGING
	sprintf(DbgSBf,"found tag \"%.*s\"\n",(int)(CBfPtr-ArgPtr), ArgPtr);
	DbgFMs(1,DbgFNm,DbgSBf);
#endif
/*
 * compare found tag (in ArgPtr) to O command tag (in TBf).
 */
	TagPtr = TBfBeg;
	while ((TagPtr < TBfPtr) && (*ArgPtr == *TagPtr)) {
	    ++ArgPtr;
	    ++TagPtr;
	}
	Found = ((TagPtr == TBfPtr) && (*ArgPtr == '!'));
	++CBfPtr;
    } /* end of while (!Found) */
    --CBfPtr;
    CmdMod = '\0';				/* clear modifiers flags */
    DBGFEX(1,DbgFNm,"SUCCESS");
    return SUCCESS;
}
