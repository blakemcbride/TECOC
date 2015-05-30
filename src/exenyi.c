/*****************************************************************************
	ExeNYI()
	This function handles a TECO command that is "not yet implemented" by
displaying the message "not yet implemented" and failing.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeNYI()			/* command not yet implemented */
{
	ErrMsg(ERR_NYI);		/* NYI = not yet implemented */
	return FAILURE;
}
