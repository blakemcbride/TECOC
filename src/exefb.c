/*****************************************************************************
	ExeFB()
	This function executes an FB command.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeFB()			/* execute an FB command */
{
	DBGFEN(1,"ExeFB",NULL);
	if (CmdMod & DCOLON) {			/* if it's ::FB */
		ErrStr(ERR_ILL, "::FB");	/* illegal command "::FB" */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	SrcTyp = FB_SEARCH;
	if (Search(FALSE) == FAILURE) {		/* FALSE: not two text args */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
