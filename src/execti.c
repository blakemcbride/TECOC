/*****************************************************************************
	ExeCtI()
	This function executes a ^I (control-I or caret-I) command.  This
is equivilent to the I command, except the <TAB> is inserted as well.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
DEFAULT ExeCtI()			/* execute control-I (tab) command */
{
	DBGFEN(1,"ExeCtI",NULL);
	if (FindES(ESCAPE) == FAILURE) {	/* find end of text arg */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (InsStr(ArgPtr-1, (CBfPtr-ArgPtr)+1) == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	CmdMod = '\0';				/* clear modifiers flags */
	EStTop = EStBot;			/* clear expression stack */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
