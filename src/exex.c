/*****************************************************************************
	ExeX()
	This function executes an X command.
	nXq	Put n lines into Q-register q
	m,nXq	Put characters m to n into Q-register q
	n:Xq	Append n lines to Q-register q
	m,n:Xq	Append characters m to n into Q-register q
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeX()			/* execute an X command */
{
	ptrdiff_t TmpSiz;
	charptr	  SvStart;		/* the old text area, kept until we... */
	charptr	  SvEnd_P1;		/* ...know nothing else can fail */
	DEFAULT	  status;
	DBGFEN(1,"ExeX",NULL);
	if (EStTop == EStBot) {			/* if no numeric argument */
		NArgmt = 1;			/* default is 1X */
	} else {
		UMinus();			/* if it's -X, make it -1X */
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE, GetNmA() failed");
			return FAILURE;
		}
	}
	if (IncCBP() == FAILURE) {		/* move to char after X */
		DBGFEX(1,DbgFNm,"FAILURE, IncCBP() failed");
		return FAILURE;
	}
	if (FindQR() == FAILURE) {		/* find q-register values */
		DBGFEX(1,DbgFNm,"FAILURE, FindQR() failed");
		return FAILURE;
	}
/*
 * Without the colon modifier X REPLACES the Q-register's text,  so the old text
 * has to go.  It used to be freed right here,  before GetAra had been given a
 * chance to reject the range and before MakRom had been given a chance to run
 * out of memory:  "0,99999Xq" reported ?POP and left the register EMPTY,  and a
 * ?MEM did the same.  ExeRBr states the rule -- a command that reports a
 * failure must not have destroyed anything -- so detach the old text here,
 * keep it until every arm below has succeeded,  and put it back on the way out
 * if one of them does not.
 */
	SvStart = SvEnd_P1 = NULL;
	if (!(CmdMod & COLON)) {		/* if no colon modifier */
		SvStart  = QR->Start;		/* detach, but do not free */
		SvEnd_P1 = QR->End_P1;
		QR->Start = QR->End_P1 = NULL;
	}
	status = SUCCESS;
	if ((CmdMod & MARGIS) == '\0') {	/* if it's nXq (not m,nXq) */
		NArgmt = Ln2Chr(NArgmt);
		if (NArgmt > 0) {
			if (MakRom((SIZE_T)NArgmt) == FAILURE) {
				DBGFMS(1,DbgFNm,"MakRom() failed");
				status = FAILURE;
				goto exexdone;
			}
			MEMMOVE(QR->End_P1, GapEnd+1, (SIZE_T)NArgmt);
			QR->End_P1 += NArgmt;
		} else if (NArgmt < 0) {
			if (MakRom((SIZE_T)-NArgmt) == FAILURE) {
				DBGFMS(1,DbgFNm,"MakRom() failed");
				status = FAILURE;
				goto exexdone;
			}
			MEMMOVE(QR->End_P1, GapBeg+NArgmt, (SIZE_T)-NArgmt);
			QR->End_P1 -= NArgmt;
		}
	} else if (NArgmt != MArgmt) {		/* else (it's m,nXq) */
		MEMMOVE(ErrTxt, "m,nX", 5);
		if (GetAra() == FAILURE) {
			DBGFMS(1,DbgFNm,"GetAra() failed");
			status = FAILURE;
			goto exexdone;
		}
		if ((AraBeg < GapBeg) && (AraEnd > GapEnd)) {
			TmpSiz = (GapBeg-AraBeg) + (AraEnd-GapEnd);
			if (TmpSiz > 0) {
				if (MakRom((SIZE_T)TmpSiz) == FAILURE) {
					DBGFMS(1,DbgFNm,"MakRom() failed");
					status = FAILURE;
					goto exexdone;
				}
				MEMMOVE(QR->End_P1,
					AraBeg,
					(SIZE_T)(GapBeg - AraBeg));
				QR->End_P1 += GapBeg-AraBeg;
				MEMMOVE(QR->End_P1,
					GapEnd+1,
					(SIZE_T)(AraEnd - GapEnd));
				QR->End_P1 += AraEnd-GapEnd;
			}
		} else {			/* else area is contiguous */
			TmpSiz = (AraEnd - AraBeg) + 1;
			if (TmpSiz > 0) {
				if (MakRom((SIZE_T)TmpSiz) == FAILURE) {
					DBGFMS(1,DbgFNm,"MakRom() failed");
					status = FAILURE;
					goto exexdone;
				}
				MEMMOVE(QR->End_P1, AraBeg, (SIZE_T)TmpSiz);
				QR->End_P1 += TmpSiz;
			}
		}
	}
exexdone:
	if (status == FAILURE) {
/*
 * Only the no-colon path detached anything.  ":Xq" APPENDS,  so QR->Start is
 * still the register's own text and MakRom leaves it valid when it fails --
 * freeing it here would destroy on the append path what we are protecting on
 * the replace path.
 */
		if (!(CmdMod & COLON)) {	/* put the old text back */
			if (QR->Start != NULL) {
				ZFree((voidptr)QR->Start);
			}
			QR->Start  = SvStart;
			QR->End_P1 = SvEnd_P1;
		}
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (SvStart != NULL) {			/* nothing can fail now */
		ZFree((voidptr)SvStart);
	}
	CmdMod = '\0';			/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
