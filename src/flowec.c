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
		if (SkpCmd() == FAILURE) {
			return FAILURE;
		}
	} while (TmpNst > 0);
	if (TraceM) {
		EchoIt(*CBfPtr);
	}
	return SUCCESS;
}
