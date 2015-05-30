/*****************************************************************************
	ExeV()
	This function executes a V command.  The V command is for "viewing"
some lines around the cursor.
	nV	do 1-nTnT
	m,nV	do 1-mTnT
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeV()			/* execute an V command */
{
	LONG firstarg;
	DBGFEN(1,"ExeV",NULL);
	if (EStTop == EStBot) {			/* if no numeric argument */
		NArgmt = 1;			/* default is 1V */
	} else {
		UMinus();			/* if it's -V, make it -1V */
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	firstarg = 1 - ((CmdMod & MARGIS) ? MArgmt : NArgmt);
	if (firstarg <= 0) {
		TypBuf(GapBeg+Ln2Chr(firstarg), GapBeg);
	} else {
		TypBuf(GapEnd+1, GapEnd+Ln2Chr(firstarg)+1);
	}
	if (NArgmt <= 0) {
		TypBuf(GapBeg+Ln2Chr(NArgmt), GapBeg);
	} else {
		TypBuf(GapEnd+1, GapEnd+Ln2Chr(NArgmt)+1);
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
