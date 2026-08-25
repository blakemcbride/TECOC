/*****************************************************************************

	ExeM()

	This function executes an M command.
	Mq	Execute string in Q-register q

*****************************************************************************/

#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */

DEFAULT ExeM()			/* execute an M command */
{
    DEFAULT status;
	BOOLEAN islocal;
    charptr MacTxt;		/* private copy of the macro text */
    ptrdiff_t MacLen;		/* length of the macro text */

    DBGFEN(1,"ExeM",NULL);

    if (IncCBP() == FAILURE) {		/* if no character after M */
	DBGFEX(1,DbgFNm,"FAILURE, no Q-register specified");
	return FAILURE;
    }

	islocal = (*CBfPtr=='.');		/* TAA Addition */

    if (FindQR() == FAILURE) {		/* find q-register */
	DBGFEX(1,DbgFNm,"FAILURE, FindQR() failed");
	return FAILURE;
    }

    if (QR->Start == NULL) {		/* if q-register is empty */
	DBGFEX(1,DbgFNm,"SUCCESS, Q-register is empty");
	return SUCCESS;
    }

    MacLen = QR->End_P1 - QR->Start;
    if (MacLen <= 0) {			/* nothing to execute */
	DBGFEX(1,DbgFNm,"SUCCESS, Q-register is empty");
	return SUCCESS;
    }

/*
 * Execute a COPY of the Q-register text,  not the Q-register text itself.
 * PshMac points CStBeg/CBfPtr/CStEnd at whatever we hand it,  and a macro is
 * free to write its own Q-register while it runs: "^Uq" frees the block,  and
 * ":^Uq", "n^Uq", "Xq", "]q" and "EQq" all reach MakRom,  which ZRalocs it and
 * can move it.  Either way the command string being executed was left dangling
 * and the interpreter carried on reading freed memory.  Copying costs one
 * allocation per macro call and makes the running macro immune to whatever it
 * does to the Q-register.
 */

    MacTxt = (charptr)ZAlloc((SIZE_T)MacLen);
    if (MacTxt == NULL) {
	ErrMsg(ERR_MEM);		/* MEM = memory overflow */
	DBGFEX(1,DbgFNm,"FAILURE, ZAlloc() failed");
	return FAILURE;
    }
    MEMMOVE(MacTxt, QR->Start, (SIZE_T)MacLen);

/*
 * Save the current execution state and make the copy be the command string
 * to execute.
 */

    if (PshMac(MacTxt, MacTxt + MacLen) == FAILURE) {
	ZFree((voidptr)MacTxt);
        DBGFEX(1,DbgFNm,"FAILURE, PshMac() failed");
	return FAILURE;
    }

/*
 * If the M command is not colon-modified, create a new set of local
 * q-registers and zero them out.
 */
    if ((CmdMod & COLON) == 0&&!islocal) {		/* if not colon-modified */
	QRptr QRp;
	WORD i;
	QRp = (QRptr)ZAlloc(36 * sizeof(struct QReg));
	if (QRp == NULL) {
	    ErrMsg(ERR_MEM);
/*
 * PshMac has already switched the command-string environment over to the
 * macro.  Returning without undoing that left CStBeg/CStEnd pointing into the
 * macro text with nothing to re-establish them,  so the next command string
 * would have been executed against it.  Unwind before giving up.
 */
	    (VVOID)PopMac();
	    ZFree((voidptr)MacTxt);
	    DBGFEX(3,DbgFNm,"FAILURE");
	    return FAILURE;
	}
	MStack[MStTop].QRgstr = QRp;
	for (i = 0; i < 36; ++i, ++QRp) {
	    QRp->Start = QRp->End_P1 = NULL;
	    QRp->Number = 0;
	}
    }

/*
 * The modifiers were written for the M command itself,  and the macro must not
 * see them:  an "@" would be taken as the delimiter modifier of the macro's
 * FIRST command,  and a "::" would turn its first search into a compare.  Only
 * MARGIS survives -- "m,nMq" is how a macro is given two arguments.
 */
    CmdMod &= MARGIS;		/* clear the modifiers written for us */

/*
 * execute command string in Q-register
 */
    status = ExeCSt();
#if DEBUGGING
    if (status == FAILURE) {
	DbgFMs(1,DbgFNm,"ExeCSt() failed");
    }
#endif

/*
 * restore old execution state
 */

    if (PopMac() == FAILURE) {
	ZFree((voidptr)MacTxt);
        DBGFEX(1,DbgFNm,"FAILURE, PopMac() failed");
	return FAILURE;
    }

    ZFree((voidptr)MacTxt);

    DBGFEX(1,DbgFNm,(status == FAILURE) ? "FAILURE" : "SUCCESS");
    return status;
}
