/*****************************************************************************

	ExeW()

	This function executes a W command.

	W	Scope "WATCH"
	n:W	Return scope characteristics
	m,n:W	Set scope characteristics

	TECO-C implements only the part of the W command relating to the
terminal type.

*****************************************************************************/

#if CURSES
#if ULTRIX
#include <cursesX.h>		/* has to come before zport.h */
#else
#include <curses.h>		/* has to come before zport.h */
#endif
#endif

#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "deferr.h"		/* identifiers for error messages */
#include "defext.h"		/* define external global variables */

#if VIDEO

#if CURSES
extern int scope_start;
extern unsigned char *p_scope_start;
extern int skiprefresh;
#endif

#if USE_PROTOTYPES
static DEFAULT VGetW(void);
static DEFAULT VSetW(void);
static DEFAULT VDoW(void);
#endif

#include "dscren.h"		/* define screen identifiers */
#include <string.h>		/* prototype for strlen() */

static DEFAULT VGetW()		/* execute an n:W command */
{
    DBGFEN(1,"VGetW",NULL);
    DBGFEX(1,DbgFNm,"PushEx()");
    return PushEx(	NArgmt==0 ? CrType :
			NArgmt==1 ? HtSize :
/*
 * 2:W is the height of the SCOPE,  not the physical height of the terminal.
 * The manual:  "Whenever the scrolling region's size is modified (that is,
 * whenever an m,7:W command is executed),  TECO alters the scope's height
 * (2:W) accordingly.  For example,  if 2:W is currently returning a value of
 * 24,  then after a 5,7:W command 2:W will return a value of 19.  Executing a
 * 0,7:W will restore 2:W to 24."  This used to answer the full screen height
 * whatever ScroLn was,  so a macro that sized its own display from 2:W after
 * turning scrolling on -- the obvious way to write one -- drew ScroLn lines
 * too many.
 *
 * VtSize itself stays the PHYSICAL height:  every "VtSize - ScroLn - 1 -
 * winline" in redraw(), Scope() and initialize_scope() depends on that, and
 * so does case 7's own bounds test below.  Only what 2:W reports and what
 * m,2:W stores change,  and the two stay each other's inverse.
 */
			NArgmt==2 ? VtSize - ScroLn :
			NArgmt==3 ? SeeAll :
			NArgmt==4 ? MrkFlg :
			NArgmt==5 ? HldFlg :
/*
 * VSetW's "case 6" writes scope_start under CURSES and TopDot only in the
 * non-CURSES build,  and redraw() maintains scope_start too;  nothing in a
 * CURSES build ever assigns TopDot,  so reading it here made 6:W answer 0
 * forever and a macro that saved the window position with 6:W and restored it
 * with m,6:W scrolled to the top of the buffer instead.
 */
#if CURSES
			NArgmt==6 ? (DEFAULT)scope_start :
#else
			NArgmt==6 ? TopDot :
#endif
			NArgmt==7 ? ScroLn :
#if CURSES
		        NArgmt==8 ? SpcMrk :
		        NArgmt==9 ? KeyPad :
#endif
		                             0 , OPERAND);
}



static DEFAULT VSetW()		/* execute an m,n:W command */
{
    DBGFEN(1,"VSetW",NULL);

/*
 * Use an if here instead of a "default" case because doing a switch will
 * cast the variable to an "int",  causing Turbo-C to warn about types.
 */

    if ((NArgmt < 0) || (NArgmt > 9)) {
	return ExeNYI();
    }

    switch ((int)NArgmt) {
	case 0:
	    if ((MArgmt < 0) || (MArgmt > 13) || (MArgmt == 3) ||
						 (MArgmt == 5) ||
						 (MArgmt == 7) ||
						 (MArgmt == 9)) {
		ZDspBf("Illegal terminal type.", 22);
		ZDspBf("  Terminal types are:\r\n", 23);
		ZDspBf("\t0\t= VT52\r\n", 11);
		ZDspBf("\t1\t= VT61\r\n", 11);
		ZDspBf("\t2\t= VT100 in VT52 mode\r\n", 25);
		ZDspBf("\t4\t= VT100 in VT100 mode\r\n", 26);
		ZDspBf("\t6\t= VT05\r\n", 11);
		ZDspBf("\t8\t= VT102\r\n", 12);
		ZDspBf("\t10\t= VK100\r\n", 13);
		ZDspBf("\t11\t= VT200 in VT200 mode\r\n", 27);
		ZDspBf("\t12\t= VT200 in ANSI (VT100) mode\r\n", 34);
		ZDspBf("\t13\t= VT200 in VT52 mode\r\n", 26);
/*
 * Do NOT store the rejected value:  this arm reports failure,  and CrType is
 * what ZScrOp (and,  on VMS,  a table lookup) uses to decide how to drive the
 * terminal.
 *
 * The list above IS the report -- there is no Err* call and so no LstErr --
 * but LstErr was left holding the PREVIOUS error,  so a following "/" or "?"
 * explained a command that had nothing to do with this one.  ERR_XXX is the
 * "no error outstanding" value ReadCS tests for,  which makes both immediate
 * commands do nothing rather than the wrong thing.
 */
		LstErr = ERR_XXX;	/* the list above was the message */
		return FAILURE;
	    }
	    CrType = (DEFAULT)MArgmt;
	    break;
	case 1:
	    if (MArgmt < 1) {
		ErrMsg(ERR_ARG);
		return FAILURE;
	    }
	    if (ZSetTT(TTWIDTH, (DEFAULT)MArgmt) == FAILURE)
		return FAILURE;
	    HtSize = (DEFAULT)MArgmt;
	    break;
	case 2:
	    if (MArgmt < 1) {
		ErrMsg(ERR_ARG);
		return FAILURE;
	    }
	    if (ZSetTT(TTHEIGHT, (DEFAULT)MArgmt) == FAILURE)
		return FAILURE;
/*
 * MArgmt is the scope height (see VGetW),  so the physical height it implies
 * is that plus the command window.  With ScroLn 0 -- the state m,2:W is
 * normally used in -- this is the plain assignment it always was.
 */
	    VtSize = (DEFAULT)MArgmt + ScroLn;
	    break;
	case 3:
	    SeeAll = (DEFAULT)MArgmt;
	    break;
	case 4:
/*
 * "Return or set 'mark' status of window support.  A value of n means that a
 * mark has been set at buffer ('dot') position n-1.  This status is used by
 * software that uses the window support and by the support itself in the case
 * of scopes that support reverse video."
 *	TECOC stores the value and hands it back through VGetW and there the
 * matter ends:  redraw() does not look at MrkFlg,  so no mark is ever shown.
 * lib/squ.tes:927's ".,4:W" sets one that is never drawn.  A macro can still
 * use 4:W as a scratch cell -- which is all it has ever been here -- but do not
 * expect the display to reflect it.  Implementing it means teaching redraw() to
 * run standout() over the marked position,  not changing this line.
 */
	    MrkFlg = (DEFAULT)MArgmt;
	    break;
	case 5:
	    HldFlg = (DEFAULT)MArgmt;
	    break;
	case 6:
#if CURSES
/*
 * m is a character position in the edit buffer,  and p_scope_start is a
 * pointer built from it that redraw() then dereferences.  Without this test
 * any value at all could be turned into a pointer to anywhere.
 */
	    if ((MArgmt < 0) ||
		(MArgmt > ((GapBeg-EBfBeg) + (EBfEnd-GapEnd)))) {
		ErrMsg(ERR_ARG);
		return FAILURE;
	    }
	    scope_start = (int)MArgmt;
	    p_scope_start = (scope_start > GapBeg - EBfBeg)
				? (scope_start - (GapBeg - EBfBeg)) + GapEnd
				: EBfBeg + scope_start;
#else
	    TopDot = (DEFAULT)MArgmt;
#endif
	    break;
	case 7:
#if CURSES
/*
 * m is the number of command lines to reserve at the bottom of the screen.
 * It has to leave at least one line for the text window,  or the
 * "VtSize - ScroLn - 1 - winline" arithmetic all through redraw() goes
 * negative and setscrreg() is handed a bogus region.
 */
/*
 * ...and when EZ_WIN_LINE is set the separator line costs one more,  so the
 * text window is VtSize-ScroLn-1-winline lines and the largest legal ScroLn is
 * one smaller again.  Scope() has not run yet,  so "winline" itself still
 * holds the value from the LAST time the scope was entered;  take the count
 * from EzFlag,  which is what Scope() is about to do.
 */
	    if ((MArgmt < 0) ||
		(MArgmt >= VtSize - ((EzFlag & EZ_WIN_LINE) ? 1 : 0))) {
		ErrMsg(ERR_ARG);
		return FAILURE;
	    }
#endif
	    ScroLn = (DEFAULT)MArgmt;
#if CURSES
	    Scope(ScroLn);
#endif
	    break;
#if CURSES
	case 8:
	    SpcMrk = (DEFAULT)MArgmt;
	    break;
	case 9:
	    KeyPad = (DEFAULT)MArgmt;
	    break;
#endif
    } /* end of switch */

    DBGFEX(1,DbgFNm,"SUCCESS");
    return (PushEx(MArgmt, OPERAND));
}


static DEFAULT VDoW()		/* execute an nW command */
{
    DBGFEN(1,"VDoW",NULL);

#if CURSES
    if (NArgmt== (-4)) {
	redraw();
	wrefresh(curscr);    
	return SUCCESS;
    } else if (NArgmt==0 || NArgmt == 16) {
	redraw();
	refresh();
	return SUCCESS;
    } else if (NArgmt == -1) {
	skiprefresh = 1;
	return SUCCESS;
    } else if (NArgmt == 1) {
	skiprefresh = 0;
	return SUCCESS;
    } else if (NArgmt == 2) {	/* put line containing dot at top */
	p_scope_start=GapBeg;
	scope_start=GapBeg - EBfBeg;
	redraw();
	refresh();
        return SUCCESS;
    } else
	return SUCCESS;
#else  /* if CURSES */
#if DEBUGGING
    if (NArgmt == -1) {
	printf("VDoW: -1W refreshing the screen\r\n");
    } else if (NArgmt == 0) {
	printf("VDoW: 0W Placing default cursor line at 16. Forgetting.\r\n");
    } else if (NArgmt == -1000) {
	printf("VDoW: -1000W Forgetting output was done.\r\n");
    } else if (NArgmt < 0) {
	printf("VDoW: %ldW telling refresher top %ld lines are altered.\r\n",
		NArgmt, -NArgmt-1);
    } else {
	printf("VDoW: %ldW Placing def. cursor at line %ld. Forgetting.\r\n",
		NArgmt, NArgmt);
    }

    DBGFEX(1,DbgFNm,"SUCCESS");
    return SUCCESS;
#else
    return ExeNYI();
#endif /* DEBUGGING */
#endif /* CURSES */
}
#endif /* VIDEO */



DEFAULT ExeW()			/* execute an W command */
{
	DBGFEN(1,"ExeW",NULL);

/*
 * If the terminal is not a scope,  don't do anything.  Don't return
 * FAILURE,  because that'll terminate the current macro.
 */
	if (!(EtFlag & ET_WAT_SCOPE)) {		/* if scope not available */
		BOOLEAN ColonMod;
/*
 * The arguments were pushed for this command,  so this command has to take
 * them,  exactly as the VIDEO path below does with GetNmA().  Leaving them on
 * the expression stack made "5W" hand its 5 to whatever command came next.
 *	But "n:W" and "m,n:W" RETURN a value -- "Sets the entity represented by
 * n:W to m and returns a value ... Note that each m,n:W command returns a
 * value,  even if your only intent is to set something." -- so consuming the
 * arguments and pushing nothing left the next command with an empty stack:
 * the documented "0:W" answered ?NAE instead of a terminal type.  That is
 * every non-video build,  which is every Windows build.  Nothing was set and
 * there is no scope to report on,  so the value is 0,  which is what VGetW
 * below answers for any code it does not know.  A plain "W" or "nW" produces
 * nothing and goes on producing nothing.
 */
		ColonMod = (CmdMod & COLON) != 0;
		EStTop = EStBot;		/* consume our arguments */
		CmdMod = ColonMod ? (CmdMod & ATSIGN) : '\0';
		if (ColonMod) {
			DBGFEX(1,DbgFNm,"PushEx(0)");
			return PushEx(0L, OPERAND);
		}
		DBGFEX(1,DbgFNm,"SUCCESS");
		return SUCCESS;
	}

#if VIDEO

/*
 * The test asks whether there is an OPERAND on top of the stack,  not merely
 * whether the stack is non-empty.  A "(" or a pending "+" leaves an OPERATOR
 * there,  and this command's value-producing form belongs in the middle of an
 * expression:  "1+%A" and "1+:C" used to hand GetNmA an operator and die with
 * ?IFE where "1+QA" has always worked.  ^R, ^T, ED, \ and Qq use this form.
 */
	if ((EStTop == EStBot) ||		    /* if no numeric arg or */
	    (EStack[EStTop].ElType != OPERAND)) {   /* partial expression */
		NArgmt = (CmdMod & COLON) ? 0 : 16;
	} else {
		if (GetNmA() == FAILURE) {	/* get the numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}

	if (CmdMod & COLON) {			/* if it's :W */
		BOOLEAN SetForm;
		if (NArgmt < 0) {		/* insert processing? */
			CmdMod = '\0';
			return ExeNYI();
		}
		SetForm = (CmdMod & MARGIS) != 0;    /* m,n:W rather than n:W */
/*
 * Both arms end in a PushEx,  so both are VALUE PRODUCERS and have to leave
 * ATSIGN alone:  ":W" has no text argument for an "@" to modify,  so an at-sign
 * written in front of one belongs to the command that takes the result.  MARGIS
 * is different here -- "m,n:W" is the SET form and has just consumed the pair
 * itself -- and the colon is this command's own.  See DoFlag and the commentary
 * on FSAray in skpcmd.c.
 */
		CmdMod &= ATSIGN;
		if (SetForm) {			/* if it's m,n:W */
			DBGFEX(1,DbgFNm,"VSetW()");
			return VSetW();
		}
		DBGFEX(1,DbgFNm,"VGetW()");
		return VGetW();			/* else (must be n:W) */
	}

	CmdMod = '\0';				/* else there's no colon */
	if (VDoW() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	EStTop = EStBot;			/* clear expression stack */

	DBGFEX(1,DbgFNm,"SUCCESS");
	CmdMod = '\0';
	return SUCCESS;

#else /* not VIDEO */

	CmdMod = '\0';
	DBGFEX(1,DbgFNm,"ExeNYI()");
	return ExeNYI();

#endif /* VIDEO */
}
