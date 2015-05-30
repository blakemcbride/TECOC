/*****************************************************************************
	ExeCom()
	This function executes a , (comma argument separator) command.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeCom()		/* execute a , (comma) command */
{
	DBGFEN(1,"ExeCom",NULL);
	if (EStTop == EStBot) {			/* if no numeric argument */
		ErrMsg(ERR_NAC);		/* no arg before , */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (GetNmA() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	MArgmt = NArgmt;
	CmdMod |= MARGIS;
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
