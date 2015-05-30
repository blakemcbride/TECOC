/*****************************************************************************
	ExeDgt()
	This function handles a digit encountered in a command string.
It converts the digit and any following digits into a binary value using
the current radix,  and pushes the value onto the expression stack.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
#include "chmacs.h"		/* define character processing macros */
DEFAULT ExeDgt()		/* execute a digit command */
{
	LONG	TmpLng;
	DBGFEN(1,"ExeDgt",NULL);
	if ((Radix == 8) && (*CBfPtr > '7')) {	/* if bad octal digit */
		ErrMsg(ERR_ILN);		/* ILN = "illegal number" */
		DBGFEX(1,DbgFNm,"FAILURE, bad octal digit");
		return FAILURE;
	}
	TmpLng = 0;
	do {
		TmpLng *= Radix;
		if (Is_Digit(*CBfPtr)) {
			TmpLng += *CBfPtr - '0';
		} else if (Is_Upper(*CBfPtr)) {
			TmpLng += *CBfPtr - '\67';
		} else {
			TmpLng += *CBfPtr - '\127';
		}
		if (CBfPtr == CStEnd) {
#if DEBUGGING
			sprintf(DbgSBf,"PushEx(%ld,OPERAND)", TmpLng);
			DbgFEx(1,DbgFNm,DbgSBf);
#endif
			return PushEx(TmpLng, OPERAND);
		}
		++CBfPtr;
		if (IsRadx(*CBfPtr) && TraceM) {
			EchoIt(*CBfPtr);
		}
	} while (IsRadx(*CBfPtr));
	--CBfPtr;			/* cancel ExeCSt's ++CBfPtr */
#if DEBUGGING
	sprintf(DbgSBf,"PushEx(%ld,OPERAND)", TmpLng);
	DbgFEx(1,DbgFNm,DbgSBf);
#endif
	return PushEx(TmpLng, OPERAND);
}
