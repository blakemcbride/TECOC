/*****************************************************************************
	Replac()
	This function implements "search and replace" commands, like FS and
FN.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
DEFAULT Replac()
{
	DBGFEN(2,"Replac",NULL);
	if (Search(TRUE) == FAILURE) {
		DBGFEX(2,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (Matchd) {				/* if successful search */
		GapBeg += RefLen;		/* delete found string */
		if (InsStr(ArgPtr, CBfPtr-ArgPtr) == FAILURE) {
			DBGFEX(2,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}
	DBGFEX(2,DbgFNm,"SUCCESS");
	return SUCCESS;
}
