/*****************************************************************************
	ExeLst()
	This function executes a less-than-sign (<) command.
	n<	Iterate n times
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeLst()				/* execute a  < command */
{
	DBGFEN(1,"ExeLst",NULL);
	if (++LStTop >= LPS_SIZE) {		/* if loop stack is full */
		ErrMsg(ERR_PDO);		/* push-down list overflow */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (EStTop == EStBot) {			/* if no numeric argument */
		NArgmt = INFINITE;		/* make it an infinite loop */
	} else {
		if (GetNmA() == FAILURE) {	/* get numeric argmument */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
		if (NArgmt <= 0) {		/* if null loop */
			if (FlowEL()== FAILURE) { /* flow to end of loop */
				DBGFEX(1,DbgFNm,"FAILURE");
				return FAILURE;
			}
			CmdMod = '\0';		/* clear modifiers flags */
			DBGFEX(1,DbgFNm,"SUCCESS");
			return SUCCESS;
		}
	}
	LStack[LStTop].LIndex = NArgmt;		/* store loop index */
	LStack[LStTop].LAddr = CBfPtr;		/* store loop start */
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
