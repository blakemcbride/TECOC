/*****************************************************************************
	TypESt()
	This function types the erroneous command string.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
VVOID TypESt()			/* type erroneous command str */
{
/*
 * TypBuf's end pointer is exclusive,  so this has to be CBfPtr+1:  CBfPtr is
 * sitting ON the command that failed,  and "up to and including the erroneous
 * command" -- which is what ErrDsp promises and what the ? immediate-action
 * command is for -- is the whole point.  It used to leave that command out.
 */
	TypBuf(CStBeg,CBfPtr+1);
	ZDspBf("?\r\n", 3);
}
