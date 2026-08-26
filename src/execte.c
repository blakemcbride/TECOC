/*****************************************************************************
	ExeCtE()
	This function executes a ^E (control-E or caret-E) command,  which
returns the value of the form feed flag.  The form feed flag is -1 if the
last read operation was terminated because a form feed was encountered.  It
is 0 otherwise.  If the ^E command is preceded by a numeric argument, then
the command sets the value of the form feed flag.  The form feed flag is
used by the output code to indicate whether a form feed should be appended
when the edit buffer is written to the output file.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeCtE()		/* execute a ^E (control-E) command */
{
    DBGFEN(1,"ExeCtE",NULL);
/*
 * The test asks whether there is an OPERAND on top of the stack,  not merely
 * whether the stack is non-empty.  A "(" or a pending "+" leaves an OPERATOR
 * there,  so "(^E)" and "1+^E" used to take the "set the flag" arm,  hand
 * GetNmA an operator and die with ?IFE -- yet ^E is the form feed flag,  which
 * belongs in the middle of an expression.  ^R, ^T, ED, \ and Qq have always
 * used this form.
 */
   if ((EStTop > EStBot) &&			/* if numeric argument */
       (EStack[EStTop].ElType == OPERAND)) {	/* ...and not a partial one */
	if (GetNmA() == FAILURE) {		/* get numeric arg */
	    DBGFEX(1,DbgFNm,"FAILURE");
	    return FAILURE;
	}
	FFPage = (NArgmt == -1) ? -1 : 0;
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
    }
/*
 * The READ form ends in the PushEx below,  so it is a VALUE PRODUCER and has to
 * leave MARGIS alone:  the "m," of "m,^E" belongs to the command that takes the
 * result,  and clearing it here turned "0,^ET" into "<flag>T".  The SET arm
 * above consumes its own argument -- and with it any "m," that came with it --
 * so it goes on clearing the whole of CmdMod.  See the commentary on FSAray in
 * skpcmd.c.
 */
    CmdMod &= MARGIS;				/* clear modifiers flags */
    DBGFEX(1,DbgFNm,"PushEx(FFPage)");
    return PushEx(FFPage, OPERAND);
}
