/*****************************************************************************
	ExeCtZ()
	This function executes a control-Z command.
	^Z	<CTRL/Z> (caret/Z) is equivalent to the total space
		occupied by text in the Q-registers (including the
		command line currently being executed).
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeCtZ()		/* execute a ^Z (control-Z) command */
{
	LONG	TmpLng;
	WORD	TmpWrd;
	DBGFEN(1,"ExeCtZ",NULL);
	TmpLng = (CStEnd - CStBeg) + 1;
	for (TmpWrd = 0; TmpWrd < 36; ++TmpWrd) {
		TmpLng += QRgstr[TmpWrd].End_P1 - QRgstr[TmpWrd].Start;
	}
#if DEBUGGING
	sprintf(DbgSBf,"PushEx(%ld)", TmpLng);
	DbgFEx(1,DbgFNm,DbgSBf);
#endif
	return PushEx(TmpLng,OPERAND);
}
