/*****************************************************************************
	ExeZ()
	This function executes a Z command.
	Z	Equivalent to the number of characters currently
		contained in the buffer.  Thus, Z represents the
		position following the last character in the buffer.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeZ()                  /* execute Z command */
{
#if DEBUGGING
	DBGFEN(1,"ExeZ",NULL);
	sprintf(DbgSBf,"PushEx(%ld)", ((GapBeg-EBfBeg)+(EBfEnd-GapEnd)));
	DbgFEx(1,DbgFNm,DbgSBf);
#endif
	return PushEx((LONG)((GapBeg-EBfBeg)+(EBfEnd-GapEnd)), OPERAND);
}
