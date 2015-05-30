/*****************************************************************************
	ExeCtY()
	This function executes a control-Y command.
	^Y	<CTRL/Y> (caret/Y) is equivalent to ".+^S,.", the n,m
		numeric argument spanning the text just searched for or
		inserted.  This value may be used to recover from
		inserting a string in the wrong place.  Type "^YXSFR`"
		to store the string in Q-register S and remove it from
		the buffer.  You can then position the pointer to the
		right place and type "GS" to insert the string.
		[TECO-11 only]
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeCtY()		/* execute ^Y command */
{
	DBGFEN(1,"ExeCtY",NULL);
	MArgmt = (GapBeg-EBfBeg) + RefLen;	/* set m part of m,n pair */
	CmdMod |= MARGIS;			/* say that m part exists */
	DBGFEX(1,DbgFNm,"PushEx()");
	return PushEx((LONG)(GapBeg-EBfBeg), OPERAND); /* push n part */
}
