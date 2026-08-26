/*****************************************************************************
	ExeY()
	This function executes a Y command.
	Y	Read into buffer
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeY()                          /* Y command */
{
	DBGFEN(1,"ExeY",NULL);
/*
 * The test asks whether there is an OPERAND on top of the stack,  not merely
 * whether the stack is non-empty.  A "(" or a pending "+" leaves an OPERATOR
 * there,  and this command's value-producing form belongs in the middle of an
 * expression:  "1+%A" and "1+:C" used to hand GetNmA an operator and die with
 * ?IFE where "1+QA" has always worked.  ^R, ^T, ED, \ and Qq use this form.
 */
	if ((EStTop > EStBot) &&		/* if numeric argument */
	    (EStack[EStTop].ElType == OPERAND)) { /* ...and not a partial one */
		ErrMsg(ERR_NYA);		/* numeric argument with Y */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if ((EdFlag & ED_YANK_OK) == 0) {	/* if no yank when full EBf */
		if (IsOpnO[CurOut] &&		/* if output file is open */
		    ((GapBeg != EBfBeg) || (GapEnd != EBfEnd))) {
			ErrMsg(ERR_YCA);	/* Y command aborted */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	DBGFEX(1,DbgFNm,"ExeEY()");
	return ExeEY();
}
