/*****************************************************************************
	DoFlag()
	This function gets or sets the value of a TECO mode control flag.
	The TECO mode control flags are ED, EH, ES, ET, EU, EV, EZ and ^X.
The EO flag can only be examined,  not set,  so ExeEO does not call this
function.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
/*
 * The flags used to be 16-bit WORDs,  which silently truncated everything the
 * user set:  ET_TRAP_CTRL_C is 32768,  so "32768ET" stored -32768 and "ET="
 * reported a negative number,  and anything above 65535 vanished altogether.
 */
DEFAULT DoFlag(Flag)		/* handle a mode control flag */
LONG *Flag;
{
	DBGFEN(2,"DoFlag",NULL);
	if ((EStTop == EStBot) ||		    /* if no numeric arg or */
	    (EStack[EStTop].ElType != OPERAND)) {   /* partial expression */
/*
 * Reading a flag consumes the modifiers the command was given,  so they have
 * to be cleared here too,  not just on the "set the flag" path below.  ":ED"
 * used to leave the colon set for the next command,  which then saw a colon it
 * was never given:  ":EDGA" typed Q-register A instead of inserting it.
 *	MARGIS and ATSIGN are the exceptions.  Reading a flag ends in the
 * PushEx below,  so it is a VALUE PRODUCER:  the value becomes the "n" of the
 * command that follows,  and any "m," or "@" written in front of it belongs to
 * THAT command.  Clearing MARGIS turned "0,EDT" into "<flag>T" and typed whole
 * lines where the user asked for a character range;  clearing ATSIGN made
 * "@EDS/x/" search for the three characters "/x/",  because the at-sign that
 * was to be the S command's delimiter modifier had gone.  ".", "B", "Z", "H",
 * "^R" and the other operands of the tree have always left both alone.
 *	The colon is different: it is cleared,  because ":ED" is not a command
 * and the bit has to go somewhere.  The set path below consumes the pair
 * itself and goes on clearing the whole of CmdMod.  See the commentary on
 * FSAray in skpcmd.c.
 */
		CmdMod &= (MARGIS | ATSIGN);		/* clear modifiers */
		DBGFEX(2,DbgFNm,"PushEx");
		return PushEx(*Flag, OPERAND);		/* return the flag */
	}
	if (GetNmA() == FAILURE) {		/* get the numeric argument */
		DBGFEX(2,DbgFNm,"FAILURE, GetNmA() failed");
		return FAILURE;
	}
	if ((CmdMod & MARGIS) == '\0') {	/* if it's n<flag> */
		*Flag = NArgmt;			/* set the flag to n */
	} else if (MArgmt && NArgmt) {		/* else if it's m,n<flag> */
		*Flag &= ~MArgmt;		/* turn off m bits */
		*Flag |= NArgmt;		/* turn on n bits */
	} else if ((MArgmt == 0) && NArgmt) {	/* else if it's 0,n<flag> */
		*Flag |= NArgmt;		/* turn on n bits */
	} else if ((MArgmt) && (NArgmt == 0)) {	/* else if it's m,0<flag> */
		*Flag &= ~MArgmt;		/* turn off m bits */
	}
	EStTop = EStBot;			/* clear expression stack */
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(2,DbgFNm,"SUCCESS");
	return SUCCESS;
}
