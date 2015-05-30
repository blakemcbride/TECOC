/*****************************************************************************
	ExeCCC()
	This function executes a ^^ (control-caret or caret-caret) command.
	^^x	Equivilent to the ASCII value of x
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeCCC()		/* execute a control-^ command */
{
	DBGFEN(1,"ExeCCC",NULL);
	if (IncCBP() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	DBGFEX(1,DbgFNm,"PushEx()");
	return PushEx((LONG)*CBfPtr, OPERAND);
}
