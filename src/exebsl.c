/*****************************************************************************
	ExeBSl()
	This function executes a \ (backslash) command.
	\	Value of digit string in buffer
	n\	Convert n to digits in buffer
	The backslash command allows the user to convert binary numbers
to/from their ASCII representations.  If a numeric argument precedes the
backslash command,  then the ASCII representation for the number is inserted
into the edit buffer at the current point.  If there is no preceding numeric
argument, then the number in it's ASCII representation in the edit buffer is
converted to binary,  and is returned by the backslash command.  In either
case,  TECO's current radix controls how the numbers are represented.  If
the current radix is decimal,  then a plus or minus sign can precede an
ASCII number being read,  and a minus sign will be generated if necessary
for a number being generated.  When converting a string that's in the edit
buffer,  the character pointer is left at the end of the string in the buffer.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "chmacs.h"		/* define character processing macros */
DEFAULT ExeBSl()		/* execute a \ (backslash) command */
{
    DBGFEN(1,"ExeBSl",NULL);
/*
 * Handle the case where there's no numeric argument:  convert the digit
 * string in the edit buffer into a binary value and push it onto the
 * expression stack.
 */
    if ((EStTop == EStBot) ||			/* if no numeric arg or */
	(EStack[EStTop].ElType != OPERAND)) {	/* partial expression */
	LONG StrVal = 0;
/*
 * Accumulate in an unsigned type,  exactly as ExeDgt does for the same digit
 * string read out of the command string instead of out of the edit buffer.
 * A number too big for a LONG is not an error in TECO -- it just wraps,  the
 * same way +, - and * do -- but doing the arithmetic in a signed type makes
 * that overflow undefined behaviour.
 */
	ULONG Acc = 0;
	RefLen=0;
	if (GapEnd != EBfEnd) {			/* if not at end of buffer */
	    BOOLEAN negative = FALSE;
/*
 * Is_Digit and Is_Upper index ChrMsk[] with their argument,  so a plain char
 * would index ChrMsk[-128..-1] for an 8-bit character on a signed-char
 * platform.  IsRadx() gates the loop to 0-9/A-Z/a-z today,  but the guarantee
 * is not local and -Wchar-subscripts flags it.
 */
	    unsigned char NxtChr;
/*
 * "If there is no digit string following the pointer,  the result is zero and
 * THE POINTER POSITION REMAINS UNCHANGED."  The sign used to be consumed
 * before the digit loop had a chance to say whether any digits followed it,
 * so a "\" on a lone + or - moved dot over the sign and decremented RefLen --
 * the length ^S reports -- for a reference that was never made.  Look one
 * character past the sign before committing to it.
 */
	    if ((((*(GapEnd+1) == '-') || (*(GapEnd+1) == '+'))) &&
		((GapEnd+1) != EBfEnd) && IsRadx(*(GapEnd+2))) {
		negative = (*(GapEnd+1) == '-');
		RefLen--;
		GapEnd++;			/* move forward... */
		*GapBeg++ = *GapEnd;		/* ... one character */
	    }
	    while ((GapEnd != EBfEnd) && (IsRadx(*(GapEnd+1)))) {
		RefLen--;
		GapEnd++;			/* move forward... */
		*GapBeg++ = *GapEnd;		/* ... one character */
		NxtChr = *GapEnd;
		if (Is_Digit(NxtChr))
		    NxtChr -= '0';
		else if (Is_Upper(NxtChr))
		    NxtChr -= '\67';
		else
		    NxtChr -= '\127';
		Acc = (Acc * (ULONG)Radix) + (ULONG)NxtChr;
	    }
/*
 * "StrVal = -StrVal" on LONG_MIN is undefined too,  so negate in the unsigned
 * type and cast once,  as ExeDgt does.
 */
	    StrVal = negative ? (LONG)(0 - Acc) : (LONG)Acc;
	}
/*
 * This arm ends in the PushEx below,  so it is a VALUE PRODUCER:  its result
 * becomes the "n" of the command that follows,  and any "m," or "@" written in
 * front of it belongs to THAT command.  Clearing the whole of CmdMod threw both
 * away.  "0,\T" -- read the count out of the buffer and type that many
 * characters from the start,  which is what the digit string is there for --
 * ran as "<count>T" and typed whole LINES instead.  The "n\" arm below consumes
 * its argument and goes on clearing the lot.  See DoFlag and the commentary on
 * FSAray in skpcmd.c.
 */
	CmdMod &= (MARGIS | ATSIGN);		/* clear modifiers flags */
#if DEBUGGING
	sprintf(DbgSBf,"PushEx(%ld)", StrVal);
	DbgFEx(1,DbgFNm,DbgSBf);
#endif
	return PushEx(StrVal,OPERAND);
    }
/*
 * If we made it to here,  then there is a numeric argument
 */
    if (GetNmA() == FAILURE) {			/* get numeric argument */
	DBGFEX(1,DbgFNm,"FAILURE");
	return FAILURE;
    }
    MakDBf(NArgmt, Radix);			/* convert it to ASCII */
    if (InsStr(DBfBeg, DBfPtr-DBfBeg) == FAILURE) {
	DBGFEX(1,DbgFNm,"FAILURE");
	return FAILURE;
    }
    CmdMod = '\0';				/* clear modifiers flags */
    EStTop = EStBot;				/* clear expression stack */
    DBGFEX(1,DbgFNm,"SUCCESS");
    return SUCCESS;
}
