/*****************************************************************************
	ExeCtU()
	This function executes a ^U (control-U or caret-U) command.
	^Uqstring`
		This command inserts character string "string" into the text
		storage area of Q-register q.  When entering a command string
		from the terminal, you must specify ^U using the caret/U
		format, since the <CTRL/U> character is the line erase
		immediate action command.
	:^Uqstring`
		This command appends character string "string" to the text
		storage area of Q-register "q".
	n^Uq'	This form of the ^Uq` command inserts the single character
		whose ASCII code is n into the text storage area of
		Q-register "q".  (n is taken modulo 256 in TECO-11, modulo
		128 in other TECOs.)
	n:^Uq`	This form of the :^Uq` command appends the single character
		whose ASCII code is n to the text storage area of Q-register
		"q".  (n is taken modulo 256 in TECO-11, modulo 128 in other
		TECOs.) [not in TECO-8]
	@^Uq/string/
	@:^Uq/string/
	n@^Uq//
	n@:^Uq//
		Equivalent, respectively, to the ^Uqstring`, :^Uqstring`,
		n^Uq`, and n:^Uq` commands, except that alternate delimiters
		are used and no <DELIM> characters are necessary.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
#include "dchars.h"		/* define identifiers for characters */
DEFAULT ExeCtU()		/* execute a ^U (control-U) command */
{
	ptrdiff_t TmpSiz;
	DBGFEN(1,"ExeCtU",NULL);
/*
 * increment CBfPtr past ^U
 */
	if (IncCBP() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, IncCBP() failed");
		return FAILURE;
	}
/*
 * try to find Q-register name after ^U
 */
	if (FindQR() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, FindQR() failed");
		return FAILURE;
	}
/*
 * If there is a colon modifier, we are appending to the Q-register text.
 * If there isn't a colon modifier, we are replacing the text currently
 * in the Q-register.  If there is any text currently in the Q-register,
 * we have to zap it first.
 */
	if (!(CmdMod & COLON)) {		/* if no colon modifier */
		if (QR->Start != NULL) {	/* if not empty */
			ZFree((voidptr)QR->Start);	/* free the memory */
			QR->Start = QR->End_P1 = NULL;
		}
	}
/*
 * If there is a numeric argument n, we are dealing with a character to
 * place into or append to the Q-register.
 */
	if (EStTop > EStBot) {			/* if numeric argument */
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE, GetNmA() failed");
			return FAILURE;
		}
		if (CmdMod & ATSIGN) {		/* it's @^U// */
/*
 * increment CBfPtr to 1st delimiter
 */
			if (IncCBP() == FAILURE) {
				DBGFEX(1,DbgFNm,"FAILURE, IncCBP() failed");
				return FAILURE;
			}
/*
 * increment CBfPtr to 2nd delimiter
 */
			if (IncCBP() == FAILURE) {
				DBGFEX(1,DbgFNm,"FAILURE, IncCBP() failed");
				return FAILURE;
			}
/*
 * the two delimiters should be the same
 */
			if (*CBfPtr != *(CBfPtr-1)) {
				ErrMsg(ERR_IIA);
				DBGFEX(1,DbgFNm,"FAILURE, ERR_IIA");
				return FAILURE;
			}
		}
/*
 * Increase the size of the text area by 1 character
 */
		if (MakRom((SIZE_T) 1) == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE, MakRom() failed");
			return FAILURE;
		}
/*
 * Append the character to the Q-register
 */
		*(QR->End_P1) = (char) NArgmt;
		QR->End_P1++;
	} else {				/* else no numeric argument */
/*
 * We must be dealing with a string, find the end of the string.
 */
		if (FindES(ESCAPE) == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE, FindES() failed");
			return FAILURE;
		}
		TmpSiz = CBfPtr - ArgPtr;
		if (TmpSiz > 0) {
/*
 * make room for the string
 */
			if (MakRom((SIZE_T)TmpSiz) == FAILURE) {
				DBGFEX(1,DbgFNm,"FAILURE, MakRom() failed");
				return FAILURE;
			}
/*
 * Append the string to the Q-register text
 */
			MEMMOVE(QR->End_P1, ArgPtr, (SIZE_T)TmpSiz);
			QR->End_P1 += TmpSiz;
		}
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
