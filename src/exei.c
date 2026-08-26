/*****************************************************************************
	ExeI()
	This function executes an I command.
	Itext$		Insert text
	nI		Insert ASCII character "n"
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeI()			/* execute an I command */
{
	unsigned char InChar;
	DBGFEN(1,"ExeI",NULL);
	if (EStTop > EStBot) {			/* if numeric argument */
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE, GetNmA() failed");
			return FAILURE;
		}
		if (CmdMod & ATSIGN) {		/* if it's n@I// */
			if (IncCBP() == FAILURE) {
				DBGFEX(1,DbgFNm,"FAILURE, 1st IncCBP() failed");
				return FAILURE;
			}
			if (IncCBP() == FAILURE) {
				DBGFEX(1,DbgFNm,"FAILURE, 2nd IncCBP() failed");
				return FAILURE;
			}
			if (*CBfPtr != *(CBfPtr-1)) {
				ErrMsg(ERR_IIA);     /* illegal insert arg */
				DBGFEX(1,DbgFNm,"FAILURE, illegal insert arg");
				return FAILURE;
			}
		} else {				/* else must be nI$ */
/*
 * The test used to be "(CBfPtr+1) == CStEnd",  which was wrong twice over:
 * it rejected a perfectly good "nI$" whose ESCAPE happened to be the last
 * character of a macro,  and it did not stop the lookahead below from reading
 * past the end of the command string when the "I" itself was last.
 */
			if (CBfPtr == CStEnd) {
				if (MStTop < 0) {
					ErrUTC();
					DBGFEX(1,DbgFNm,"FAILURE, unterminated command");
					return FAILURE;
				} else {
					DBGFEX(1,DbgFNm,"SUCCESS");
					return SUCCESS;
				}
			}
			if (*(CBfPtr+1) != ESCAPE) {
				ErrMsg(ERR_IIA);     /* illegal insert arg */
				DBGFEX(1,DbgFNm,"FAILURE, illegal insert arg");
				return FAILURE;
			}
/*
 * The <DELIM> is part of the syntax of nI$,  so consume it -- ExeCtU does the
 * same for the identically-shaped n^Uq$,  and SkpArg consumes it on the skip
 * side.  Leaving CBfPtr on the "I" worked only because ExeCSt then handed the
 * stray ESCAPE to ExeEsc,  which swallowed it;  but ExeEsc also clears the
 * expression stack,  so anything the user had pending in front of the "n" was
 * thrown away,  which n^Uq$ does not do.
 */
			++CBfPtr;	/* consume the delimiter */
		}
		InChar = (char)NArgmt;
		if (InsStr(&InChar, (ptrdiff_t)1) == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE, InsStr() failed");
			return FAILURE;
		}
	} else {				/* else no numeric argument */
		if (FindES(ESCAPE) == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE, FindES() failed");
			return FAILURE;
		}
		if (InsStr(ArgPtr, CBfPtr-ArgPtr) == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE, InsStr() failed");
			return FAILURE;
		}
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
