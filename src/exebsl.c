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
	RefLen=0;
	if (GapEnd != EBfEnd) {			/* if not at end of buffer */
	    BOOLEAN negative = FALSE;
	    char NxtChr;
	    if (*(GapEnd+1) == '-') {		/* minus sign? */
		RefLen--;
		GapEnd++;			/* move forward... */
		*GapBeg++ = *GapEnd;		/* ... one character */
		negative = TRUE;
	    } else if (*(GapEnd+1) == '+') {	/* plus sign? */
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
		StrVal = (StrVal * Radix) + NxtChr;
	    }
	    if (negative)
		StrVal = -StrVal;
	}
	CmdMod = '\0';				/* clear modifiers flags */
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
