/*****************************************************************************
	ExeC()
	This function executes a C command.
	nC	Advance n characters
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeC()			/* execute a C command */
{
	DBGFEN(1,"ExeC",NULL);
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
	UMinus();				/* if it's -C, make it -1C */
	if ((EStTop == EStBot) ||		    /* if no numeric arg or */
	    (EStack[EStTop].ElType != OPERAND)) {   /* partial expression */
		NArgmt = 1;			/* default is 1C */
	} else {
		if (GetNmA() == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	DBGFEX(1,DbgFNm,"DoCJR");
	return DoCJR(NArgmt);
}
