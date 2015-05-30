/*****************************************************************************
	ExeCtV()
	This function executes a ^V (control-V or caret-V) command.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeCtV()		/* execute a ^V (control-V) command */
{
	DBGFEN(1,"ExeCtV",NULL);
	if (EStTop == EStBot) {			/* if no numeric argument */
		IniSrM = LOWER;
	} else {
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
		if (NArgmt) {			/* if it wasn't 0^V */
			ErrMsg(ERR_ARG);	/* improper arguments */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
		IniSrM = NONE;
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
