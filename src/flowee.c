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
			ErrUTC();		/* unterminated command */
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
