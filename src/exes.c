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
	WORD	SvEStB;			/* expression stack bottom, saved */
	DEFAULT	status;
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
/*
 * The test asks whether there is an OPERAND on top of the stack,  not merely
 * whether the stack is non-empty.  A "(" or a pending "+" leaves an OPERATOR
 * there,  and "::Stext$" is a VALUE PRODUCER -- it answers -1 or 0 -- so it
 * belongs in the middle of an expression.  "(::Sabc$)" and the obvious way to
 * ask whether the text starts with one string or another,  "::Sabc$#::Sdef$",
 * both had their operator read as an "n" and died with ?ILL.  ^R, ^T, ED, \ and
 * Qq have always used this form.
 */
		if ((EStTop > EStBot) &&	/* if it's n::S */
		    (EStack[EStTop].ElType == OPERAND)) {
			ErrStr(ERR_ILL, "n::S");
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
		if (GapEnd == EBfEnd) {		/* if nothing to search */
			if (FindES(ESCAPE) == FAILURE) {
				DBGFEX(1,DbgFNm,"FAILURE");
				return FAILURE;
			}
/*
 * This shortcut has to leave CmdMod the way the full path below leaves it.
 * It did not,  so the double-colon bit survived into the next command -- and
 * several commands reject "::" outright,  so "::Sx$Ny$" failed with
 * '?ILL Illegal command "::N"'.
 */
			CmdMod = '\0';		/* clear modifiers flags */
			DBGFEX(1,DbgFNm,"PushEx(0)");
			return PushEx(0L,OPERAND);
		}
		CmdMod &= ~DCOLON;		/* clear double-colon bit */
		CmdMod |= COLON;		/* set colon bit */
		CmdMod |= MARGIS;		/* set m defined bit */
		MArgmt = GapBeg - EBfBeg;	/* set m */
/*
 * The "n" below is not part of the user's expression:  it is how this command
 * hands ExeFB the second half of the ".,.+1" pair,  and Search pops it straight
 * back off again.  PushEx REDUCES as it pushes,  though,  so with an operator
 * of the user's already on the stack -- "::Sabc$#::Sdef$",  the obvious way to
 * ask whether the text starts with one string or the other -- the "#" ate it
 * the instant it was pushed and Search then took the user's own value as the
 * search bound.  Raising EStBot for the one push hides everything below it from
 * Reduce,  which is the same thing PshMac does to a macro's expression stack;
 * restore it before ExeFB runs,  so that the -1 or 0 IT pushes does reduce
 * against the user's pending operator.
 */
		SvEStB = EStBot;
		EStBot = EStTop;
		status = PushEx((LONG)((GapBeg-EBfBeg)+1),OPERAND);
		EStBot = SvEStB;
		if (status == FAILURE) {
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
