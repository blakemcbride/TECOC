/*****************************************************************************
	ExeCtS()
	This function executes a ^S (control-S or caret-S) command.  This
is equivilent to the negative of the last referenced string (last insert,
string found, or string inserted with "G" command).
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeCtS()		/* execute a ^S (control-S) command */
{
	DBGFEN(1,"ExeCtS",NULL);
	DBGFEX(1,DbgFNm,"PushEx(RefLen)");
	return PushEx(RefLen, OPERAND);
}
