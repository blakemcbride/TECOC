/*****************************************************************************

	Search()

	This function handles most of the special stuff that happens in
search commands (S, N, FS, FN, _, F_, E_, FB and FK).  It

	1. Loads the search buffer (SBfBeg, SBfEnd, SBfPtr) with the text
	   argument or leaves the search buffer alone if there is no
	   text argument.
	2. Sets variable NArgmt with the numeric argument or 1 if there
	   is no numeric argument.
	3. Calls SrcLop to do the basic search loop.
	4. If the search command is colon modified or if the search command
	   is in a loop and the next character after the search command is
	   a semicolon,  then return appropriately.
	5. If successful search,  return successfully but worry about
	   the ES flag.
	6. If unsuccessful search,  return unsuccessfully but worry about
	   whether we need to skip to the end of a loop or not.

	This function is called by ExeEUn, ExeFB, ExeFK, ExeFN, ExeFS, ExeFUn,
ExeN, ExeS and ExeUnd.  Each of those functions sets up the variable SrcTyp
before calling this function to do the search.

*****************************************************************************/

#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
#include "dchars.h"		/* define identifiers for characters */

DEFAULT Search(RepCmd)		/* main search code */
BOOLEAN	RepCmd;			/* TRUE if the command has two arguments */
{
	DBGFEN(2,"Search",NULL);

/*
 * If there is a search argument,  put it into SBf,  otherwise use the
 * string from the last search command.
 */

	if (*(CBfPtr+1) == ESCAPE) {		/* if it's s$, n$, etc. */
		++CBfPtr;			/* skip the escape */
	} else {
		if (BldStr(SBfBeg, SBfEnd, &SBfPtr) == FAILURE) {
			DBGFEX(2,DbgFNm,"FAILURE, BldStr() failed");
			return FAILURE;
		}
	}
/*
 * If the search is one of the "search and replace" commands,  then parse the
 * second argument (the replace string).  We need to do this here because we
 * may later need to check if a semicolon follows the command,  and we need
 * to be positioned at the end of the command to do so.
 */

	if (RepCmd) {				/* if FC, FN, FS or F_ */
/*
 * If the command is @-modified,  then we need to move the command pointer
 * back by one.  The call to Search will have eaten the first text argument
 * of the command,  leaving the pointer on the delimiter between the arguments.
 * We are about to call FindES,  which will call IncCBP to move to the next
 * character.   Moving back by one cancels the IncCBp.
 */

		if (CmdMod & ATSIGN) {		/* if it's at-sign modified */
			--CBfPtr;
		}

		if (FindES(ESCAPE) == FAILURE) {/* find end of 2nd argument */
			DBGFEX(2,DbgFNm,"FAILURE, FindES() failed");
			return FAILURE;
		}
	}

	if (EStTop == EStBot) {			/* if no numeric argument */
		NArgmt = 1;			/* default is 1S */
	} else {
		UMinus();			/* if it's -S, make it -1S */
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(2,DbgFNm,"FAILURE, GetNmA() failed");
			return FAILURE;
		}

		if (NArgmt == 0) {		/* if it's 0s, 0n, etc. */
			ErrMsg(ERR_ISA);	/* illegal search argument */
			DBGFEX(2,DbgFNm,"FAILURE, NArgmt == 0");
			return FAILURE;
			}
		}

	if (SrcLop() == FAILURE) {
		DBGFEX(2,DbgFNm,"FAILURE, SrcLop failed");
		return FAILURE;
	}

	if ((CmdMod & COLON) ||			/* if colon modifier or */
	    ((LStTop != LStBot) &&		/* (in loop and         */
	     (*(CBfPtr+1) == ';'))) {		/* next cmd = ;)        */
		if (Matchd) {			/* if successful search */
			if (EsFlag&& LStTop == LStBot && MStTop < 0) {
			        /* if ES flag is set, not in loop or macro */
				DoEvEs(EsFlag);	/* handle ES flag */
			}
			DBGFEX(2,DbgFNm,"PushEx(-1)");
			return PushEx(-1L, OPERAND);
		} else {			/* else unsuccessful search */
			DBGFEX(2,DbgFNm,"PushEx(0)");
			return PushEx(0L, OPERAND);
		}
	}

	if (Matchd) {				/* if successful search */
		if (EsFlag&& LStTop == LStBot && MStTop < 0) {
			/* if ES flag is set, not in loop and not in macro */
			DoEvEs(EsFlag);		/* handle ES flag */
		}
		DBGFEX(2,DbgFNm,"SUCCESS");
		return SUCCESS;
	} else {				/* else unsuccessful search */
		*SBfPtr = '\0';
		ErrPSt(ERR_SRH, SBfBeg, SBfPtr);/* "search failure" */
		DBGFEX(2,DbgFNm,(LStTop==LStBot) ? "FAILURE"  : "FlowEL()");
		return (LStTop == LStBot) ? FAILURE : FlowEL();
	}
}
