/*****************************************************************************
	ExeBar()
	This function executes a | (vertical bar) command.
	|	Start ELSE part of conditional
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeBar()		/* execute | (vertical bar) command */
{
	DBGFEN(1,"ExeBar",NULL);
	if (FlowEC() == FAILURE) {		/* flow to ' */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	CmdMod = '\0';				/* clear modifiers flags */
	EStTop = EStBot;			/* clear expression stack */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
