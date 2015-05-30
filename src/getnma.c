/*****************************************************************************
	GetNmA()
	This function "gets the numeric argument".  When a command which takes
a numeric argument is executed,  it checks whether the expression stack is
empty.  If the expression stack is not empty,  this function is called to get
the numeric argument on the expression stack into global variable Number.
	It is assumed that the calling function checked that the expression
stack is non-empty (i.e. that EStTop > EStBot).  This function checks that
the expression stack contains an operand (not an operator).  If it does,  the
operand is removed from the stack and placed in NArgmt.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT GetNmA()		/* get numeric argument */
{
    DBGFEN(2,"GetNmA",NULL);
    if (EStack[EStTop].ElType == OPERAND) {	/* it's an operand */
	NArgmt = EStack[EStTop].Elemnt;
	--EStTop;
#if DEBUGGING
	sprintf(DbgSBf,"SUCCESS, NArgmt = %ld",NArgmt);
	DbgFEx(2,DbgFNm,DbgSBf);
#endif
	return SUCCESS;
    }
#if DEBUGGING
    sprintf(DbgSBf,"dying with IFE, EstBot = %ld, stack =",(LONG)EStBot);
    DbgFMs(2,DbgFNm,DbgSBf);
    while (EStTop > 0) {
	printf("EStack[%ld].ElType = ", (LONG)EStTop);
	if (EStack[EStTop].ElType == OPERATOR) {
	    printf("OPERATOR, EStack[%ld].Elemnt = '%c'\r\n",
	    (LONG)EStTop, (char)EStack[EStTop].Elemnt);
	} else {
	    printf("OPERAND, EStack[%ld].Elemnt = %ld\r\n",
	    (LONG)EStTop, (LONG)EStack[EStTop].Elemnt);
	}
	EStTop--;
    }
#endif
    ErrMsg(ERR_IFE);			/* ill-formed numeric expression */
    DBGFEX(2,DbgFNm,"FAILURE, unterminated command");
    return FAILURE;
}
