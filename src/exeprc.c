/*****************************************************************************
	ExePrc()
	This function executes a % command.
	n%q	Add n to Q-register q and return result
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExePrc()		/* execute % (percent) command */
{
	DBGFEN(1,"ExePrc",NULL);
	if (EStTop == EStBot) {			/* if no numeric argument */
		NArgmt = 1;			/* default is 1%q */
	} else {
		UMinus();			/* if it's -%, make it -1% */
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE, GetNmA() failed");
			return FAILURE;
		}
	}
	if (IncCBP() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, IncCBP() failed");
		return FAILURE;
	}
	if (FindQR() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, FindQR() failed");
		return FAILURE;
	}
	QR->Number += NArgmt;
/*
 * % is a VALUE PRODUCER -- it ends in the PushEx below -- so it must leave
 * MARGIS alone.  The "n" of "n%q" is %'s own argument;  the "m," of "m,n%q"
 * belongs to the command that takes the result,  so clearing the flag here
 * turned "0,2%9T" into "<result>T" and typed the whole buffer.  See the
 * commentary on FSAray in skpcmd.c.
 */
	CmdMod &= MARGIS;			/* clear modifiers flags */
#if DEBUGGING
	sprintf(DbgSBf,"PushEx(%ld,OPERAND)",QR->Number);
	DbgFEx(1,DbgFNm,DbgSBf);
#endif
	return PushEx(QR->Number, OPERAND);
}
