/*****************************************************************************
	ExeD()
	This function executes a D command.
	nD	Delete n characters.
	m,nD	Delete between m and n
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeD()			/* execute a D command */
{
	LONG	Status;
	DBGFEN(1,"ExeD",NULL);
	if (CmdMod & MARGIS) {			/* if it's m,nD */
		DBGFEX(1,DbgFNm,"ExeK()");
		return ExeK();
	}
	if (EStTop == EStBot) {			/* if no numeric argument */
		NArgmt = 1;			/* default is 1D */
	} else {
		UMinus();			/* if it's -D, make it -1D */
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	Status = -1;					/* -1 means success */
	if (NArgmt > 0) {
		if ((GapEnd+NArgmt) > EBfEnd) {		/* if out of range */
			Status = 0;			/* 0 means failure */
		} else {
			GapEnd += NArgmt;		/* delete */
		}
	} else {
		if ((GapBeg+NArgmt) < EBfBeg) {		/* if out of range */
			Status = 0;			/* 0 means failure */
		} else {
			GapBeg += NArgmt;		/* delete */
		}
	}
	if (CmdMod & COLON) {			/* if it's :D */
		CmdMod = '\0';			/* clear modifiers flags */
		DBGFEX(1,DbgFNm,"PushEx()");
		return PushEx(Status, OPERAND);
	}
	if (Status == 0) {
		ErrMsg(ERR_DTB);		/* DTB = "delete too big" */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
