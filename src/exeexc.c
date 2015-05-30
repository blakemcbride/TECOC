/*****************************************************************************
	ExeExc()
	This function executes the ! (exclamation point) command.
	!	Define label
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeExc()		/* execute a ! (exclamation mark) command */
{
	DBGFEN(1,"ExeExc",NULL);
	if (FindES('!') == FAILURE) {		/* simply skip to next '!' */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	CmdMod = '\0';				/* clear modifiers flags */
	EStTop = EStBot;			/* clear expression stack */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
