/*****************************************************************************
	TypBuf()
	This function displays a buffer on the terminal screen.  YBfBeg
points to the first character of the buffer,  and YBfEnd points to the
character following the last character of the buffer.
	If the "no conversions" bit is not set in the ET flag,  then this
function will convert non-displayable characters into a displayable form
before displaying them.  The following conversions are performed:
	1. uppercase and lowercase conversions based on EU flag
	2. <DEL> is not output
	3. <VT> to <lf><lf><lf><lf><lf>
	4. <FF> to <cr><lf><lf><lf><lf><lf>
	5. ^G to <BEL> and ^G
	6. <ESC> to $
	7. all control characters except <BS>, <TAB>, <LF>, <CR> and the
	   ones listed above to the ^x format,  where x is uppercase
	It is inefficient (though easy) to simply output each character in the
buffer to be displayed individually,  one after the other.  To make output
to the terminal fast requires calls to ZDspBf,  which outputs a group of
characters in one operation.  The following code groups each sequence of
characters containing no "convertable" characters and calls ZDspBf with the
group.  Whenever a character which needs to be converted is encountered,
the group up to the character is output,  a separate output call is made
for the special character,  and another group is started.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
#include "chmacs.h"		/* define character processing macros */
VVOID TypBuf(YBfBeg, YBfEnd)	/* type a buffer on the terminal */
charptr YBfBeg;
charptr YBfEnd;
{
    charptr YBfPtr;
#if DEBUGGING
    static char *DbgFNm = "TypBuf";
    sprintf(DbgSBf,"YBfBeg = %ld, YBfEnd = %ld, (length = %ld)",
	    Zcp2ul(YBfBeg), Zcp2ul(YBfEnd), (YBfEnd-YBfBeg));
    DbgFEn(5,DbgFNm,DbgSBf);
#endif
    if (EtFlag & ET_IMAGE_MODE) {		/* if no conversions */
        ZDspBf(YBfBeg, YBfEnd-YBfBeg);
	DBGFEX(5,DbgFNm,NULL);
	return;
    }
    YBfPtr = YBfBeg;
    while (YBfPtr < YBfEnd && !GotCtC) {
        if (EuFlag != EU_NONE) {		/* do case flagging? */
	    if (Is_Lower(*YBfPtr)) {
/*
 * It is lowercase. If EuFlag is EU_LOWER, flag it.  In any case, convert
 * to uppercase for display.
 */
	        if (YBfPtr != YBfBeg) {
		    ZDspBf(YBfBeg, YBfPtr-YBfBeg);
		}
		if (EuFlag == EU_LOWER) {
		    ZDspCh('\'');		/* flag it */
		}
		ZDspCh(*YBfPtr & '\137');
		YBfBeg = ++YBfPtr;
		continue;
	    } else if (Is_Upper(*YBfPtr) && (EuFlag == EU_UPPER)) {
/*
 * it is uppercase and we're flagging uppercase.
 */
	        if (YBfPtr != YBfBeg) {
		    ZDspBf(YBfBeg, YBfPtr-YBfBeg);
		}
		ZDspCh('\'');			/* flag it */
		ZDspCh(*YBfPtr);
		YBfBeg = ++YBfPtr;
		continue;
	    }
/* else it isn't lower or uppercase, fall through...*/
	}
/*
 * if *YBfPtr is a line feed, display what we've accumulated so far.
 * We do this so ^C's can interrupt the display of a large buffer.
 *
 * else if the character is displayable, accumulate it
 *
 * else (the character is a control character), handle it
 */
	if (*YBfPtr == LINEFD) {
	    ++YBfPtr;
	    ZDspBf(YBfBeg, YBfPtr-YBfBeg);
	    YBfBeg = YBfPtr;
	} else if (((*YBfPtr > USCHAR) && (*YBfPtr < DELETE)) ||
		   (*YBfPtr == TABCHR) ||
		   (*YBfPtr == CRETRN)) {
	    ++YBfPtr;
	} else if (*YBfPtr  & '\200') {		/* MSB set? */
	    if (EtFlag & ET_EIGHTBIT) {
	        ++YBfPtr;
	    } else {
	        if (YBfPtr != YBfBeg) {
		    ZDspBf(YBfBeg, YBfPtr-YBfBeg);
		}
		ZDspCh('[');
		MakDBf((LONG)*YBfPtr, 16);
		*DBfPtr++ = ']';
		ZDspBf(DBfBeg, DBfPtr-DBfBeg);
		YBfBeg = ++YBfPtr;
	    }
	} else {				/* it's a control character */
	    if (YBfPtr != YBfBeg) {
	        ZDspBf(YBfBeg, YBfPtr-YBfBeg);
	    }
	    switch (*YBfPtr) {
		case ESCAPE:	ZDspCh('$');
				break;
		case BAKSPC:	ZDspBf("^H",2);
				break;
		case FORMFD:	ZDspCh('\r');
				/* fall through */
		case VRTTAB:	ZDspBf("\n\n\n\n",4);
				/* fall through */
		case DELETE:	break;
		case CTRL_G:	ZDspCh(CTRL_G);
		default:	ZDspCh('^');
				ZDspCh(*YBfPtr | '\100');
	    }
	    YBfBeg = ++YBfPtr;
	}
    }
    if (YBfPtr != YBfBeg) {
        ZDspBf(YBfBeg, YBfPtr-YBfBeg);
    }
    DBGFEX(5,DbgFNm,NULL);
}
