/*****************************************************************************
	ExeOpr()
	This function executes an operator command.
	+	Addition
	-	Subtraction or negation
	(	Expression grouping (ExeRtP() executes ')')
	#	Logical OR
	/	Division
	&	Logical AND
	^_	Ones complement (logical NOT)
	*	Multiplication
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
DEFAULT ExeOpr()		/* operators: + - * / # & ^_ */
{
	DBGFEN(1,"ExeOpr",NULL);
	if (PushEx((LONG)*CBfPtr, OPERATOR) == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
