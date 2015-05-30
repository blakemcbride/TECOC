/*****************************************************************************
	DoEvEs()
	This function "does" what's specified by the EV (edit verify) or
ES (search verify) flag.  The ES and EV flags are mode control flags that
allow the user to cause TECO to display a part of the edit buffer after
executing a command string (edit verify) or after a search (search verify).
Both the ES and EV flags are 0 by default,  which means don't do anything.
This function is only called when the ES or EV flag is non-zero.
	The flag is passed to this function for decoding.  This function
examines the flag and acts accordingly.  Both the ES and EV flags have the
following meaning:
	-1	do a 1V command to display the current line
	0	do nothing (this function is not even called)
	1-31	type out current line with line-feed at CP
	32-255	type out current line with ASCII character (32-255) at CP
	else	bottom byte as above, top byte is argument to V command
*****************************************************************************/
#if CURSES			/* need to define standout and standend? */
#if ULTRIX
#include <cursesX.h>		/* note: this has to come before zport.h */
#else
#include <curses.h>		/* note: this has to come before zport.h */
#endif
#endif
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
VVOID DoEvEs(Flag)		/* EV or ES flag code */
WORD Flag;
{
    unsigned char PtChar;		/* special "CP" character */
    unsigned char VLines;		/* argument to V command */
    if (Flag == 0) {
	return;
    }
    if (Flag == -1) {			/* -1 means do a 1V command */
	VLines = 1;			/* view 1 line */
	PtChar = '\0';
    } else {
	PtChar = (unsigned char)Flag;
	VLines = (unsigned char)(Flag >> 8);
	if (VLines == 0) {		/* be sure it's at least 1 */
	    VLines = 1;
	}
    }
    TypBuf(GapBeg+Ln2Chr((LONG)1 - (LONG)VLines), GapBeg);
#if CURSES
    if (GapEnd+1 < EBfEnd) {
	unsigned char c = *(GapEnd + 1);
	standout();
	if (c=='\b') {
	    ZDspBf("^H", 2);
	} else if (c=='\r') {
	    ZDspBf("^M", 2);
	} else if (c=='\n') {
	    ZDspBf("^J", 2);
	} else if (c & 0200) {
	    int i;
	    char a,b;
	    c = c & 0177;
	    i = c/16;
	    if (i==0)
		a='8';
	    else if (i==1)
		a=9;
	    else
		a = i - 2  + 'A';
	    i = (c % 16);
	    b = (i > 9) ?  i - 10 + 'A' : i + '0';
	    ZDspCh('[');
	    ZDspCh(a);
	    ZDspCh(b); 
	    ZDspCh(']');
	} else
	    ZDspCh(c);
	standend();
	TypBuf(GapEnd+2, GapEnd + Ln2Chr((LONG)VLines)+1);
    }
#else
    if ((PtChar >= '\001') && (PtChar <= '\037')) {
	ZDspCh(LINEFD);
    } else if (PtChar != 0) {
	ZDspCh(PtChar);
    }
    TypBuf(GapEnd+1, GapEnd+Ln2Chr((LONG)VLines)+1);
#endif
}
