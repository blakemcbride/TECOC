/*****************************************************************************

	Search()

	This function handles most of the special stuff that happens in
search commands (S, N, FS, FN, _, F_, E_, FB and FK).  It

	1. Loads the search buffer (SBfBeg, SBfEnd, SBfPtr) with the text
	   argument or leaves the search buffer alone if there is no
	   text argument.
	2. Sets variable NArgmt with the numeric argument or 1 if there
	   is no numeric argument.
	3. Calls SrcLop to do the basic search loop.
	4. If the search command is colon modified or if the search command
	   is in a loop and the next character after the search command is
	   a semicolon,  then return appropriately.
	5. If successful search,  return successfully but worry about
	   the ES flag.
	6. If unsuccessful search,  return unsuccessfully but worry about
	   whether we need to skip to the end of a loop or not.

	This function is called by ExeEUn, ExeFB, ExeFK, ExeFN, ExeFS, ExeFUn,
ExeN, ExeS and ExeUnd.  Each of those functions sets up the variable SrcTyp
before calling this function to do the search.

*****************************************************************************/

#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
#include "dchars.h"		/* define identifiers for characters */

#if USE_PROTOTYPES
static BOOLEAN SemiNx(void);
#endif

/*****************************************************************************

	SemiNx()

	TECO-C has no "did the last search succeed" flag.  Instead Search
looks ahead at the command string and,  when the command that follows the
search is a semi-colon,  pushes -1 or 0 for that semi-colon to consume.

	The lookahead used to be a single test of the character immediately
after the search command,  which got two things wrong.  ":;" -- documented as
"branch out of the current iteration if the immediately preceding search
SUCCEEDED",  the exact complement of ";" -- begins with a colon,  so nothing
was ever pushed for it and it always reported ?NAS.  And SPACE, CR and LF are
no-ops everywhere else in a command string,  so "S b$ ;" had to be written
without the space to work at all.

	This function steps over those no-ops and recognises both forms.

*****************************************************************************/

static BOOLEAN SemiNx()		/* is the next command ";" or ":;"? */
{
	charptr	cp;

	cp = CBfPtr + 1;		/* character after the search command */

	while ((cp <= CStEnd) &&	/* step over the no-op characters */
	       ((*cp == SPACE) || (*cp == CRETRN) || (*cp == LINEFD))) {
		++cp;
	}
	if (cp > CStEnd) {
		return FALSE;
	}
	if (*cp == ';') {		/* plain ";" */
		return TRUE;
	}
	if (*cp != ':') {
		return FALSE;
	}

	++cp;				/* a colon -- is it modifying a ";"? */
	while ((cp <= CStEnd) &&
	       ((*cp == SPACE) || (*cp == CRETRN) || (*cp == LINEFD))) {
		++cp;
	}

	return (BOOLEAN)((cp <= CStEnd) && (*cp == ';'));
}

DEFAULT Search(RepCmd)		/* main search code */
BOOLEAN	RepCmd;			/* TRUE if the command has two arguments */
{
	DBGFEN(2,"Search",NULL);

/*
 * If there is a search argument,  put it into SBf,  otherwise use the
 * string from the last search command.
 */

/*
 * "Stext$" with an EMPTY text -- that is,  "S$" -- means "search again for the
 * string the last search used",  and the test for it is "the character after
 * the S is an ESCAPE".  But an @-modified command's next character is the
 * DELIMITER THE USER CHOSE,  and nothing stops that delimiter being an ESCAPE:
 * "@S$text$" was taken for "S$",  the previous search string was searched for
 * instead of "text",  and "text$" was left for ExeCSt to read as commands.
 * With ATSIGN set there is always a text argument,  even an empty one.
 */
	if (!(CmdMod & ATSIGN) &&
	    (CBfPtr < CStEnd) && (*(CBfPtr+1) == ESCAPE)) { /* if it's s$, n$ */
		++CBfPtr;			/* skip the escape */
	} else {
		if (BldStr(SBfBeg, SBfEnd, &SBfPtr) == FAILURE) {
			DBGFEX(2,DbgFNm,"FAILURE, BldStr() failed");
			return FAILURE;
		}
	}
/*
 * If the search is one of the "search and replace" commands,  then parse the
 * second argument (the replace string).  We need to do this here because we
 * may later need to check if a semicolon follows the command,  and we need
 * to be positioned at the end of the command to do so.
 */

	if (RepCmd) {				/* if FC, FN, FS or F_ */
/*
 * If the command is @-modified,  then we need to move the command pointer
 * back by one.  The call to Search will have eaten the first text argument
 * of the command,  leaving the pointer on the delimiter between the arguments.
 * We are about to call FindES,  which will call IncCBP to move to the next
 * character.   Moving back by one cancels the IncCBp.
 */

		if (CmdMod & ATSIGN) {		/* if it's at-sign modified */
			--CBfPtr;
		}

		if (FindES(ESCAPE) == FAILURE) {/* find end of 2nd argument */
			DBGFEX(2,DbgFNm,"FAILURE, FindES() failed");
			return FAILURE;
		}
	}

/*
 * The test asks whether there is an OPERAND on top of the stack,  not merely
 * whether the stack is non-empty.  A "(" or a pending "+" leaves an OPERATOR
 * there,  and this command's value-producing form belongs in the middle of an
 * expression:  "1+%A" and "1+:C" used to hand GetNmA an operator and die with
 * ?IFE where "1+QA" has always worked.  ^R, ^T, ED, \ and Qq use this form.
 *	UMinus has to run BEFORE the test:  a bare "-" is on the stack as an
 * OPERATOR,  and it is UMinus that turns the lone one into the operand -1.
 * Testing first made "-C", "-R", "-D", "-A", "-%q" and "-S" all look like
 * partial expressions and take their default of 1.
 */
	UMinus();				/* if it's -S, make it -1S */
	if ((EStTop == EStBot) ||		    /* if no numeric arg or */
	    (EStack[EStTop].ElType != OPERAND)) {   /* partial expression */
		NArgmt = 1;			/* default is 1S */
	} else {
		if (GetNmA() == FAILURE) {	/* get numeric argument */
			DBGFEX(2,DbgFNm,"FAILURE, GetNmA() failed");
			return FAILURE;
		}

/*
 * n is an occurrence count for S, N, _, E_ and friends,  and 0S / 0N / 0_ have
 * no meaning -- but FB and FC also take an m,n PAIR,  and there n is a
 * character POSITION,  for which 0 is the perfectly ordinary value B.  The
 * test used to apply to both forms,  so the natural way to write a bounded
 * search over the front of the buffer,  "10,0FBa$",  was refused with ?ISA.
 * MARGIS can only be set here for FB/FC:  ExeS and the rest reject m,n
 * outright before Search is ever called.  Nothing downstream minds -- DoFBFC
 * takes the direction from MArgmt < NArgmt before GetAra sorts the pair,  and
 * GetAra is happy with m == 0.
 */
		if (!(CmdMod & MARGIS) && (NArgmt == 0)) {
			ErrMsg(ERR_ISA);	/* illegal search argument */
			DBGFEX(2,DbgFNm,"FAILURE, NArgmt == 0");
			return FAILURE;
			}
		}

	if (SrcLop() == FAILURE) {
		DBGFEX(2,DbgFNm,"FAILURE, SrcLop failed");
		return FAILURE;
	}

	if ((CmdMod & COLON) ||			/* if colon modifier or */
	    ((LStTop != LStBot) &&		/* (in loop and         */
	     SemiNx())) {			/*  next cmd = ; or :;) */
		if (Matchd) {			/* if successful search */
			if (EsFlag&& LStTop == LStBot && MStTop < 0) {
			        /* if ES flag is set, not in loop or macro */
				DoEvEs(EsFlag);	/* handle ES flag */
			}
			DBGFEX(2,DbgFNm,"PushEx(-1)");
			return PushEx(-1L, OPERAND);
		} else {			/* else unsuccessful search */
			DBGFEX(2,DbgFNm,"PushEx(0)");
			return PushEx(0L, OPERAND);
		}
	}

	if (Matchd) {				/* if successful search */
		if (EsFlag&& LStTop == LStBot && MStTop < 0) {
			/* if ES flag is set, not in loop and not in macro */
			DoEvEs(EsFlag);		/* handle ES flag */
		}
		DBGFEX(2,DbgFNm,"SUCCESS");
		return SUCCESS;
	} else {				/* else unsuccessful search */
		*SBfPtr = '\0';
/*
 * ?SRH is "A search command not preceded by a colon modifier AND NOT WITHIN AN
 * ITERATION has failed to find the specified text".  Inside an iteration the
 * failure is not an error at all:  the manual lists "%Search fail in iter"
 * among its two non-fatal warnings,  and says that after printing it "the
 * iteration is terminated,  i.e., TECO simulates a 0; command" -- which is
 * exactly what FlowEL does here.  Reporting it with ErrPSt instead printed
 * "?SRH",  which everywhere else in TECO means the command string was
 * abandoned;  it also set LstErr,  so a later ? or / blamed a search that had
 * not failed fatally,  and with EH&4 set TypESt dumped the command string.
 *	%Superseding existing file (ZOpOut) is the other warning of the pair,
 * and is printed the same way.
 */
		if (LStTop == LStBot) {		/* if not in an iteration */
			ErrPSt(ERR_SRH, SBfBeg, SBfPtr);/* "search failure" */
			DBGFEX(2,DbgFNm,"FAILURE");
			return FAILURE;
		}
		ZDspBf("% Search fail in iter\r\n", 23);
		DBGFEX(2,DbgFNm,"FlowEL()");
		return FlowEL();
	}
}
