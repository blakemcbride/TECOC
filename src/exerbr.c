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
	charptr		NewTxt;
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
/*
 * Whatever the destination Q-register held has to be discarded:  MakRom only
 * ever GROWS a text area,  so asking it for TmpSiz bytes on top of the old
 * contents (which are then overwritten from the start anyway) would leave the
 * block permanently oversized.  Get the new block FIRST though.  Freeing the
 * old one and then calling MakRom meant that a ?MEM left the register empty
 * with QStTop still raised -- the pushed copy on the stack and the register it
 * came from both gone,  from a command that reported a failure.
 */
	if (TmpSiz > 0) {
		NewTxt = (charptr)ZAlloc((SIZE_T)TmpSiz);
		if (NewTxt == NULL) {
			ErrMsg(ERR_MEM);	/* MEM = memory overflow */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
		MEMMOVE(NewTxt, TmpQRp->Start, (SIZE_T)TmpSiz);
	}
	if (QR->Start != NULL) {
		ZFree((voidptr)QR->Start);
		QR->Start = QR->End_P1 = NULL;
	}
	if (TmpSiz > 0) {
		QR->Start  = NewTxt;
		QR->End_P1 = NewTxt + TmpSiz;
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
