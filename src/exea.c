/*****************************************************************************
	ExeA()
	This function executes an A command.
	A	Appends the next page of the input file to the contents
		of the text buffer, thus combining the two pages of
		text on a single page with no intervening form feed
		character.  This command takes no argument.  To perform
		n Appends, use the n<A> construct.  Note that nA is a
		completely different command.
	:A	Equivalent to the A command except that a value is
		returned.  -1 is returned if the append succeeded, and
		0 is returned if the append failed because the
		end-of-the-input-file had previously been reached (^N
		flag is -1 at start of this command).
	n:A	Appends n lines of text from the input file to the
		contents of the text buffer.  The value of n must not
		be negative.  A value is returned indicating whether or
		not there were in fact n lines remaining in the input
		file.  -1 is returned if the command succeeded.  0 is
		returned if end-of-file on the input file was
		encountered before all n lines were read in.  Note that
		the command can succeed and yet read in fewer than n
		lines in the case that the text buffer fills up.
	nA	Equivalent to the ASCII code for the .+n+1th character
		in the buffer (that is, the character to the right of
		buffer pointer position .+n). The expression -1A is
		equivalent to the ASCII code of the character
		immediately preceding the pointer and 0A is equivalent
		to the ASCII code of the character immediately
		following the pointer (the current character). If the
		character position referenced lies outside the bounds
		of the text buffer, this command returns a -1.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeA()			/* execute an A command */
{
    BOOLEAN		EBfFul;
    unsigned char	TmpChr;
    BOOLEAN		ColonMod;
    DBGFEN(1,"ExeA",NULL);
    ColonMod = (CmdMod & COLON);		/* is it :A or n:A */
    CmdMod &= ~COLON;				/* clear : flag */
/*
 * if we have a numeric argument, it's nA or n:A
 */
/*
 * The test asks whether there is an OPERAND on top of the stack,  not merely
 * whether the stack is non-empty.  A "(" or a pending "+" leaves an OPERATOR
 * there,  and this command's value-producing form belongs in the middle of an
 * expression:  "1+%A" and "1+:C" used to hand GetNmA an operator and die with
 * ?IFE where "1+QA" has always worked.  ^R, ^T, ED, \ and Qq use this form.
 *	UMinus has to run BEFORE the test:  a bare "-" is on the stack as an
 * OPERATOR,  and it is UMinus that turns the lone one into the operand -1.
 * Testing first made "-C", "-R", "-D", "-A", "-%q" and "-S" all look like
 * partial expressions and take their default of 1.
 */
    UMinus();					/* if it's -A, make it -1A */
    if ((EStTop > EStBot) &&			/* if numeric argument */
	(EStack[EStTop].ElType == OPERAND)) {	/* ...and not a partial one */
	if (GetNmA() == FAILURE) {		/* get numeric argument */
	    DBGFEX(1,DbgFNm,"FAILURE, GetNmA() failed");
	    return FAILURE;
	}
/*
 * Both arms below -- "nA" and "n:A" -- end in a PushEx,  so both are VALUE
 * PRODUCERS and both have to leave MARGIS alone.  The "n" is A's own argument;
 * the "m," of "m,nA" belongs to the command that takes the result,  so
 * clearing the flag here turned "0,1AT" into "<value>T" and typed the whole
 * buffer.  See the commentary on FSAray in skpcmd.c.
 *	ATSIGN goes with it,  and for the same reason:  this command has no
 * text argument for an "@" to modify,  so an at-sign written in front of it
 * belongs to the command that takes the result -- "@0AS/x/" searched for the
 * three characters "/x/".  See DoFlag.
 */
	CmdMod &= (MARGIS | ATSIGN);		/* clear modifiers flags */
	if (ColonMod) {				/* if it's n:A */
/*
 * The manual says of n:A only that "the value of n must not be negative".
 * Refusing 0 as well made "0:A" an error;  it should append no lines and
 * return -1,  which is what the loop below does when NArgmt is 0.
 */
	    if (NArgmt < 0) {
	        ErrMsg(ERR_IPA);
		DBGFEX(1,DbgFNm,"FAILURE, n:A, n < 0");
		return FAILURE;
	    }
	    while (NArgmt-- > 0) {
	        EBfFul = FALSE;
		if (RdLine(&EBfFul) == FAILURE) {
		    DBGFEX(1,DbgFNm,"FAILURE, RdLine() failed");
		    return FAILURE;
		}
		if (EBfFul) {
		    break;
		}
		if (IsEofI[CurInp]) {		/* if end-of-file */
		    DBGFEX(1,DbgFNm,"PushEx(0)");
		    return PushEx(0L, OPERAND);
		}
	    }
	    DBGFEX(1,DbgFNm,"PushEx(-1)");
	    return PushEx(-1L, OPERAND);
	}
/*
 * it's nA
 */
/*
 * NArgmt comes straight from the user,  so the range test has to be done in
 * integer arithmetic.  Forming "GapBeg+NArgmt" first,  as this used to,
 * overflows the pointer for a large argument -- which is undefined behaviour,
 * and in practice made the test give the wrong answer and the read below
 * fault.
 */
	if (NArgmt < 0) {
	    if (NArgmt < -(GapBeg - EBfBeg)) {		/* out of range */
	        DBGFEX(1,DbgFNm,"PushEx(-1)");
		return PushEx(-1L, OPERAND);
	    }
	    TmpChr = *(GapBeg+NArgmt);
	} else {
	    if (NArgmt > (EBfEnd - GapEnd) - 1) {	/* out of range */
	        DBGFEX(1,DbgFNm,"PushEx(-1)");
		return PushEx(-1L, OPERAND);
	    }
	    TmpChr = *(GapEnd+NArgmt+1);
	}
#if DEBUGGING
	sprintf(DbgSBf,"PushEx(%d)", TmpChr);
	DbgFEx(1,DbgFNm,DbgSBf);
#endif
	return PushEx((LONG)TmpChr, OPERAND);
    }
/*
 * if there is no numeric argument, must be A or :A
 */
/*
 * Only the COLON bit was cleared above,  so "@A" and "@:A" leaked their at-sign
 * into the next command.  ":A" is a value producer -- both of its exits below
 * are PushEx calls -- so it keeps MARGIS the way the numeric arm above does;
 * a bare "A" produces nothing,  so an "m," left in front of it has nowhere to
 * go and is dropped.
 */
    CmdMod = (ColonMod) ? (CmdMod & (MARGIS | ATSIGN)) : '\0';	/* modifiers */
    if (IsEofI[CurInp]) {			/* if already at end-of-file */
        DBGFEX(1,DbgFNm,(ColonMod) ? "PushEx(0)" : "SUCCESS");
	return ((ColonMod) ? PushEx(0L,OPERAND) : SUCCESS);
    }
    if (RdPage() == FAILURE) {			/* read a page */
        DBGFEX(1,DbgFNm,"FAILURE, RdPage() failed");
	return FAILURE;
    }
    DBGFEX(1,DbgFNm,(ColonMod) ? "PushEx(-1)" : "SUCCESS");
    return (ColonMod) ? PushEx(-1L, OPERAND) : SUCCESS;
}
