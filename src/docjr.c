/*****************************************************************************
	DoCJR()
	This function contains the code that's common to the C, J and R
commands.  It is called by ExeC, ExeJ and ExeR only.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT DoCJR(HowFar)		/* do C, J or R stuff */
LONG HowFar;			/* positive or negative displacement */
{
	LONG	InRange;
	BOOLEAN	ColonMod;
#if DEBUGGING
	static char *DbgFNm = "DoCJR";
	sprintf(DbgSBf,"HowFar = %ld", HowFar);
	DbgFEn(2,DbgFNm,DbgSBf);
#endif
	ColonMod = (CmdMod & COLON);		/* is it :C, :J, or :R? */
	CmdMod &= ~COLON;			/* clear : flag */
	InRange = -1;				/* -1 means SUCCESS in TECO */
/*
 * HowFar is derived from the user's numeric argument,  so the range test is
 * done in integer arithmetic.  Adding it to a pointer first overflows the
 * pointer for a large argument,  which is undefined behaviour and can make
 * the test give the wrong answer.
 */
	if (HowFar > 0) {
		if (HowFar > (EBfEnd - GapEnd)) {
			InRange = 0;		/* 0 means FAILURE in TECO */
		} else {
			MEMMOVE(GapBeg, GapEnd+1, HowFar);
			GapBeg += HowFar;
			GapEnd += HowFar;
		}
	} else {
		if (HowFar < -(GapBeg - EBfBeg)) {
			InRange = 0;		/* 0 means FAILURE in TECO */
		} else {
			GapBeg += HowFar;
			GapEnd += HowFar;
			MEMMOVE(GapEnd+1, GapBeg, -(HowFar));
		}
	}
/*
 * ":C", ":J" and ":R" end in the PushEx below,  so they are VALUE PRODUCERS and
 * have to leave MARGIS alone:  the "m," of "m,:C" belongs to the command that
 * takes the result,  and clearing the flag here turned "0,:CT" into "-1T".
 * The plain forms produce nothing,  so an "m," in front of one has nowhere to
 * go and is dropped.  See the commentary on FSAray in skpcmd.c.
 *	ATSIGN goes with it,  and for the same reason:  this command has no
 * text argument for an "@" to modify,  so an at-sign written in front of it
 * belongs to the command that takes the result -- "@:CS/x/" searched for the
 * three characters "/x/".  See DoFlag.
 */
	CmdMod = (ColonMod) ? (CmdMod & (MARGIS | ATSIGN)) : '\0';  /* modifiers */
	if (ColonMod) {				/* if colon modifier */
		DBGFEX(2,DbgFNm,"PushEx");
		return PushEx(InRange, OPERAND);
	} else {
		if (InRange == 0) {		/* if out-of-bounds */
			ErrChr(ERR_POP, *CBfPtr);
			DBGFEX(2,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	EStTop = EStBot;			/* clear expression stack */
	DBGFEX(2,DbgFNm,"SUCCESS");
	return SUCCESS;
}
