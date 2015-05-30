/*****************************************************************************
	SinglP()
	This function performs the action of a single P command.  It must
	1.  write the edit buffer to the output file.
	2.  clear the edit buffer
	3.  read a page into the edit buffer
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
DEFAULT SinglP()		/* do a single P command */
{
	DBGFEN(2,"SinglP",NULL);
	if (WrPage(CurOut, EBfBeg, EBfEnd, FFPage) == FAILURE) {
		DBGFEX(2,DbgFNm,"FAILURE, WrPage() failed");
		return FAILURE;
	}
	GapBeg = EBfBeg;		/* clear the... */
	GapEnd = EBfEnd;		/* ...edit buffer */
	if (RdPage() == FAILURE) {
		DBGFEX(2,DbgFNm,"FAILURE, RdPage() failed");
		return FAILURE;
	}
	DBGFEX(2,DbgFNm,"SUCCESS");
	return SUCCESS;
}
