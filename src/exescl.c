/*****************************************************************************
	ExeSCl()
	This function executes a semi-colon (;) command.
	;	Exit iteration on search failure
	n;	Exit iteration if n is positive
	:;	Exit iteration on search success
	n:;	Exit iteration if n is negative
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeSCl()			/* execute semi-colon command */
{
#if DEBUGGING
	static char *DbgFNm = "ExeSCl";
	DbgFEn(1,DbgFNm,
		(CmdMod & COLON)
			? (EStTop == EStBot) ? "have :;" : "have n:;"
			: (EStTop == EStBot) ? "have ;"  : "have n;"   );
#endif
	if (LStTop == LStBot) {			/* if not in a loop */
		ErrMsg(ERR_SNI);		/* ; not in iteration */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (EStTop == EStBot) {			/* if no numeric argument */
		ErrMsg(ERR_NAS);		/* no argument before ; */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (GetNmA() == FAILURE) {		/* get numeric argument */
		DBGFEX(1,DbgFNm,"FAILURE. GetNmA failed()");
		return FAILURE;
	}
/*
 * If not colon-modified and numeric arg is positive OR
 *    if colon-modified and numeric arg is negative
 *	flow to the end of the loop and exit it
 */
	if ( (!(CmdMod & COLON) && (NArgmt >= 0) ) ||
	     ( (CmdMod & COLON) && (NArgmt <  0) ) ) {
		if (FlowEL() == FAILURE) {	/* flow to end of loop */
			DBGFEX(1,DbgFNm,"FAILURE, FlowEL() failed");
			return FAILURE;
		}
	}
	CmdMod = '\0';				/* clear modifiers flags */
	EStTop = EStBot;			/* clear expression stack */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
