/*****************************************************************************
	ExeCtP()
	This function executes a ^P (control-P or caret-P) command.  ^P
is not a TECO command and is used to set TECO-C debugging parameters.  For
more information on the debugging routines, see the DEBUGGING lines in
Tecoc.c.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeCtP()		/* execute a ^P (control-P) command */
{
#if DEBUGGING
	if (DbgDsp() == FAILURE) {
		return FAILURE;
	}
	EStTop = EStBot;			/* clear expression stack */
	CmdMod = '\0';				/* clear modifiers flags */
	return SUCCESS;
#else
	return (ExeIll ());			/* Illegal command */
#endif
}
