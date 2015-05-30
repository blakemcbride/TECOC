/*****************************************************************************
	ExeB()
	This function executes a B command.
	B	Always equivalent to zero. Thus, B represents the
		position preceding the first character in the buffer.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeB()			/* execute a B command */
{
	DBGFEN(1,"ExeB",NULL);
	DBGFEX(1,DbgFNm,"PushEx(0)");
	return PushEx(0L, OPERAND);
}
