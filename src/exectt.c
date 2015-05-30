/*****************************************************************************
	ExeCtT()
	This function executes a ^T (control-T or caret-T) command.
	^T	ASCII value of next char typed
	n^T	Types out character for ASCII code n.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeCtT()		/* execute a ^T (control-T) command */
{
	DEFAULT lchar;
	DBGFEN(1,"ExeCtT",NULL);
	if ((EStTop == EStBot) ||		    /* if no numeric arg or */
	    (EStack[EStTop].ElType != OPERAND)) {     /* partial expression */
	    lchar = ZChIn(EtFlag & ET_NO_WAIT);	      /* read a character */
	    if (GotCtC) {			      /* if got a control-C */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	    }
	    if (((EtFlag & ET_NO_ECHO) == 0) && (lchar != -1)) {
		EchoIt((unsigned char)lchar);
	    }
	    DBGFEX(1,DbgFNm,"PushEx()");
	    return PushEx((LONG)lchar, OPERAND);
	}
	if (GetNmA() == FAILURE) {		/* get numeric arg */
	    DBGFEX(1,DbgFNm,"FAILURE, GetNmA() failed");
	    return FAILURE;
	}
	if ((CmdMod & COLON) || (EtFlag & ET_IMAGE_MODE)) {
	    ZDspCh((char)NArgmt);
	} else {
	    EchoIt((unsigned char)NArgmt);
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
