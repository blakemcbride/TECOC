/*****************************************************************************
	GetAra()
	This function is used by commands which can be preceded by two
arguments which specify an "area" of the edit buffer.  For instance,  the "T"
command can have the form "m,nT",  which will cause the characters between the
mth and nth character of the edit buffer to be typed on the terminal screen.
All commands which support m,n arguments call this function to compute the
addresses of the m and nth characters.  The resulting addresses are left
in the global variables AraBeg and AraEnd.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT GetAra()		/* get m,n addresses */
{
	LONG	TmpLng;
#if DEBUGGING
	static char *DbgFNm = "GetAra";
	sprintf(DbgSBf,"MArgmt = %ld, NArgmt = %ld", MArgmt, NArgmt);
	DbgFEn(4,DbgFNm,DbgSBf);
#endif
	if (MArgmt < 0) {			/* if negative m */
		ErrMsg(ERR_NCA);		/* negative argument to , */
		DBGFEX(4,DbgFNm,"FAILURE, negative m");
		return FAILURE;
	}
	if (NArgmt < 0)	{			/* if negative n */
		ErrStr(ERR_POP, ErrTxt);	/* POP = pointer off page */
		DBGFEX(4,DbgFNm,"FAILURE, negative n");
		return FAILURE;
	}
	if (MArgmt > NArgmt) {			/* if wrong order */
		TmpLng = NArgmt;
		NArgmt = MArgmt;
		MArgmt = TmpLng;
	}
	AraBeg = EBfBeg + MArgmt;		/* compute area beginning */
	if (AraBeg > GapBeg-1) {		/* if past start of gap */
		AraBeg += (GapEnd-GapBeg) + 1;	/* correct for gap */
	}
	AraEnd = (EBfBeg + NArgmt) - 1;		/* compute area end */
	if (AraEnd > GapBeg-1) {		/* if before end of gap */
		AraEnd += (GapEnd-GapBeg) + 1;	/* correct for gap */
	}
	if ((AraBeg > EBfEnd) ||		/* if m or n too large */
	    (AraEnd > EBfEnd)) {
		ErrStr(ERR_POP, ErrTxt);	/* POP = pointer off page */
		DBGFEX(4,DbgFNm,"FAILURE");
		return FAILURE;
	}
#if DEBUGGING
	sprintf(DbgSBf,"SUCCESS, AraBeg = %ld, AraEnd = %ld",
		Zcp2ul(AraBeg), Zcp2ul(AraEnd));
	DbgFEx(4,DbgFNm,DbgSBf);
#endif
	return SUCCESS;
}
