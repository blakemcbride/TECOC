/*****************************************************************************
	ExeCtQ()
	This function executes a ^Q (control-Q or caret-Q) command.
	n^Q	returns number of characters between the buffer pointer
		and the nth line separator (n^QC == nL).
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeCtQ()		/* execute a ^Q (control-Q) command */
{
	DBGFEN(1,"ExeCtQ",NULL);
	CmdMod = '\0';				/* clear modifiers flags */
	if (EStTop == EStBot) {			/* if no numeric argument */
		DBGFEX(1,DbgFNm,"PushEx(0)");
		return PushEx(0L, OPERAND);
		}
	if (GetNmA() == FAILURE) {		/* get numeric argument */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	DBGFEX(1,DbgFNm,"PushEx()");
	return PushEx(Ln2Chr(NArgmt), OPERAND);
}
