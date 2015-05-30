/*****************************************************************************
	ExeEsc()
	This function executes an ESCAPE command.
An ESCAPE that is executed as a TECO command (as distinct from an ESCAPE
that is part of the syntax of some other TECO command) is ignored by TECO,
except that pending numeric values are discarded.  This command is useful
for discarding the value returned from a command (such as n%q) when that
value is not needed.
Two escapes cause TECO to exit from the current macro level.  If two escapes
are encountered from top level (not from within a macro),  then the command
string execution is terminated.  Both escapes must be true TECO commands
and not part of the syntax of some previous command.  That is, the first
ESCAPE cannot be part of the syntax of the preceding TECO command.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
DEFAULT ExeEsc()		/* execute an ESCAPE command */
{
	DBGFEN(1,"ExeEsc",NULL);
	if (*(CBfPtr+1) == ESCAPE) {		/* if it's <ESC><ESC> */
		if (TraceM) {			/* if tracing is on */
			ZDspCh('$');		/* echo the escape */
		}
		CBfPtr = CStEnd;		/* exit this macro level */
	} else {
		CmdMod = '\0';			/* clear modifiers flags */
		EStTop = EStBot;		/* clear expression stack */
	}
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
