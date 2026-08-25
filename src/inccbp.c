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
	if (CBfPtr >= CStEnd) {			/* if end of command string */
/*
 * There is nothing left to move to.  This used to return SUCCESS without
 * moving CBfPtr when we were inside a macro,  which meant the caller silently
 * re-used the last character of the macro as though it were the next one.
 * Worse,  FindES loops calling IncCBP until it sees the terminator,  so an
 * unterminated string argument at the end of a macro spun forever -- and
 * since nothing in that loop tests GotCtC,  not even ^C could break out.
 *	The Standard TECO manual is explicit that "an entire command sequence
 * stored in a Q-register must be complete within the Q-register",  and ?UTM
 * is the error for a macro that isn't.  ErrUTC() already picks UTC or UTM
 * according to MStTop.
 */
		ErrUTC();			/* unterminated command/macro */
		return FAILURE;
	}
	++CBfPtr;
	if (TraceM) {				/* if trace mode is on */
		EchoIt(*CBfPtr);		/* display the character */
	}
	return SUCCESS;
}
