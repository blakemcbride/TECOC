/*****************************************************************************
	ExeCtN()
	This function executes a ^N (control-N or caret-N) command.
	^N	<CTRL/N> (caret/N) is the end of file flag.  It is
		equivalent to -1 if the file open on the currently
		selected input stream is at end of file, and zero
		otherwise.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeCtN()		/* execute a ^N (control-N) command */
{
	DBGFEN(1,"ExeCtN",NULL);
	DBGFEX(1,DbgFNm,"PushEx()");
	return PushEx((IsEofI[CurInp] ? -1L : 0L), OPERAND);
}
