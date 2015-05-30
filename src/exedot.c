/*****************************************************************************
	ExeDot()
	This function executes a . (dot) command.
	.	Equivalent to the number of characters between the
		beginning of the buffer and the current position of the
		pointer. Thus "." represents the current position of
		the pointer.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeDot()		/* execute a . (dot) command */
{
#if DEBUGGING
	DBGFEN(1,"ExeDot",NULL);
	sprintf(DbgSBf,"PushEx(%ld)", (LONG)(GapBeg-EBfBeg));
	DbgFEx(1,DbgFNm,DbgSBf);
#endif
	return PushEx((LONG)(GapBeg-EBfBeg), OPERAND);
}
