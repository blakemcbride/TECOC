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
/*
 * The test asks whether there is an OPERAND on top of the stack,  not merely
 * whether the stack is non-empty.  A "(" or a pending "+" leaves an OPERATOR
 * there,  and this command's value-producing form belongs in the middle of an
 * expression:  "1+%A" and "1+:C" used to hand GetNmA an operator and die with
 * ?IFE where "1+QA" has always worked.  ^R, ^T, ED, \ and Qq use this form.
 */
	if ((EStTop == EStBot) ||		    /* if no numeric arg or */
	    (EStack[EStTop].ElType != OPERAND)) {   /* partial expression */
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
/*
 * This arm consumed the colon but left it set,  so the NEXT command saw a
 * colon it was never given:  ":PGA" typed Q-register A instead of inserting it.
 *	MARGIS is the exception:  this arm ends in a PushEx,  so it is a VALUE
 * PRODUCER and the "m," in front of it belongs to the command that takes the
 * result.  The plain "nP" below produces nothing and goes on clearing all of
 * CmdMod.  See the commentary on FSAray in skpcmd.c.
 */
		CmdMod &= MARGIS;		/* clear modifiers flags */
		DBGFEX(1,DbgFNm,SavEof ? "PushEx(0)" : "PushEx(-1)");
		return PushEx(SavEof ? 0L : -1L, OPERAND);
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
