/*****************************************************************************
	PopMac()
	This function "pops" the current macro environment,  which was
established by an earlier call to the PshMac function.  Part of this job
is de-allocating memory consumed by local q-registers.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT PopMac()		/* restore environment after macro exit */
{
    WORD	i;
    MSptr	MSp;		/* pointer into the macro stack */
    QRptr	QRp;		/* pointer into local Q-register table */
    BOOLEAN	RetVal;		/* TRUE if macro is returning a value */
    char	SvMArg;		/* the "m," of an "m,n" return value */
    DEFAULT	Status;		/* FAILURE if the returned value was bad */
    DBGFEN(1,"PopMac",NULL);
    Status = SUCCESS;
    RetVal = (EStTop > EStBot);
    if (RetVal) {		/* if macro is returning a value */
        if (GetNmA() == FAILURE) {
/*
 * The macro left something that isn't a value on the expression stack (say
 * "1+"),  and GetNmA has already reported ?IFE.  Do NOT return from here:
 * the environment below still describes the macro,  and CStBeg/CStEnd would
 * be left pointing into the Q-register's text.  Nothing re-establishes them,
 * so the NEXT command string would be executed against that stale -- and,
 * once the Q-register is reused, freed -- memory.  Remember the failure,
 * finish unwinding, and report it at the end.
 */
	    RetVal = FALSE;		/* there is no value to push */
	    Status = FAILURE;
	}
    }
/*
 * Belt and braces for the modifier leak ExeEsc's <ESC><ESC> arm describes:  a
 * macro can also simply run off the end of its text with an "@", ":" or "::"
 * still pending,  in which case ExeEsc never runs at all.  The modifiers the
 * CALLER wrote were consumed before the macro was entered -- ExeM clears all
 * but MARGIS,  and MARGIS is the M command's own,  which it has now finished
 * with -- so nothing set here belongs to anybody but the macro.  (The
 * expression stack is deliberately NOT cleared:  that is the return value.)
 *	MARGIS is the exception,  and for the same reason.  "m,n$$  This
 * command causes TECO to exit from the current macro level,  returning the
 * PAIR of values m and n as arguments to the first command following the macro
 * call."  The "n" is the value on the expression stack;  the "m" is MArgmt,
 * flagged by MARGIS,  and MArgmt is not part of the environment PshMac saves,
 * so it is still the macro's.  Clearing the bit here threw half of every pair
 * away:  "m,nMq" handed a pair INTO a macro correctly and nothing carried one
 * back out,  so a macro returning "2,5" ran as "5" in the caller.  Only when
 * the macro really is returning something -- a bare MARGIS with no value
 * behind it is not a return value,  it is a leftover.
 */
    SvMArg = (RetVal) ? (CmdMod & MARGIS) : '\0';
    CmdMod = '\0';			/* clear modifiers flags */
/*
 * restore old environment
 */
    MSp = &MStack[MStTop];
    CStBeg = MSp->CStBeg;		/* restore old command string start */
    CBfPtr = MSp->CBfPtr;		/* restore old command string ptr */
    CStEnd = MSp->CStEnd;		/* restore old command string end */
    EStBot = MSp->EStBot;		/* restore expression stack bottom */
    EStTop = MSp->EStTop;		/* restore expression stack top */
    LStBot = MSp->LStBot;		/* restore loop stack bottom */
    LStTop = MSp->LStTop;		/* restore loop stack top */
    if (MSp->QRgstr != NULL) {		/* local q-registers allocated? */
        for (QRp = MSp->QRgstr, i = 0; i < 36; ++i, ++QRp) {
	    if (QRp->Start != NULL) {
		ZFree((voidptr)QRp->Start);
	    }
	}
	ZFree(MSp->QRgstr);
    }
    --MStTop;
#if DEBUGGING
    sprintf(DbgSBf,"%s", (Status == FAILURE) ? "FAILURE"
				: (RetVal) ? "PushEx(NArgmt)" : "SUCCESS");
    DbgFEx(1,DbgFNm,DbgSBf);
#endif
    if (Status == FAILURE) {
	return FAILURE;
    }
    if (RetVal) {
	CmdMod |= SvMArg;		/* the "m," of "m,n$$", if there was one */
	return PushEx(NArgmt, OPERAND);
    }
    return SUCCESS;
}
