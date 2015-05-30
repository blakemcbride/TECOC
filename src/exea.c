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
    if (EStTop > EStBot) {			/* if numeric argument */
        UMinus();				/* if it's -A, make it -1A */
	if (GetNmA() == FAILURE) {		/* get numeric argument */
	    DBGFEX(1,DbgFNm,"FAILURE, GetNmA() failed");
	    return FAILURE;
	}
	if (ColonMod) {				/* if it's n:A */
	    if (NArgmt < 1) {
	        ErrMsg(ERR_IPA);
		DBGFEX(1,DbgFNm,"FAILURE, n:A, n < 1");
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
	if (NArgmt < 0) {
	    if ((GapBeg+NArgmt) < EBfBeg) {
	        DBGFEX(1,DbgFNm,"PushEx(-1)");
		return PushEx(-1L, OPERAND);
	    }
	    TmpChr = *(GapBeg+NArgmt);
	} else {
	    if ((GapEnd+NArgmt+1) > EBfEnd) {
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
