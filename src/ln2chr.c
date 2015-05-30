/*****************************************************************************
	Ln2Chr()
	This function converts an edit buffer offset represented by a line
number into the corresponding character number.  It basically performs the
function of the control-Q command.
	The returned character offset is minimized with the edge of the
buffer.  This means that if there are 10 lines in the edit buffer and this
function is called to compute the character offset of the 100th line,  the
character offset of the 10th line is returned.  Of course,  this happens
whether the argument is positive or negative.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
#include "chmacs.h"		/* define character processing macros */
LONG Ln2Chr(Value)
LONG Value;
{
	charptr	TmpPtr;
	if (Value > 0) {
		TmpPtr = GapEnd;
		while ((TmpPtr < EBfEnd) &&  (Value > 0)) {
			++TmpPtr;
			if (IsEOL(*TmpPtr)) {
				--Value;
			}
		}
		return TmpPtr-GapEnd;
	}
	TmpPtr = GapBeg;
	while ((TmpPtr > EBfBeg) && (Value <= 0)) {
		--TmpPtr;
		if (IsEOL(*TmpPtr)) {
			++Value;
		}
	}
/*
 * at this point (Value > 0 OR TmpPtr == EBfBeg)
 */
	if ((TmpPtr != EBfBeg) || (IsEOL(*TmpPtr))) {
		++TmpPtr;
	}
	return TmpPtr-GapBeg;
}
