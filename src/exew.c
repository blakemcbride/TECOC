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
			NArgmt==2 ? VtSize :
			NArgmt==3 ? SeeAll :
			NArgmt==4 ? MrkFlg :
			NArgmt==5 ? HldFlg :
			NArgmt==6 ? TopDot :
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
		CrType = (DEFAULT)MArgmt;
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
	    VtSize = (DEFAULT)MArgmt;
	    break;
	case 3:
	    SeeAll = (DEFAULT)MArgmt;
	    break;
	case 4:
	    MrkFlg = (DEFAULT)MArgmt;
	    break;
	case 5:
	    HldFlg = (DEFAULT)MArgmt;
	    break;
	case 6:
#if CURSES
	    scope_start = (int)MArgmt;
	    p_scope_start = (scope_start > GapBeg - EBfBeg)
				? (scope_start - (GapBeg - EBfBeg)) + GapEnd
				: EBfBeg + scope_start;
#else
	    TopDot = (DEFAULT)MArgmt;
#endif
	    break;
	case 7:
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
		CmdMod = '\0';
		DBGFEX(1,DbgFNm,"SUCCESS");
		return SUCCESS;
	}

#if VIDEO

	if (EStTop == EStBot) {			/* if no numeric argument */
		NArgmt = (CmdMod & COLON) ? 0 : 16;
	} else {
		if (GetNmA() == FAILURE) {	/* get the numeric argument */
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
	}

	if (CmdMod & COLON) {			/* if it's :W */
		if (NArgmt < 0) {		/* insert processing? */
			CmdMod = '\0';
			return ExeNYI();
		} else {
			if (CmdMod & MARGIS) {	/* if it's m,n:W */
				CmdMod = '\0';
				if (VSetW() == FAILURE) {
					DBGFEX(1,DbgFNm,"FAILURE");
					return FAILURE;
				}
			} else {		/* else (must be n:W) */
				CmdMod = '\0';
				if (VGetW() == FAILURE) {
					DBGFEX(1,DbgFNm,"FAILURE");
					return FAILURE;
				}
			}
		}
	} else {				/* else there's no colon */
		CmdMod = '\0';
		if (VDoW() == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
		EStTop = EStBot;		/* clear expression stack */
		CmdMod &= ~COLON;		/* clear colon modifier */
	}

	DBGFEX(1,DbgFNm,"SUCCESS");
	CmdMod = '\0';
	return SUCCESS;

#else /* not VIDEO */

	CmdMod = '\0';
	DBGFEX(1,DbgFNm,"ExeNYI()");
	return ExeNYI();

#endif /* VIDEO */
}
