/*****************************************************************************
	ExeCtL()
	This function executes a ^L (control-L or caret-L) command.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
DEFAULT ExeCtL()		/* execute an ^L (control-L) command */
{
	ZDspCh(FORMFD);
/*
 * ^L takes no arguments and no modifiers,  so it has to discard whatever was
 * in front of it,  the way ExeCtA, ExeExc and the rest of the no-argument
 * commands do.  Without this a ":^L" left the colon set for the next command,
 * which then saw a colon it was never given.
 */
	CmdMod = '\0';				/* clear modifiers flags */
	EStTop = EStBot;			/* clear expression stack */
	return SUCCESS;
}
