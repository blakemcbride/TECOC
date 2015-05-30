/*****************************************************************************
	ExeCln()
	This function handles the colon and double-colon modifiers.
	This function implements the modifiers by setting bits in the
CmdMod variable.  Commands which are sensitive to colon or double colon
modification check CmdMod explicitly.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeCln()		/* execute : or :: modifiers */
{
	DBGFEN(1,"ExeCln",NULL);
	if (CmdMod & COLON) {			/* if colon bit is set */
		CmdMod &= ~COLON;		/* clear colon bit */
		CmdMod |= DCOLON;		/* set double-colon bit */
	} else {
		CmdMod |= COLON;		/* set colon bit */
	}
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
