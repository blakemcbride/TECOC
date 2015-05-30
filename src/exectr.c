/*****************************************************************************
	ExeCtR()
	This function executes a ^R (control-R or caret-R) command.  The
control-R command sets TECO's radix,  which controls how ASCII strings are
converted to/from their binary representations.  The current radix is used
by the backslash command and whenever TECO encounters a string of digits in
a command string.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeCtR()		/* execute a ^R (control-R) command */
{
	DBGFEN(1,"ExeCtR",NULL);
	if ((EStTop == EStBot) ||		    /* if no numeric arg or */
	    (EStack[EStTop].ElType != OPERAND)) {     /* partial expression */
		DBGFEX(1,DbgFNm,"PushEx");
		return PushEx((LONG)Radix, OPERAND);
	}
	if (GetNmA() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, GetNmA() failed");
		return FAILURE;
	}
	if ((NArgmt != 8) && (NArgmt != 10) && (NArgmt != 16)) {
		ErrMsg(ERR_IRA);	/* illegal radix with ^R */
		DBGFEX(1,DbgFNm,"FAILURE, illegal radix");
		return FAILURE;
	}
	Radix = (DEFAULT)NArgmt;
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
