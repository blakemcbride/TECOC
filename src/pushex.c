/*****************************************************************************
	PushEx ()
	Reduce ()
	Push an item on the stack and try to reduce it.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
#include "deferr.h"		/* define identifiers for error messages */
/*****************************************************************************
	Reduce()
	This function attempts to reduce the expression stack.  It is called
by PushEx when the stack has two or more values on it and therefore might be
reduceable.
*****************************************************************************/
#if USE_PROTOTYPES
static DEFAULT Reduce(void);
#endif
static DEFAULT Reduce()		/* reduce the expression stack */
{
	ESptr	ESp1;		/* 1st thing on stack (top) */
	ESptr	ESp2;		/* 2nd thing on stack (top-1) */
	ESptr	ESp3;		/* 3rd thing on stack (top-2) */
	WORD	OnStack;	/* number of items on stack */
	LONG	x;		/* temporary operand */
	LONG	y;		/* temporary operand */
	FOREVER {		/* reduce until you can't reduce anymore */
		OnStack = EStTop - EStBot;
		if (OnStack < 0) {	/* nothing on stack */
			break;
		}
		ESp1 = &(EStack[EStTop]);
		ESp2 = ESp1 - 1;
		ESp3 = ESp2 - 1;
		if (OnStack >= 3) {
/*
 * if it's x+y, x-y, x*y, x/y, x&y or x#y, reduce it
 */
			if (ESp1->ElType == OPERAND  &&
			    ESp2->ElType == OPERATOR &&
			    ESp3->ElType == OPERAND  &&
			    ESp2->Elemnt != ')' &&
			    ESp2->Elemnt != '(') {
				x = ESp3->Elemnt;
				y = ESp1->Elemnt;
				switch ((char)ESp2->Elemnt) {
				case '+':	x += y;		break;
				case '-':	x -= y;		break;
				case '*':	x *= y;		break;
				case '/':	x /= y;		break;
				case '&':	x &= y;		break;
				case '#':	x |= y;		break;
				}
				ESp3->Elemnt = x;
/*
 * ESp3->ElType is already OPERAND
 */
				EStTop -= 2;
				continue;
			} else {
/*
 * if it's (x),  reduce it to x
 */
				if (ESp1->ElType == OPERATOR &&
				    ESp2->ElType == OPERAND  &&
				    ESp3->ElType == OPERATOR &&
				    ESp1->Elemnt == ')' &&
				    ESp3->Elemnt == '(') {
					ESp3->Elemnt = ESp2->Elemnt;
					ESp3->ElType = OPERAND;
					EStTop -= 2;
					continue;
				}
			}
		}
		if (OnStack >= 2) {
/*
 * if it's +x or -x, reduce it to + or - x
 */
			if (ESp1->ElType == OPERAND  &&
			    ESp2->ElType == OPERATOR &&
			    (ESp2->Elemnt == '+' || ESp2->Elemnt == '-')) {
				x = ESp1->Elemnt;
				if (ESp2->Elemnt == '-') {
					x = -x;
				}
				ESp2->Elemnt = x;
				ESp2->ElType = OPERAND;
				--EStTop;
				continue;
			}
		}
		if (OnStack >= 1) {
/*
 * check for one's complement operator ^_.  This operator complements the
 * PRECEDING argument.
 */
			if (ESp1->ElType == OPERATOR &&
			    ESp1->Elemnt == USCHAR) {
				if (OnStack == 1 || ESp2->ElType != OPERAND) {
					ErrMsg(ERR_NAB); /* no arg before ^_ */
					return FAILURE;
				}
				ESp2->Elemnt = ~(ESp2->Elemnt);
/*
 * ESp2->ElType is already OPERAND
 */
				--EStTop;
				continue;
			}
		}
		break;		/* can't reduce anymore */
	}
	return (SUCCESS);
}
/*****************************************************************************
	PushEx()
	This function pushes an item onto the expression stack.  The
expression stack implement's TECO's expression handling capability.  For
instance,  if a command like 10+qa=$ is executed,  then three values are
pushed onto the expression stack: 10, plus sign and the value of qa.  Each
time a value is pushed onto the expression stack,  the Reduce function is
called to see if the stack can be reduced.  In the above example,  Reduce
would cause the stack to be reduced when the value of qa is pushed,  because
the expression can be evaluated at that time.
*****************************************************************************/
DEFAULT PushEx(Item, EType)	/* push onto expression stack */
LONG Item;
DEFAULT EType;
{
#if DEBUGGING
	static char *DbgFNm = "PushEx";
	if (EType == OPERAND) {
		sprintf(DbgSBf,"Item = %ld, EType = OPERAND", (LONG)Item);
	} else if (EType == OPERATOR) {
		sprintf(DbgSBf,"Item = '%c', EType = OPERATOR", (char)Item);
	} else {
		DbgFEn(3,DbgFNm,"FAILURE, illegal item type");
		TAbort(EXIT_FAILURE);
	}
	DbgFEn(3,DbgFNm,DbgSBf);
#endif
	if (++EStTop >= EXS_SIZE) {		/* if expression stack full */
		ErrMsg(ERR_PDO);		/* push-down list overflow */
		DBGFEX(3,DbgFNm,"FAILURE, expression stack full");
		return FAILURE;
	}
	EStack[EStTop].Elemnt = Item;		/* put item on stack */
	EStack[EStTop].ElType = EType;		/* put item type on stack */
	if (EStTop > (EStBot + 1)) {		/* if stack might reduce */
		if (Reduce() == FAILURE) {	/* reduce expression stack */
			DBGFEX(3,DbgFNm,"FAILURE, Reduce() failed");
			return FAILURE;
		}
	}
#if DEBUGGING
	sprintf(DbgSBf,"EStTop = %ld, EStBot = %ld", (LONG)EStTop, (LONG)EStBot);
	DbgFEx(3,DbgFNm,DbgSBf);
#endif
	return SUCCESS;
}
