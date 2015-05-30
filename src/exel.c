/*****************************************************************************
	ExeL()
	This function executes an L command.
	nL	Advance n lines
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeL()			/* execute L command */
{
	LONG	HowFar;
	DBGFEN(1,"ExeL",NULL);
	if (EStTop == EStBot) {			/* if no numeric argument */
		NArgmt = 1;			/* default is 1 */
	} else {
		UMinus();			/* if it's -L, make it -1L */
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	HowFar = Ln2Chr(NArgmt);
	if (HowFar > 0) {
		MEMMOVE(GapBeg, GapEnd+1, (SIZE_T)HowFar);
		GapBeg += HowFar;
		GapEnd += HowFar;
	} else if (HowFar < 0) {
		GapBeg += HowFar;
		GapEnd += HowFar;
		MEMMOVE(GapEnd+1, GapBeg, (SIZE_T)-HowFar);
	}
	CmdMod = '\0';
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
