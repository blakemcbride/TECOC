/*****************************************************************************
	FindES()
	This function finds the end of the text argument to a command.
If the command is at-sign (@) modified,  the text argument may be delimited
by a special character.
	On entry,  CBfPtr points at the beginning of the text argument.
TrmChr contains the "usual" termination character for the command.  If the
ATSIGN bit of CmdMod is set,  it means the command is at-sign modified.
In that case the first character of the text argument (pointed to by CBfPtr)
is the delimiter character.
	On return,  ArgPtr points to the beginning of the true text argument
and CBfPtr points to the termination character.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT FindES(TrmChr)		/* find end of text argument */
unsigned char TrmChr;		/* termination char if no @ modifier */
{
#if DEBUGGING
    static char *DbgFNm = "FindES";
    sprintf(DbgSBf,", string is %sat-sign modified",
	(CmdMod & ATSIGN) ? "" : "not ");
    DbgFEn(3,DbgFNm,DbgSBf);
#endif
    if (IncCBP() == FAILURE) {		/* point to 1st char of text */
	DBGFEX(3,DbgFNm,"FAILURE, IncCBP() failed");
	return FAILURE;
    }
    if (CmdMod & ATSIGN) {		/* if it's @-modified */
	TrmChr = *CBfPtr;
	if (IncCBP() == FAILURE) {
	    DBGFEX(3,DbgFNm,"FAILURE, IncCBP() failed");
	    return FAILURE;
	}
    }
    ArgPtr = CBfPtr;
    while (*CBfPtr != TrmChr) {
	if (IncCBP() == FAILURE) {
	    DBGFEX(3,DbgFNm,"FAILURE");
	    return FAILURE;
	}
    }
    DBGFEX(3,DbgFNm,"SUCCESS");
    return SUCCESS;
}
