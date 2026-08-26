/*****************************************************************************
	FlowEC()
	This function moves the command string pointer to the next matching
' character in the command string.  It is called when a F' command is
encountered or when a | character is encountered while a conditional command
string is executing.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT FlowEC()		/* flow to end of conditional */
{
	WORD	TmpNst;
	TmpNst = 1;
	do {
		if (CBfPtr == CStEnd) {
			ErrMsg(ERR_MAP);	/* missing ' */
			return FAILURE;
		}
		++CBfPtr;
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
	return SUCCESS;
}
