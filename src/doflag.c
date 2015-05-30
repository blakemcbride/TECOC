/*****************************************************************************
	DoFlag()
	This function gets or sets the value of a TECO mode control flag.
	The TECO mode control flags are ED, EH, ES, ET, EU, EV, EZ and ^X.
The EO flag can only be examined,  not set,  so ExeEO does not call this
function.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT DoFlag(Flag)		/* handle a mode control flag */
WORD *Flag;
{
	DBGFEN(2,"DoFlag",NULL);
	if ((EStTop == EStBot) ||		    /* if no numeric arg or */
	    (EStack[EStTop].ElType != OPERAND)) {   /* partial expression */
		DBGFEX(2,DbgFNm,"PushEx");
		return PushEx((LONG)*Flag, OPERAND);	/* return the flag */
	}
	if (GetNmA() == FAILURE) {		/* get the numeric argument */
		DBGFEX(2,DbgFNm,"FAILURE, GetNmA() failed");
		return FAILURE;
	}
	if ((CmdMod & MARGIS) == '\0') {	/* if it's n<flag> */
		*Flag = (WORD)NArgmt;		/* set the flag to n */
	} else if (MArgmt && NArgmt) {		/* else if it's m,n<flag> */
		*Flag &= (WORD)~MArgmt;		/* turn off m bits */
		*Flag |= (WORD)NArgmt;		/* turn on n bits */
	} else if ((MArgmt == 0) && NArgmt) {	/* else if it's 0,n<flag> */
		*Flag |= (WORD)NArgmt;		/* turn on n bits */
	} else if ((MArgmt) && (NArgmt == 0)) {	/* else if it's m,0<flag> */
		*Flag &= (WORD)~MArgmt;		/* turn off m bits */
	}
	EStTop = EStBot;			/* clear expression stack */
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(2,DbgFNm,"SUCCESS");
	return SUCCESS;
}
