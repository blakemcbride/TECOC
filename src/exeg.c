/*****************************************************************************
	ExeG()
	This function executes a G command.
	Gq	Get string from Q-register q into buffer
	G*	Get last filespec string into buffer
	G_	Get last search string into buffer
	:Gq	Type Q-register q on terminal
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeG()			/* execute a G command */
{
	charptr TxtBeg;		/* beginning of a text area */
	charptr TxtEnd;		/* end of a text area */
	DBGFEN(1,"ExeG",NULL);
	if (IncCBP() == FAILURE) {		/* move to char after G */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (*CBfPtr == '*') {			/* if it's G* */
		TxtBeg = FBfBeg;
		TxtEnd = FBfPtr;
	} else if (*CBfPtr == '_') {		/* else if it's G_ */
		TxtBeg = SBfBeg;
		TxtEnd = SBfPtr;
	} else {				/* else it's Gq */
		if (FindQR() == FAILURE) {
			return FAILURE;
		}
		if (QR->Start == NULL) {	/* if empty */
			RefLen = 0;
			CmdMod = '\0';		/* clear modifiers flags */
			EStTop = EStBot;	/* clear expression stack */
			DBGFEX(1,DbgFNm,"SUCCESS, Q-register empty");
			return SUCCESS;
		}
		TxtBeg = QR->Start;
		TxtEnd = QR->End_P1;
	}
	if (CmdMod & COLON) {			/* if it's :G */
		TypBuf(TxtBeg, TxtEnd);
	} else {				/* else (no : modifier) */
		if (InsStr(TxtBeg, TxtEnd-TxtBeg) == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	CmdMod = '\0';				/* clear modifiers flags */
	EStTop = EStBot;			/* clear expression stack */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
