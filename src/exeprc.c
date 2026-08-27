/*****************************************************************************
	ExePrc()
	This function executes a % command.
	n%q	Add n to Q-register q and return result
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExePrc()		/* execute % (percent) command */
{
	DBGFEN(1,"ExePrc",NULL);
/*
 * The test asks whether there is an OPERAND on top of the stack,  not merely
 * whether the stack is non-empty.  A "(" or a pending "+" leaves an OPERATOR
 * there,  and this command's value-producing form belongs in the middle of an
 * expression:  "1+%A" and "1+:C" used to hand GetNmA an operator and die with
 * ?IFE where "1+QA" has always worked.  ^R, ^T, ED, \ and Qq use this form.
 *	UMinus has to run BEFORE the test:  a bare "-" is on the stack as an
 * OPERATOR,  and it is UMinus that turns the lone one into the operand -1.
 * Testing first made "-C", "-R", "-D", "-A", "-%q" and "-S" all look like
 * partial expressions and take their default of 1.
 */
	UMinus();				/* if it's -%, make it -1% */
	if ((EStTop == EStBot) ||		    /* if no numeric arg or */
	    (EStack[EStTop].ElType != OPERAND)) {   /* partial expression */
		NArgmt = 1;			/* default is 1%q */
	} else {
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE, GetNmA() failed");
			return FAILURE;
		}
	}
	if (IncCBP() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, IncCBP() failed");
		return FAILURE;
	}
	if (FindQR() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, FindQR() failed");
		return FAILURE;
	}
	QR->Number += NArgmt;
/*
 * % is a VALUE PRODUCER -- it ends in the PushEx below -- so it must leave
 * MARGIS alone.  The "n" of "n%q" is %'s own argument;  the "m," of "m,n%q"
 * belongs to the command that takes the result,  so clearing the flag here
 * turned "0,2%9T" into "<result>T" and typed the whole buffer.  See the
 * commentary on FSAray in skpcmd.c.
 *	ATSIGN goes with it,  and for the same reason:  this command has no
 * text argument for an "@" to modify,  so an at-sign written in front of it
 * belongs to the command that takes the result -- "@0%aS/x/" searched for the
 * three characters "/x/".  See DoFlag.
 */
	CmdMod &= (MARGIS | ATSIGN);		/* clear modifiers flags */
#if DEBUGGING
	sprintf(DbgSBf,"PushEx(%ld,OPERAND)",QR->Number);
	DbgFEx(1,DbgFNm,DbgSBf);
#endif
	return PushEx(QR->Number, OPERAND);
}
