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

/*
 * Save the current execution state and make the text in the Q-register be
 * the command string to execute.
 */

    if (PshMac(QR->Start, QR->End_P1) == FAILURE) {
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
	    DBGFEX(3,DbgFNm,"FAILURE");
	    return FAILURE;
	}
	MStack[MStTop].QRgstr = QRp;
	for (i = 0; i < 36; ++i, ++QRp) {
	    QRp->Start = QRp->End_P1 = NULL;
	    QRp->Number = 0;
	}
    }
    else CmdMod &= ~COLON;	/* clear colon flag TAA Mod */

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
        DBGFEX(1,DbgFNm,"FAILURE, PopMac() failed");
	return FAILURE;
    }

    DBGFEX(1,DbgFNm,(status == FAILURE) ? "FAILURE" : "SUCCESS");
    return status;
}
