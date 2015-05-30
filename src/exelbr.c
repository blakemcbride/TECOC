/*****************************************************************************
	ExeLBr()
	This function executes a left-bracket ([) command.
	[q	Q-register push
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeLBr()				/* execute [ command */
{
	QRptr		TmpQRp;
	ptrdiff_t	TmpSiz;
#if DEBUGGING
	static char *DbgFNm = "ExeLBr";
	sprintf(DbgSBf, "QStTop = %d", QStTop);
	DbgFEn(1,DbgFNm,DbgSBf);
#endif
	if (IncCBP() == FAILURE) {
		DBGFEX(1,DbgFNm,"IncCBP() FAILURE");
		return FAILURE;
	}
	if (FindQR() == FAILURE) {
		DBGFEX(1,DbgFNm,"FindQR() FAILURE");
		return FAILURE;
	}
	if (++QStTop >= QRS_SIZE) {
		ErrMsg(ERR_PDO);		/* push-down list overflow */
		DBGFEX(1,DbgFNm,"ERR_PDO FAILURE");
		return FAILURE;
	}
/*
 * Copy QR to top of QStack
 */
	TmpQRp = QR;
	QR = &QStack[QStTop];
	TmpSiz = TmpQRp->End_P1 - TmpQRp->Start;
	if (TmpSiz > 0) {
		if (MakRom((SIZE_T)TmpSiz) == FAILURE) {
			QR = TmpQRp;
			--QStTop;
			DBGFEX(1,DbgFNm,"MakRom() FAILURE");
			return FAILURE;
		}
		MEMMOVE(QR->Start, TmpQRp->Start, (SIZE_T) TmpSiz);
		QR->End_P1 = QR->Start + TmpSiz;
	}
	QR->Number = TmpQRp->Number;
	QR = TmpQRp;
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
