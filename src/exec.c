/*****************************************************************************
	ExeC()
	This function executes a C command.
	nC	Advance n characters
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeC()			/* execute a C command */
{
	DBGFEN(1,"ExeC",NULL);
	if (EStTop == EStBot) {			/* if no numeric argument */
		NArgmt = 1;			/* default is 1C */
	} else {
		UMinus();			/* if it's -C, make it -1C */
		if (GetNmA() == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	DBGFEX(1,DbgFNm,"DoCJR");
	return DoCJR(NArgmt);
}
