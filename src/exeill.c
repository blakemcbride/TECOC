/*****************************************************************************
	ExeIll()
	This function "executes" an illegal command.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeIll()			/* illegal TECO command */
{
	ErrChr(ERR_ILL, *CBfPtr);	/* ILL = illegal command */
	return FAILURE;
}
