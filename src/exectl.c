/*****************************************************************************
	ExeCtL()
	This function executes a ^L (control-L or caret-L) command.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
DEFAULT ExeCtL()		/* execute an ^L (control-L) command */
{
	ZDspCh(FORMFD);
	return SUCCESS;
}
