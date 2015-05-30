/*****************************************************************************
	ExeRBr()
	This function executes a ] command.
	]q	Q-register pop
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeRBr()                /* execute ] command */
{
	QRptr		TmpQRp;
	ptrdiff_t	TmpSiz;
	BOOLEAN		ColonMod;
#if DEBUGGING
	static char *DbgFNm = "ExeRBr";
	sprintf(DbgSBf, "QStTop = %d", QStTop);
	DbgFEn(1,DbgFNm,DbgSBf);
#endif
	ColonMod = (CmdMod & COLON);		/* is it :] ? */
	CmdMod &= ~COLON;			/* clear colon flag */
	if (IncCBP() == FAILURE) {
		return FAILURE;
	}
	if (FindQR() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (QStTop < 0)	{			/* if q-reg stack is empty */
		if (ColonMod) {			/* if it's :] */
			DBGFEX(1,DbgFNm,"PushEx(0)");
			return PushEx(0L, OPERAND);
		} else {
			ErrMsg(ERR_PES);	/* can't pop empty stack */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
/*
 * Copy QStack Q-register to QR
 */
	TmpQRp = &QStack[QStTop];
	TmpSiz = TmpQRp->End_P1 - TmpQRp->Start;
	if (TmpSiz == 0) {			/* if it should be empty */
		if (QR->Start != NULL) {	    /* but it isn't empty */
			ZFree((voidptr)QR->Start);	/* then empty it */
			QR->Start = QR->End_P1 = NULL;
		}
	} else {
		if (MakRom((SIZE_T)TmpSiz) == FAILURE) { /* adjust QR space */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
		MEMMOVE(QR->Start, TmpQRp->Start, (SIZE_T)TmpSiz);
		QR->End_P1 = QR->Start + TmpSiz;
	}
	QR->Number = TmpQRp->Number;
/*
 * clear QStack Q-register
 */
	if (TmpQRp->Start != NULL) {
		ZFree((voidptr)TmpQRp->Start);
		TmpQRp->Start = TmpQRp->End_P1 = NULL;
	}
	TmpQRp->Number = 0;
	--QStTop;
	DBGFEX(1,DbgFNm,(ColonMod) ? "PushEx(-1)" : "SUCCESS");
	return (ColonMod) ? PushEx(-1L, OPERAND) : SUCCESS;
}
