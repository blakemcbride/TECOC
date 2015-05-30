/*****************************************************************************
	ExeCtC()
	This function executes a control-C command.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
DEFAULT ExeCtC()		/* execute an control-C command */
{
	DBGFEN(1,"ExeCtC",NULL);
	if ((CBfPtr < CStEnd) && (*(CBfPtr+1) == CTRL_C)) { /* if it's ^C^C */
		TAbort(EXIT_SUCCESS);
	}
	CmdMod = '\0';			/* clear modifiers flags */
	EStTop = EStBot;		/* clear expression stack */
	DBGFEX(1,DbgFNm,"FAILURE");
	return FAILURE;
}
