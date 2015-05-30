/*****************************************************************************
	MakDBf()
	This function converts a binary number to it's ASCII equivalent.
The resultant ASCII string is left in the digit buffer DBfBeg.  The second
argument to this function specifies the radix to be used.  If the radix is
decimal,  then a minus sign will be prepended to the number if it is negative.
If the radix is not decimal,  then minus sign are never prepended.
	The recursive algorithm for base 10 conversion comes from "C Traps and
Pitfalls" by Andrew Koenig.  In the "portability problems" section,  the book
discusses common problems converting binary numbers to characters.  One such
problem arises when the maximum negative number is presented to a conversion
algorithm.  Many algorithms don't handle this well,  including the one that
I used to use here.  So I stole his,  which avoids the problem and is also
impervious to the character collating sequence.  I liked using recursion to
order the string so much that I made the base 8 and 16 code use it too.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#if USE_PROTOTYPES
static VVOID DoHex(ULONG n);
static VVOID DoOct(ULONG n);
static VVOID DoNeg(LONG n);
#endif
static char digits[] = "0123456789ABCDEF";
static VVOID DoHex(n)
ULONG n;
{
    if (n != 0) {
        DoHex(n >> 4);
        *DBfPtr++ = digits[(int) n & 0x000f];
    }
}
static VVOID DoOct(n)
ULONG n;
{
    if (n != 0) {
        DoOct(n >> 3);
        *DBfPtr++ = digits[(int) n & 0x0007];
    }
}
/*
 *  Put the character representation for the negative number n into DBf
 */
static VVOID DoNeg(n)
LONG n;
{
    LONG quotient;
    DEFAULT remainder;
    quotient = n / 10;
    remainder = (int) (n % 10);	   /* trailing digit */
    if (remainder > 0) {   /* on some machines, remainder might be positive */
        remainder -= 10;
	quotient++;
    }
    if (n <= -10)
	DoNeg(quotient);
    *DBfPtr++ = digits[-remainder];
}
VVOID MakDBf(Binary, NRadix)	/* make digit buffer (DBfBeg) */
LONG Binary;			/* binary number to be converted */
DEFAULT NRadix;			/* radix to be used: 8, 10 or 16 */
{
        ULONG TmpBin;
	DBfPtr = DBfBeg;		/* initialize digit buffer ptr */
	if (Binary == 0) {		/* simple case? */
		*DBfPtr++ = '0';
		return;
	}
	if (NRadix == 10) {
		if (Binary < 0) {
		    *DBfPtr++ = '-';
		    DoNeg(Binary);
		} else {
		    DoNeg(-Binary);
		}
		return;
	}
	TmpBin = Binary;
	if (NRadix == 8) {
	    DoOct(TmpBin);
	} else {
	    DoHex(TmpBin);
	}
}
