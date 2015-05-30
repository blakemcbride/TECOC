/*****************************************************************************
	ExeJ()
	This function executes a J command.
	nJ	Move pointer to "n"
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeJ()			/* execute J command */
{
	DBGFEN(1,"ExeJ",NULL);
	if (EStTop == EStBot) {			/* if no numeric argument */
		NArgmt = 0;			/* default is 0J */
	} else {
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	DBGFEX(1,DbgFNm,"DoCJR()");
	return DoCJR(EBfBeg-GapBeg+NArgmt);
}
