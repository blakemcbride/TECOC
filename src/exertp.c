/*****************************************************************************
	ExeRtP()
	This function executes a ) command.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeRtP()		/* execute right parenthesis command */
{
	DBGFEN(1,"ExeRtP",NULL);
/*
 * if no numeric arg.  or not a number
 */
	if ((EStTop == EStBot) || (EStack[EStTop].ElType != OPERAND)) {
		ErrMsg(ERR_NAP);		/* no argument before ) */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	DBGFEX(1,DbgFNm,"PushEx(')')");
	return PushEx((LONG)')', OPERATOR);
}
