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
	CmdMod = '\0';				/* clear modifiers flags */
	EStTop = EStBot;			/* clear expression stack */
	DBGFEX(2,DbgFNm,"SUCCESS");
	return SUCCESS;
}
