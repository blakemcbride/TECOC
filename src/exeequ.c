/*****************************************************************************
	ExeEqu()
	This function executes the = (equals sign) command:
	n=	display n in decimal,  with carriage return
	n==	display n in octal,  with carriage return
	n===	display n in hexadecimal,  with carriage return
	n:=	display n in decimal,  without carriage return
	n:==	display n in octal,  without carriage return
	n:===	display n in hexadecimal,  without carriage return
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeEqu()		/* execute an = (equals sign) comand */
{
	DBGFEN(1,"ExeEqu",NULL);
	if (EStTop == EStBot) {			/* if no numeric argument */
		ErrMsg(ERR_NAE);		/* NAE = "No arg before =" */
		DBGFEX(1,DbgFNm,"FAILURE, no arg before =");
		return FAILURE;
	}
	if (GetNmA() == FAILURE) {		/* get the numeric argument */
		DBGFEX(1,DbgFNm,"FAILURE, GetNmA() failed");
		return FAILURE;
	}
	if (*(CBfPtr+1) != '=') {		/* if next char not = */
		MakDBf(NArgmt, 10);		/* make string in decimal */
	} else {
		if (IncCBP() == FAILURE) {	/* move to next char */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
		if (*(CBfPtr+1) != '=') {	/* if next char not = */
			MakDBf(NArgmt, 8);	/* make string in octal */
		} else {
			if (IncCBP() == FAILURE) { /* move to next char */
				DBGFEX(1,DbgFNm,"FAILURE");
				return FAILURE;
			}
			MakDBf(NArgmt, 16);	/* make string in hex */
		}
	}
	if (CmdMod & COLON) {			/* if colon modified */
		CmdMod &= ~COLON;		/* clear colon flag */
	} else {
		*DBfPtr++ = CRETRN;		/* append a carriage return */
		*DBfPtr++ = LINEFD;		/* append a line feed */
	}
	ZDspBf(DBfBeg, DBfPtr-DBfBeg);		/* display the string */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
