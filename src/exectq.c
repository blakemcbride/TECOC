/*****************************************************************************
	ExeCtQ()
	This function executes a ^Q (control-Q or caret-Q) command.
	n^Q	returns number of characters between the buffer pointer
		and the nth line separator (n^QC == nL).
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeCtQ()		/* execute a ^Q (control-Q) command */
{
	DBGFEN(1,"ExeCtQ",NULL);
/*
 * ^Q is a VALUE PRODUCER -- both of its exits are PushEx calls -- so it must
 * leave MARGIS and ATSIGN alone.  The "n" of "n^Q" is ^Q's own argument;  the
 * "m," and the "@" of "m,n^Q" and "@n^Q" belong to the command that takes the
 * result.  Clearing the whole of CmdMod on the way in threw the "m" away,  and
 * the manual's own way of saying "copy the next line" -- ".,.+(1^Q)Xq",
 * composing "n^QC == nL" with the character-range form of X -- ran as "5Xq":
 * five LINES,  which is the whole buffer.  ".,.+(1^Q)T" and ".,.+(1^Q)K" went
 * the same way.  Clearing ATSIGN went on to eat the delimiter modifier of the
 * command after it.  See DoFlag and the commentary on FSAray in skpcmd.c.
 *	The test asks whether there is an OPERAND on top of the stack,  not
 * merely whether the stack is non-empty.  A "(" or a pending "+" leaves an
 * OPERATOR there,  so "(^Q)" and "1+^Q" used to hand GetNmA an operator and die
 * with ?IFE instead of yielding the documented default of 0.  ^R, ^T, ED, \ and
 * Qq have always used this form.
 */
	if ((EStTop == EStBot) ||		    /* if no numeric arg or */
	    (EStack[EStTop].ElType != OPERAND)) {   /* partial expression */
		CmdMod &= (MARGIS | ATSIGN);	/* clear modifiers flags */
		DBGFEX(1,DbgFNm,"PushEx(0)");
		return PushEx(0L, OPERAND);
		}
	if (GetNmA() == FAILURE) {		/* get numeric argument */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	CmdMod &= (MARGIS | ATSIGN);		/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"PushEx()");
	return PushEx(Ln2Chr(NArgmt), OPERAND);
}
