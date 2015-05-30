/*****************************************************************************
	ExeCtD()
	This function executes a ^D (control-D or caret-D) command.  This
command sets the radix to decimal.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeCtD()		/* execute a ^D (control-D) command */
{
	DBGFEN(1,"ExeCtD",NULL);
	Radix = 10;
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
