/*****************************************************************************
	ExeUnd()
	This function executes an underscore (_) command.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeUnd()		/* execute a _ (underscore) command */
{
	DBGFEN(1,"ExeUnd",NULL);
/*
 * The command m,n_ is illegal: the user should use m,nFB
 */
	if (CmdMod & MARGIS) {			/* if it's m,n_ */
		ErrStr(ERR_ILL, "m,n_");	/* illegal command "m,n_" */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (CmdMod & DCOLON) {			/* if it's ::_ */
		ErrStr(ERR_ILL, "::_");		/* illegal command "::_" */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	SrcTyp = U_SEARCH;
	if (Search(FALSE) == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
