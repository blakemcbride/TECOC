/*****************************************************************************
	ExeCtO()
	This function executes a ^O (control-O or caret-O) command.  This
command sets the radix to octal.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeCtO()		/* execute a ^O (control-O) command */
{
	DBGFEN(1,"ExeCtO",NULL);
	Radix = 8;			/* set radix to octal */
	CmdMod = '\0';			/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
