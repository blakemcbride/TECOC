/*****************************************************************************
	ExeJ()
	This function executes a J command.
	nJ	Move pointer to "n"
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeJ()			/* execute J command */
{
	DBGFEN(1,"ExeJ",NULL);
/*
 * The test asks whether there is an OPERAND on top of the stack,  not merely
 * whether the stack is non-empty.  A "(" or a pending "+" leaves an OPERATOR
 * there,  and this command's value-producing form belongs in the middle of an
 * expression:  "1+%A" and "1+:C" used to hand GetNmA an operator and die with
 * ?IFE where "1+QA" has always worked.  ^R, ^T, ED, \ and Qq use this form.
 */
	if ((EStTop == EStBot) ||		    /* if no numeric arg or */
	    (EStack[EStTop].ElType != OPERAND)) {   /* partial expression */
		NArgmt = 0;			/* default is 0J */
	} else {
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	DBGFEX(1,DbgFNm,"DoCJR()");
/*
 * NArgmt is a user-supplied LONG,  so "EBfBeg-GapBeg+NArgmt" -- dot subtracted
 * from it -- overflows for an extreme value,  which is undefined behaviour and
 * hands DoCJR a displacement that has already wrapped.  Compare against Z first
 * and,  when the position is out of range,  pass a displacement that is just
 * outside the buffer so DoCJR reports it the way it always did.
 */
	{
		LONG Dot = GapBeg - EBfBeg;
		LONG Zed = Dot + (EBfEnd - GapEnd);
		if (NArgmt < 0) {
			return DoCJR(-(Dot + 1));
		}
		if (NArgmt > Zed) {
			return DoCJR((Zed - Dot) + 1);
		}
		return DoCJR(NArgmt - Dot);
	}
}
