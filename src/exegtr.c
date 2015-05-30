/*****************************************************************************
	ExeGtr()
	This function executes a greater-than-sign (>) command.
	>	End iteration
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeGtr()				/* execute a > command */
{
	DBGFEN(1,"ExeGtr",NULL);
	if (LStTop == LStBot) {			/* if not in loop */
		ErrMsg(ERR_BNI);		/* BNI = > not in iteration */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (LStack[LStTop].LIndex != INFINITE) {/* if not infinite loop */
		--LStack[LStTop].LIndex;	/* decrement loop counter */
	}
	if ((LStack[LStTop].LIndex == INFINITE) ||	/* if infinite loop */
	    (LStack[LStTop].LIndex > 0)) {	/* or more iterations to do */
		CBfPtr = LStack[LStTop].LAddr;	/* jump to front of loop */
	} else {
		--LStTop;			/* leave the loop */
	}
	CmdMod = '\0';				/* clear modifiers flags */
	EStTop = EStBot;			/* clear expression stack */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
