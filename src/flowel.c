/*****************************************************************************
	FlowEL()
	This function exits the current loop.  It is called when a loop
command (<) is executed with a numeric argument less than or equal to zero,
when a semi-colon command is executed with a numeric argument greater than
zero,  when a search command within a loop fails,  or when an F> command is
executed.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT FlowEL()		/* flow to end of loop */
{
	WORD	TmpNst;		/* temporary loop nest count */
	DBGFEN(3,"FlowEL",NULL);
	TmpNst = 1;
	do {
		if (CBfPtr == CStEnd) {		/* if end of command string */
			ErrUTC();		/* unterminated command */
			DBGFEX(3,DbgFNm,"FAILURE");
			return FAILURE;
		}
		++CBfPtr;			/* move to next command */
		if (*CBfPtr == '<') {		/* if loop start character */
			++TmpNst;		/* increment nesting count */
		} else if (*CBfPtr == '>') {	/* else if loop end char */
			--TmpNst;		/* decrement nesting count */
		} else {
			if (SkpCmd() == FAILURE) {
				DBGFEX(3,DbgFNm,"FAILURE");
				return FAILURE;
			}
		}
	} while (TmpNst > 0);
	if (TraceM) {				/* if tracing is on */
		EchoIt(*CBfPtr);		/* echo the character */
	}
	--LStTop;				/* pop loop stack */
	DBGFEX(3,DbgFNm,"FlowEL");
	return SUCCESS;
}
