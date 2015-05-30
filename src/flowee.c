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
	return SUCCESS;
}
