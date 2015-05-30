/*****************************************************************************
	ExeCtA()
	This function executes a ^A (control-A or caret-A) command.
	^Atext^A	Types "text" on the terminal.  The closing
			character must be a control-A.
	@^A/text/	Equivilent to ^A except text can be bracketed
			with any character, avoids need for second ^A.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
DEFAULT ExeCtA()		/* execute a ^A (control-A) command */
{
	DBGFEN(1,"ExeCtA",NULL);
	if (FindES(CTRL_A) == FAILURE) {	/* find end of string */
		DBGFEX(1,DbgFNm,"FAILURE, FindES() failed");
		return FAILURE;
	}
	TypBuf(ArgPtr, CBfPtr);
	CmdMod = '\0';				/* clear modifiers flags */
	EStTop = EStBot;			/* clear expression stack */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
