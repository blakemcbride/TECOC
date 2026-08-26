/*****************************************************************************
	ExeD()
	This function executes a D command.
	nD	Delete n characters.
	m,nD	Delete between m and n
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeD()			/* execute a D command */
{
	LONG	Status;
	DBGFEN(1,"ExeD",NULL);
	if (CmdMod & MARGIS) {			/* if it's m,nD */
		DBGFEX(1,DbgFNm,"ExeK()");
		return ExeK();
	}
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
	UMinus();				/* if it's -D, make it -1D */
	if ((EStTop == EStBot) ||		    /* if no numeric arg or */
	    (EStack[EStTop].ElType != OPERAND)) {   /* partial expression */
		NArgmt = 1;			/* default is 1D */
	} else {
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	Status = -1;					/* -1 means success */
/*
 * Do the range tests in integer arithmetic:  adding the user's argument to a
 * pointer first overflows the pointer for a large argument.
 */
	if (NArgmt > 0) {
		if (NArgmt > (EBfEnd - GapEnd)) {	/* if out of range */
			Status = 0;			/* 0 means failure */
		} else {
			GapEnd += NArgmt;		/* delete */
		}
	} else {
		if (NArgmt < -(GapBeg - EBfBeg)) {	/* if out of range */
			Status = 0;			/* 0 means failure */
		} else {
			GapBeg += NArgmt;		/* delete */
		}
	}
	if (CmdMod & COLON) {			/* if it's :D */
/*
 * ":D" ends in the PushEx below,  so it is a VALUE PRODUCER and has to leave
 * MARGIS alone -- the "m," in front of it belongs to the command that takes the
 * result.  The plain "nD" below produces nothing,  so an "m," left in front of
 * it has nowhere to go and is dropped.  See the commentary on FSAray in
 * skpcmd.c.
 */
		CmdMod &= MARGIS;		/* clear modifiers flags */
		DBGFEX(1,DbgFNm,"PushEx()");
		return PushEx(Status, OPERAND);
	}
	if (Status == 0) {
		ErrMsg(ERR_DTB);		/* DTB = "delete too big" */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
