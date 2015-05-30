/*****************************************************************************
	ExeK()
	This function executes a K command.
	nK	Kill n lines
	m,nK	Delete between m and n
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeK()			/* execute a K command */
{
	ptrdiff_t	HowFar;
	DBGFEN(1,"ExeK",NULL);
	if (EStTop == EStBot) {			/* if no numeric argument */
		NArgmt = 1;			/* default is 1K */
	} else {
		UMinus();			/* if it's -K, make it -1K */
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(1,DbgFNm,"FALURE");
			return FAILURE;
		}
	}
	if (CmdMod & MARGIS) {			/* if it's m,nK */
		if (NArgmt == MArgmt) {		/* if n equals m */
			CmdMod = '\0';		/* clear modifiers flags */
			DBGFEX(1,DbgFNm,"SUCCESS");
			return SUCCESS;
		}
		MEMMOVE(ErrTxt,
			(*CBfPtr == 'D' || *CBfPtr=='d') ? "m,nD" : "m,nK",
			5);
		if (GetAra() == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
		if ((AraBeg < GapBeg) && (AraEnd > GapEnd)) {
			GapBeg = AraBeg;
			GapEnd = AraEnd;
		} else {
			if (AraEnd < GapEnd) {
				HowFar = (GapBeg-1) - AraEnd;
				GapBeg -= HowFar;
				GapEnd -= HowFar;
				MEMMOVE(GapEnd+1, GapBeg, (SIZE_T)HowFar);
				GapBeg = AraBeg;
			} else {
				HowFar = AraBeg - (GapEnd+1);
				MEMMOVE(GapBeg, GapEnd+1, (SIZE_T)HowFar);
				GapBeg += HowFar;
				GapEnd = AraEnd;
			}
		}
	} else {				/* else it's nK */
		NArgmt = Ln2Chr(NArgmt);	/* cvt line to char offset */
		if (NArgmt > 0) {
			GapEnd += NArgmt;
		} else {
			GapBeg += NArgmt;
		}
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
