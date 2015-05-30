/*****************************************************************************
	ExeCrt()
	This function "executes" a ^ (caret) command.  Most TECO commands
which are control characters (^A, ^B, etc) can also be entered as a caret
and letter combination.  For example, control-A can also be entered as
caret-A.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "chmacs.h"		/* define character processing macros */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeCrt()		/* execute a ^ (caret) command */
{
	unsigned char TmpChr;
	static DEFAULT (*FCAray[])(VVOID) = {
		/* ^A*/ ExeCtA,   /* ^B*/ ZExCtB,   /* ^C*/ ExeCtC,
		/* ^D*/ ExeCtD,   /* ^E*/ ExeCtE,   /* ^F*/ ExeNYI,
		/* ^G*/ ExeIll,   /* ^H*/ ZExCtH,   /*TAB*/ ExeCtI,
		/* LF*/ ExeNul,   /* VT*/ ExeIll,   /* FF*/ ExeCtL,
		/* CR*/ ExeNul,   /* ^N*/ ExeCtN,   /* ^O*/ ExeCtO,
		/* ^P*/ ExeCtP,   /* ^Q*/ ExeCtQ,   /* ^R*/ ExeCtR,
		/* ^S*/ ExeCtS,   /* ^T*/ ExeCtT,   /* ^U*/ ExeCtU,
		/* ^V*/ ExeCtV,   /* ^W*/ ExeCtW,   /* ^X*/ ExeCtX,
		/* ^Y*/ ExeCtY,   /* ^Z*/ ExeCtZ,   /* ^[*/ ExeEsc,
		/* ^\*/ ExeIll,   /* ^]*/ ExeIll,   /* ^^*/ ExeCCC,
		/* ^_*/ ExeUsc
	};
	if (IncCBP() == FAILURE) {
		return FAILURE;
	}
	TmpChr = To_Upper(*CBfPtr);
	if ((TmpChr < 'A') || (TmpChr > '_')) {
		ErrChr(ERR_IUC, *CBfPtr);
		return FAILURE;
	}
	return (*FCAray[TmpChr-'A'])();
}
