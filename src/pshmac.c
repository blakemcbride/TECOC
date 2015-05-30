/*****************************************************************************
	PshMac()
	This function is called when TECO needs to execute a macro.  It saves
several variables which define the "current" command string environment on
the macro stack.  It then defines the environment for the macro command
string,  so that the macro command can be executed when this function exits.
The PopMac function reverses the effects of this function,  redefining the
"current" command string environment.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT PshMac(Start, End)	/* push environment for a macro call */
charptr Start;			/* start of new command string */
charptr End;			/* end of new command string, plus one */
{
	MSptr	MSp;		/* pointer into the macro stack */
	BOOLEAN	NumArg;		/* TRUE if there is a number on the stack */
	DBGFEN(1,"PshMac",NULL);
	if (++MStTop >= MCS_SIZE) {		/* if macro stack is full */
		ErrMsg(ERR_PDO);		/* push-down list overflow */
		DBGFEX(1,DbgFNm,"FAILURE, macro stack is full");
		return FAILURE;
	}
	NumArg = (EStTop != EStBot) &&		/* if numeric arg and */
	    (EStack[EStTop].ElType == OPERAND);	/* not a partial expression */
	if (NumArg) {				/* if numeric argument */
		if (GetNmA() == FAILURE) {	/* get the numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE, GetNmA() failed");
			return FAILURE;
		}
	}
/*
 * save current environment
 */
	MSp = &MStack[MStTop];
	MSp->CStBeg = CStBeg;		/* save command string beginning */
	MSp->CBfPtr = CBfPtr;		/* save command string pointer */
	MSp->CStEnd = CStEnd;		/* save command string end */
	MSp->EStBot = EStBot;		/* save expression stack bottom */
	MSp->EStTop = EStTop;		/* save expression stack top */
	MSp->LStBot = LStBot;		/* save loop stack bottom */
	MSp->LStTop = LStTop;		/* save loop stack top */
	MSp->QRgstr = NULL;		/* no local q-reg table (yet) */
/*
 * setup new environment
 */
	CStBeg = CBfPtr = Start;	/* set up the new command string */
	CStEnd = End - 1;		/* set up the new command string */
	EStBot = EStTop;		/* set new expression stack bottom */
	LStBot = LStTop;		/* set new loop stack bottom */
	if (NumArg) {			/* if we had a numeric argument */
		if (PushEx(NArgmt, OPERAND) == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
