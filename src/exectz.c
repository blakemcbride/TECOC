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
/*
 * All of them:  QRgstr[0..35] are the globals,  QRgstr[36..71] the main level's
 * locals,  and each macro level on the stack has a table of its own.  Only the
 * globals used to be counted.
 */
	TmpLng = (CStEnd - CStBeg) + 1;
	for (TmpWrd = 0; TmpWrd < 72; ++TmpWrd) {
		TmpLng += QRgstr[TmpWrd].End_P1 - QRgstr[TmpWrd].Start;
	}
	for (TmpWrd = 0; TmpWrd <= MStTop; ++TmpWrd) {
		QRptr QRp = MStack[TmpWrd].QRgstr;
		if (QRp != NULL) {
			WORD i;
			for (i = 0; i < 36; ++i, ++QRp) {
				TmpLng += QRp->End_P1 - QRp->Start;
			}
		}
	}
#if DEBUGGING
	sprintf(DbgSBf,"PushEx(%ld)", TmpLng);
	DbgFEx(1,DbgFNm,DbgSBf);
#endif
	return PushEx(TmpLng,OPERAND);
}
