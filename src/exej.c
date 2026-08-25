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
	if (EStTop == EStBot) {			/* if no numeric argument */
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
