/*****************************************************************************
	ExeR()
	This function executes a R command.
	nR	Back up n characters
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeR()			/* execute a R command */
{
	DBGFEN(1,"ExeR",NULL);
	if (EStTop == EStBot) {			/* if no numeric argument */
		NArgmt = 1;			/* default is 1R */
	} else {
		UMinus();			/* if it's -R, make it -1R */
		if (GetNmA() == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	DBGFEX(1,DbgFNm,"DoCJR()");
	return DoCJR(-NArgmt);
}
