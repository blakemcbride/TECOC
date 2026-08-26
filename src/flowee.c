/*****************************************************************************
	FlowEE()
	This function moves the command string pointer to the next matching
' or | character in the command string.  It is called when a conditional
command (e.g. "E) is encountered and the condition fails, or when an F|
command is encountered.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT FlowEE()		/* flow to else or end (| or ') */
{
	WORD	TmpNst;
	TmpNst = 1;
	do {
		if (CBfPtr == CStEnd) {
/*
 * "?MAP  Missing ' -- every conditional (opened with the " command) must be
 * closed with the ' command."  That is what a conditional with no "'" gets;
 * ?UTC is documented for an unterminated text argument,  message,  tag or
 * comment.  FlowEC has always answered ?MAP here.  FlowEE is the arm that runs
 * when the test FAILS -- the common case -- and it answered ?UTC,  so the same
 * missing "'" was reported two different ways depending on which way the test
 * went.  F| outside a conditional reaches this line too,  and the manual gives
 * ?MAP for that as well.
 */
			ErrMsg(ERR_MAP);	/* missing ' */
			return FAILURE;
		}
		++CBfPtr;
		if ((*CBfPtr == '|') && (TmpNst == 1)) {
			break;
		}
		if (*CBfPtr == '"') {
			++TmpNst;
		} else if (*CBfPtr == '\'') {
			--TmpNst;
		}
/*
 * A conditional may be closed OUTSIDE the iteration its '"' was written inside:
 * "< ... "E ... > '" is how a macro breaks out of a loop on a test,  and
 * lib/squ.tes uses precisely that idiom.  The scan then walks past the ">",  so
 * the loop it is leaving has to be popped;  otherwise ExeCSt reaches the end of
 * the command string with LStTop still raised and reports an unterminated
 * iteration.  FlowEL and ExeO's tag scan both keep the stack this way.  Pushing
 * on "<" is what makes a loop written entirely inside the skipped region net out
 * to zero,  and LStBot guards the pop so the skip can never take the stack below
 * the current macro level.
 */
		if (*CBfPtr == '<') {
			if ((LStTop + 1) < LPS_SIZE) {
				++LStTop;
				LStack[LStTop].LIndex = INFINITE;
				LStack[LStTop].LAddr  = CBfPtr;
			}
		} else if ((*CBfPtr == '>') && (LStTop > LStBot)) {
			--LStTop;
		}
		if (SkpCmd() == FAILURE) {
			return FAILURE;
		}
	} while (TmpNst > 0);
	if (TraceM) {
		EchoIt(*CBfPtr);
	}
/*
 * Every other way out of a skipped stretch of command string ends with a
 * SkpCmd on the terminator,  and that clears CmdMod.  The "break" on "|"
 * above does not,  so a modifier written just before the else-part -- "@" or
 * ":" -- survived into the command AFTER the "|",  which then saw a modifier
 * it was never given.  FlowEC and FlowEL need no such line: they always reach
 * their terminator through SkpCmd,  or through a caller that clears.
 */
	CmdMod = '\0';				/* clear modifiers flags */
	return SUCCESS;
}
