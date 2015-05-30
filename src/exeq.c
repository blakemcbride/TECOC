/*****************************************************************************

	ExeQ()

	This function executes a Q command.

	Qq	Use the integer stored in the number storage area of
		Q-register q as the argument of the next command.

	nQq	Return the ASCII value of the (n+1)th character in
		Q-register q.  The argument n must be between 0 and the
		Q-register's size minus 1.  If n is out of range, a
		value of -1 is returned.  Characters within a
		Q-register are numbered the same way that characters in
		the text buffer are numbered.  The initial character is
		at character position 0, the next character is at
		character position 1, etc.  Therefore, if Q-register A
		contains "xyz", then 0QA will return the ASCII code for
		"x" and 1QA will return the ASCII code for "y".

	:Qq	Return the number of characters stored in the text storage
		area of Q-register q as the argument of the next command.

*****************************************************************************/

#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */

DEFAULT ExeQ()			/* execute a Q command */
{
	LONG	TmpVal;

	DBGFEN(1,"ExeQ",NULL);

/*
 * increment command buffer pointer to point to Q-register name
 */

	if (IncCBP() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, no Q-register specified");
		return FAILURE;
	}

/*
 * verify Q-register name and point QR global at it
 */

	if (FindQR() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, FindQR() failed");
		return FAILURE;
	}

/*
 * if something is on the stack and it's a number then we have nQq
 */

	if ((EStTop > EStBot) && (EStack[EStTop].ElType == OPERAND)) {
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE, GetNmA() failed");
			return FAILURE;
		}

/*
 * if NArgmt is between 0 and length of text area, return the ASCII value
 * of the (NArgmt+1)th character, otherwise return -1
 */

		TmpVal = (NArgmt >= 0 && NArgmt < (QR->End_P1 - QR->Start))
				? *(QR->Start + NArgmt)
				: -1;
		EStTop = EStBot;		/* clear expression stack */
	} else {				/* else we have Qq or :Qa */
		if (CmdMod & COLON) {		/* if it's :Qa */
			TmpVal = QR->End_P1 - QR->Start;
			CmdMod &= ~COLON;	/* clear colon flag */
		} else {
			TmpVal = QR->Number;
		}
	}
#if DEBUGGING
	sprintf(DbgSBf,"PushEx(%ld,OPERAND)",TmpVal);
	DbgFEx(1,DbgFNm,DbgSBf);
#endif
	return PushEx(TmpVal,OPERAND);
}
