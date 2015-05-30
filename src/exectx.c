/*****************************************************************************
	ExeCtX()
	This function executes a control-X command.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeCtX()		/* execute a ^X (control-X) command */
{
	DBGFEN(1,"ExeCtX",NULL);
	DBGFEX(1,DbgFNm,"DoFlag()");
	return DoFlag(&SMFlag);
}
