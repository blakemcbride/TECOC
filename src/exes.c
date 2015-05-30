/*****************************************************************************
	ExeS()
	This function executes an S command.
	nStext$		Local search
	m,nStext$	Search for nth occurrance within m chars
	::Stext$	Compare string
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeS()			/* execute an S command */
{
	DBGFEN(1, "ExeS", NULL);
/*
 * The command m,nS is illegal: the user should use m,nFB
 */
	if (CmdMod & MARGIS) {			/* if it's m,nS */
		ErrStr(ERR_ILL, "m,nS");	/* illegal command "m,nS" */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
/*
 * If it's ::Stext$,  it's a compare,  not a search.  The text argument is
 * compared to the characters immediately following the character pointer.
 * It returns -1 if the strings match, else 0.  A ::Stext$ command is
 * equivalent to a .,.+1:FBtext$ command,  so that's the way it's implemented.
 */
	if (CmdMod & DCOLON) {			/* if it's ::S */
		if (CmdMod & MARGIS) {		/* if it's m,n::S */
			ErrStr(ERR_ILL, "m,n::S");
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
		if (EStTop > EStBot) {		/* if it's n::S */
			ErrStr(ERR_ILL, "n::S");
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
		if (GapEnd == EBfEnd) {		/* if nothing to search */
			if (FindES(ESCAPE) == FAILURE) {
				DBGFEX(1,DbgFNm,"FAILURE");
				return FAILURE;
			}
			DBGFEX(1,DbgFNm,"PushEx(0)");
			return PushEx(0L,OPERAND);
		}
		CmdMod &= ~DCOLON;		/* clear double-colon bit */
		CmdMod |= COLON;		/* set colon bit */
		CmdMod |= MARGIS;		/* set m defined bit */
		MArgmt = GapBeg - EBfBeg;	/* set m */
		if (PushEx((LONG)((GapBeg-EBfBeg)+1),OPERAND) == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
		DBGFEX(1,DbgFNm,"ExeFB()");
		return ExeFB();			/* execute FB command */
	}
	SrcTyp = S_SEARCH;
	if (Search(FALSE) == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
