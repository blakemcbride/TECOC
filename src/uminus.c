/*****************************************************************************
	UMinus()
	This function is called when a command is executed which can have a
numeric argument of "-",  like "-C".  The command "-C" really means "-1C".
This function looks at the expression stack to see if it contains only a
minus-sign operand.  It it does,  it is replaced with a -1 operator.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
VVOID UMinus()			/* turn '-' arg into -1 */
{
	ESptr	ESp;
	if (EStTop == (EStBot + 1)) {		/* if only one guy */
		ESp = &(EStack[EStTop]);
		if (ESp->ElType == OPERATOR &&	/* and it's an operator */
		    ESp->Elemnt == '-') {	/* and it's a minus sign */
			ESp->Elemnt = -1L;	/* then convert to -1 */
			ESp->ElType = OPERAND;
		}
	}
}
