/*****************************************************************************
	ExeCtW()
	This function executes a control-W command.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeCtW()		/* execute a ^W (control-W) command */
{
	DBGFEN(1,"ExeCtW",NULL);
	if (EStTop == EStBot) {			/* if no numeric argument */
		IniSrM = UPPER;
	} else {
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
		if (NArgmt) {			/* if it wasn't 0^W */
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
