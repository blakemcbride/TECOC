/*****************************************************************************
	ExeH()
	This function executes an H command.
	H	Equivalent to the numeric pair "B,Z", or "from the
		beginning of the buffer up to the end of the buffer."
		Thus, H represents the whole buffer.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeH()			/* execute an H command */
{
	DBGFEN(1,"ExeH",NULL);
	MArgmt = 0;		/* load m with B */
	CmdMod |= MARGIS;	/* say there's an m,n pair */
	DBGFEX(1,DbgFNm,"ExeZ()");
	return ExeZ();		/* load n with Z */
}
