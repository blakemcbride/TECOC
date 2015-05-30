/*****************************************************************************
	ExeN()
	This function executes an N command.
	nNtext$		Global search
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeN()			/* execute an N command */
{
    DBGFEN(1,"ExeN",NULL);
/*
 * The command m,nN is illegal: the user should use m,nFB
 */
    if (CmdMod & MARGIS) {			/* if it's m,nN */
        ErrStr(ERR_ILL, "m,nN");		/* illegal command "m,nN" */
	DBGFEX(1,DbgFNm,"FAILURE");
	return FAILURE;
    }
    if (CmdMod & DCOLON) {			/* if it's ::N */
        ErrStr(ERR_ILL, "::N");			/* illegal command "::N" */
	DBGFEX(1,DbgFNm,"FAILURE");
	return FAILURE;
    }
    SrcTyp = N_SEARCH;
    if (Search(FALSE) == FAILURE) {
        DBGFEX(1,DbgFNm,"FAILURE");
	return FAILURE;
    }
    CmdMod = '\0';				/* clear modifiers flags */
    DBGFEX(1,DbgFNm,"SUCCESS");
    return SUCCESS;
}
