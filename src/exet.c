/*****************************************************************************
	ExeT()
	This function executes a T command.
	nT	Type n lines
	m,nT	Type from m to n
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
DEFAULT ExeT()				/* execute T command */
{
	DBGFEN(1,"ExeT",NULL);
	if (EStTop == EStBot) {			/* if no numeric argument */
		NArgmt = 1;			/* default is 1T */
	} else {
		UMinus();			/* if it's -T, make it -1T */
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	if (CmdMod & MARGIS) {			/* if it's m,nT */
		if (NArgmt != MArgmt) {
			MEMMOVE(ErrTxt, "m,nT", 5);
			if (GetAra() == FAILURE) {
				DBGFEX(1,DbgFNm,"FAILURE");
				return FAILURE;
			}
/*
 * if the area to type out is split by the buffer gap,  we have to display
 * each half separately.
 */
			if ((AraBeg < GapBeg) && (AraEnd > GapEnd)) {
				TypBuf(AraBeg, GapBeg);
				TypBuf(GapEnd+1, AraEnd+1);
			} else {
				TypBuf(AraBeg, AraEnd+1);
			}
		}
	} else {					/* else it's nT */
		if (NArgmt <= 0) {
			TypBuf(GapBeg+Ln2Chr(NArgmt), GapBeg);
		} else {
			TypBuf(GapEnd+1, GapEnd+Ln2Chr(NArgmt)+1);
		}
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
