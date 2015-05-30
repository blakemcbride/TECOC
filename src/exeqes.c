/*****************************************************************************
	ExeQes()
	This function executes a ? command.
	?	Toggle trace mode
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeQes()                /* question mark command */
{
	DBGFEN(1,"ExeQes",NULL);
	TraceM = !TraceM;
	CmdMod = '\0';				/* clear modifiers flags */
	EStTop = EStBot;			/* clear expression stack */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
