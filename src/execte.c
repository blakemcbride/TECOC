/*****************************************************************************
	ExeCtE()
	This function executes a ^E (control-E or caret-E) command,  which
returns the value of the form feed flag.  The form feed flag is -1 if the
last read operation was terminated because a form feed was encountered.  It
is 0 otherwise.  If the ^E command is preceded by a numeric argument, then
the command sets the value of the form feed flag.  The form feed flag is
used by the output code to indicate whether a form feed should be appended
when the edit buffer is written to the output file.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeCtE()		/* execute a ^E (control-E) command */
{
    DBGFEN(1,"ExeCtE",NULL);
   if (EStTop > EStBot) {			/* if numeric argument */
	if (GetNmA() == FAILURE) {		/* get numeric arg */
	    DBGFEX(1,DbgFNm,"FAILURE");
	    return FAILURE;
	}
	FFPage = (NArgmt == -1) ? -1 : 0;
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
    }
    CmdMod = '\0';				/* clear modifiers flags */
    DBGFEX(1,DbgFNm,"PushEx(FFPage)");
    return PushEx(FFPage, OPERAND);
}
