/*****************************************************************************
	ExeUsc()
	This function executes a control-underscore (^_) command.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
DEFAULT ExeUsc()                /* execute control-_ command */
{
	DBGFEN(1,"ExeUsc",NULL);
	DBGFEX(1,DbgFNm,"PushEx(USCHAR)");
	return PushEx((LONG)USCHAR, OPERATOR);
}
