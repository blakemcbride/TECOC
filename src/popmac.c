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
    DBGFEN(1,"PopMac",NULL);
    RetVal = (EStTop > EStBot);
    if (RetVal) {		/* if macro is returning a value */
        if (GetNmA() == FAILURE) {
	    DBGFEX(1,DbgFNm,"FAILURE, GetNmA() failed");
	    return FAILURE;
	}
    }
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
    sprintf(DbgSBf,"%s", (RetVal) ? "PushEx(NArgmt)" : "SUCCESS");
    DbgFEx(1,DbgFNm,DbgSBf);
#endif
    return ((RetVal) ? PushEx(NArgmt, OPERAND) : SUCCESS);
}
