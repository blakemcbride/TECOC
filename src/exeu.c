/*****************************************************************************
	ExeU()
	This function performs a U command.
	nUq	Put number n into Q-register q
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeU()					/* execute U command */
{
	DBGFEN(1,"ExeU",NULL);
	if (EStTop == EStBot) {			/* if no numeric argument */
		ErrMsg(ERR_NAU);
		DBGFEX(1,DbgFNm,"FAILURE, no arg before U");
		return FAILURE;
	}
	if (GetNmA() == FAILURE) {		/* get numeric argument */
		DBGFEX(1,DbgFNm,"FAILURE, GetNmA() failed");
		return FAILURE;
	}
	if (IncCBP() == FAILURE) {		/* increment to Q-reg name */
		DBGFEX(1,DbgFNm,"FAILURE, IncCBp() failed.");
		return FAILURE;
	}
	if (FindQR() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, FindQR() failed");
		return FAILURE;
	}
	QR->Number = NArgmt;
	if (CmdMod & MARGIS) {			/* if m,nUq */
/*
 * "m,nUq" returns m as a plain single value.  Leaving MARGIS set made the NEXT
 * command think it had been given an m,n pair (with a stale MArgmt),  so the
 * manual's own "UAUB" idiom for saving two macro arguments did the wrong thing.
 */
		CmdMod = '\0';			/* clear modifiers flags */
		DBGFEX(1,DbgFNm,"PushEx()");
		return PushEx(MArgmt, OPERAND);
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
