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
	TypBuf(CStBeg,CBfPtr);
	ZDspBf("?\r\n", 3);
}
