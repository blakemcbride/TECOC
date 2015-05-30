/*****************************************************************************
	ExeDQu()
	This function executes a " (double-quote) command.
	"	Start conditional
	n"<	Text for less than zero
	n"=	Test for equal to zero
	n">	Test for greater than zero
	n"A	Test for alphabetic
	n"C	Test for symbol constituent
	n"D	Test for numeric
	n"E	Test for equal to zero
	n"F	Test for false
	n"G	Test for greater than zero
	n"L	Test for less than zero
	n"N	Test for not equal to zero
	n"R	Test for alphanumeric
	n"S	Test for successful
	n"T	Test for true
	n"U	Test for unsuccessful
	n"V	Test for lower case
	n"W	Test for upper case
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
#include "chmacs.h"		/* define character processing macros */
DEFAULT ExeDQu()		/* execute a " (double quote) command */
{
	unsigned char RelExp;			/* relational expression */
	DBGFEN(1,"ExeDQu",NULL);
	if (EStTop == EStBot) {			/* if no numeric argument */
		ErrMsg(ERR_NAQ);		/* no argument before " */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (GetNmA() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, GetNmA() failed");
		return FAILURE;
	}
	if (IncCBP() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, IncCBP() failed");
		return FAILURE;
	}
	CmdMod = '\0';
	RelExp = (unsigned char)NArgmt;
	switch (To_Upper(*CBfPtr)) {
	case 'A':	/* test for alphabetic */
		if (Is_Alpha(RelExp)) {
			DBGFEX(1,DbgFNm,"SUCCESS, Is_Alpha");
			return SUCCESS;
		}
		break;
	case 'C':	/* test for symbol constituent */
		if (Is_SyCon(RelExp)) {
			DBGFEX(1,DbgFNm,"SUCCESS, Is_SyCon");
			return SUCCESS;
		}
		break;
	case 'D':	/* test for numeric */
		if (Is_Digit(RelExp)) {
			DBGFEX(1,DbgFNm,"SUCCESS, Is_Digit");
			return SUCCESS;
		}
		break;
	case 'E':	/* test for equal to zero */
	case 'F':	/* test for false */
	case 'U':	/* test for unsuccessful */
	case '=':	/* test for equal to zero */
		if (NArgmt == 0) {
			DBGFEX(1,DbgFNm,"SUCCESS, equal to zero");
			return SUCCESS;
		}
		break;
	case 'G':	/* test for greater than zero */
	case '>':	/* test for greater than zero */
		if (NArgmt > 0) {
			DBGFEX(1,DbgFNm,"SUCCESS, greater than zero");
			return SUCCESS;
		}
		break;
	case 'L':	/* test for less than zero */
	case 'S':	/* test for successful */
	case 'T':	/* test for TRUE */
	case '<':	/* test for less than zero */
		if (NArgmt < 0) {
			DBGFEX(1,DbgFNm,"SUCCESS, less than zero");
			return SUCCESS;
		}
		break;
	case 'N':	/* test for not equal to zero */
		if (NArgmt != 0) {
			DBGFEX(1,DbgFNm,"SUCCESS, not equal to zero");
			return SUCCESS;
		}
		break;
	case 'R':	/* test for alphanumeric */
		if (Is_Alnum(RelExp)) {
			DBGFEX(1,DbgFNm,"SUCCESS, Is_Alnum");
			return SUCCESS;
		}
		break;
	case 'V':	/* test for lowercase */
		if (Is_Lower(RelExp)) {
			DBGFEX(1,DbgFNm,"SUCCESS, Is_Lower");
			return SUCCESS;
		}
		break;
	case 'W':	/* test for uppercase */
		if (Is_Upper(RelExp)) {
			DBGFEX(1,DbgFNm,"SUCCESS, Is_Upper");
			return SUCCESS;
		}
		break;
	default:
		ErrMsg(ERR_IQC);		/* ill. char. after " */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (FlowEE() == FAILURE) {		/* flow to | or ' */
		DBGFEX(1,DbgFNm,"FAILURE, FlowEE() failed");
		return FAILURE;
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS, test failed");
	return SUCCESS;
}
