/*****************************************************************************
	ExePW()
	This function executes a PW command.  Note that this function is
called only by ExeP.
	nPW	Write buffer n times
	m,nPW	Write out chars m to n
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExePW()			/* execute a PW command */
{
	DBGFEN(1,"ExePW",NULL);
	if (CmdMod & MARGIS) {			/* if it's m,nPW */
		if (MArgmt != NArgmt) {		/* if m != n */
		    if ((*CBfPtr == 'W') || (*CBfPtr == 'w')) {
			MEMMOVE(ErrTxt, "m,nPW", 6);
		    } else {
			MEMMOVE(ErrTxt, "m,nP", 5);
		    }
		    if (GetAra() == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		    }
		    if (WrPage(CurOut,AraBeg,AraEnd,0L) == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		    }
		}
	} else {				/* else it's nPW */
		while (NArgmt-- > 0)
			if (WrPage(CurOut,EBfBeg,EBfEnd,-1L) == FAILURE) {
				DBGFEX(1,DbgFNm,"FAILURE");
				return FAILURE;
			}
	}
	CmdMod = '\0';				/* clear modifiers flags */
	EStTop = EStBot;			/* clear expression stack */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
