/*****************************************************************************
	ExeAtS()
	This function executes an @ (at-sign) command.
	This function implements the at-sign modifier by setting a bit in
the CmdMod variable.  Commands which are sensitive to at-sign modification
check CmdMod explicitly.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeAtS()		/* execute an @ (at sign) command */
{
	DBGFEN(1,"ExeAtS",NULL);
	CmdMod |= ATSIGN;		/* set at sign */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
