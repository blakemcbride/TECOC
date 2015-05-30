/*****************************************************************************
	ExeEY()
	This function executes an EY command.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeEY()			/* execute an EY command */
{
	BOOLEAN	ColonMod;
	DBGFEN(1,"ExeEY",NULL);
	ColonMod = (CmdMod & COLON);		/* is it :EY ? */
	CmdMod &= ~COLON;			/* clear modifiers flags */
	GapBeg = EBfBeg;			/* clear the...   */
	GapEnd = EBfEnd;			/*   ...edit buffer */
	if (IsEofI[CurInp]) {			/* if at end-of-file */
		if (ColonMod) {			/* if it's :EY */
			DBGFEX(1,DbgFNm,"PushEx(0)");
			return PushEx(0L, OPERAND);
		} else {
			EStTop = EStBot;	/* clear expression stack */
			DBGFEX(1,DbgFNm,"SUCCESS");
			return SUCCESS;
		}
	}
	if (RdPage() == FAILURE) {		/* read a page */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (ColonMod) {				/* if it's :EY */
		DBGFEX(1,DbgFNm,"PushEx(-1)");
		return PushEx(-1L, OPERAND);
	}
	EStTop = EStBot;			/* clear expression stack */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
