/*****************************************************************************
	ExeP()
	This function executes a P or PW command.
	nP	Advance n pages
	m,nP	Write out chars m to n
	nPW	Write buffer n times
	m,nPW	Write out chars m to n
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeP()			/* execute a P or PW command */
{
	BOOLEAN	SavEof;
	DBGFEN(1,"ExeP",NULL);
	SavEof = IsEofI[CurInp];		/* save end-of-file flag */
	if (EStTop == EStBot) {			/* if no numeric argument */
		NArgmt = 1;			/* default is 1P */
	} else {
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
		if ((NArgmt <= 0) && ((CmdMod & MARGIS) == 0)) {
			ErrMsg(ERR_IPA);	/* negative or 0 arg to P */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
/*
 * If it's a "PW" or "m,nP" command,  let ExePW handle it.
 */
	if ((CBfPtr != CStEnd) &&
	    ((*(CBfPtr+1) == 'W') || (*(CBfPtr+1) == 'w'))) {
		++CBfPtr;			/* skip the "W" */
		DBGFEX(1,DbgFNm,"ExePW");
		return ExePW();			/* execute a PW command */
	}
	if (CmdMod & MARGIS) {			/* if it's m,nP */
		DBGFEX(1,DbgFNm,"ExePW");
		return ExePW();			/* execute a PW command */
	}
	do {
		if (SinglP() == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	} while ((--NArgmt > 0) && !IsEofI[CurInp]);
	if (CmdMod & COLON) {			/* if it's :P */
		DBGFEX(1,DbgFNm,SavEof ? "PushEx(0)" : "PushEx(-1)");
		return PushEx(SavEof ? 0L : -1L, OPERAND);
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
