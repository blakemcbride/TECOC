/*****************************************************************************
	IncCBp()
	This function increments the command buffer pointer CBfPtr.  It
checks whether incrementing the command buffer pointer will move the command
buffer pointer past the end of the command buffer.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT IncCBP()
{
	if (CBfPtr == CStEnd) {			/* if end of command string */
		if (MStTop < 0) {		/* if not in a macro */
			ErrUTC();		/* unterminated command */
			return FAILURE;
		}
		return SUCCESS;
	}
	++CBfPtr;
	if (TraceM) {				/* if trace mode is on */
		EchoIt(*CBfPtr);		/* display the character */
	}
	return SUCCESS;
}
