/*****************************************************************************
	ExeY()
	This function executes a Y command.
	Y	Read into buffer
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeY()                          /* Y command */
{
	DBGFEN(1,"ExeY",NULL);
	if (EStTop > EStBot) {			/* if numeric argument */
		ErrMsg(ERR_NYA);		/* numeric argument with Y */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if ((EdFlag & ED_YANK_OK) == 0) {	/* if no yank when full EBf */
		if (IsOpnO[CurOut] &&		/* if output file is open */
		    ((GapBeg != EBfBeg) || (GapEnd != EBfEnd))) {
			ErrMsg(ERR_YCA);	/* Y command aborted */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	DBGFEX(1,DbgFNm,"ExeEY()");
	return ExeEY();
}
