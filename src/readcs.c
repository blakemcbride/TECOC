/*****************************************************************************
	ReadCS()
	Read command string.
*****************************************************************************/
#if CURSES
#if ULTRIX
#include <curses.h>		/* has to come before zport.h */
#else
#include <curses.h>		/* has to come before zport.h */
#endif
#endif
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
#include "chmacs.h"		/* define character processing macros */
#include "deferr.h"		/* define identifiers for error messages */
#include "dscren.h"		/* define identifiers for screen i/o */
#if USE_PROTOTYPES
static VVOID InpDel(void);
static unsigned char FirstChar(void);
static BOOLEAN ChkHlp(charptr HlpEnd);
#endif
/*****************************************************************************
	CRCnt maintains a count of the number of carriage returns that have
been typed by the user while entering a command string.  It is used to
recognize when the user executes a HELP command,  which is the only TECO
command that does not need to be terminated by double escapes.  When the very
first carriage return in the user's command string is struck,  TECO checks
if the text preceding the carriage return is a valid HELP command.  This
checking is only performed when the first carriage return is struck.
*****************************************************************************/
static LONG CRCnt;		/* carriage-return count */
static charptr Prompt = (charptr)"\n\r*";
#if CURSES
extern int skiprefresh;
#endif
/*****************************************************************************
	InpDel()
	This function is called when the user types the DELETE key while
typing in a command string.  What is displayed on his terminal when this
happens depends on what kind of terminal he has and what the deleted
character is.  This function displays the required character(s).
*****************************************************************************/
static VVOID InpDel()
{
    int i;
    charptr TmpPtr;				/* temporary pointer */
    DBGFEN(4, "InpDel", NULL);
    --CBfPtr;					/* back up over the DEL */
    if (CBfPtr < CBfBeg) {			/* if cmd buffer is empty */
	ZDspBf(Prompt, 3);			/* prompt */
	DBGFEX(4, DbgFNm, NULL);
	return;
    }
    if (EtFlag & ET_SCOPE) {			/* if it's a scope */
	if (((*CBfPtr > USCHAR) &&		/* if it was ... */
	     (*CBfPtr < DELETE)) ||		/* ... printable or ... */
	     (*CBfPtr == ESCAPE) ||		/* ... escape or ... */
	     (*CBfPtr & '\200')) {		/* ....eighth bit is on */
	    ZDspBf("\b \b", 3);			/* erase one character */
	} else {
	    switch (*CBfPtr) {
		case LINEFD:
		    ZScrOp(SCR_CUP);
		    /* fall through */
		case DELETE:
		    break;
		case VRTTAB:
		case FORMFD:
		    for (i = 1; i <= FFLINS; i++)
			ZScrOp(SCR_CUP);
		    /* fall through */
		case TABCHR:
		case BAKSPC:
		case CRETRN:
		    ZDspCh(CRETRN);
		    ZScrOp(SCR_EEL);
		    if (*CBfPtr == CRETRN) {
			--CRCnt;
		    }
		    TmpPtr = CBfPtr;
		    while (TmpPtr > CBfBeg) {
			TmpPtr--;
			if (IsEOL(*TmpPtr)) {
			    ++TmpPtr;
			    break;
			}
		    }
		    if (TmpPtr == CBfBeg) {
			ZDspCh('*');
		    }
		    TypBuf(TmpPtr, CBfPtr);
		    break;
		default:
		    ZDspBf("\b \b\b \b", 6);
		    break;
	    } /* end of switch */
	}
    } else {					/* else (it's not a scope) */
	EchoIt(*CBfPtr);
    }
    --CBfPtr;					/* actually delete the char */
    DBGFEX(4,DbgFNm,NULL);
}
/*****************************************************************************
	FirstChar()
	This function deals with the following immediate mode commands:
	?	display previous command string up to erroneous command
	/	display verbose explanation of last error
	*q	copy last command string to q-register q
	This function is called by ReadCS to read the very first command
typed by the user.  It handles the above immediate mode commands without
destroying the previous command buffer.  Once this function returns,  the
previous command string and error are considered lost by ReadCS,  which then
overwrites the old command string with a new one.
	This function does not deal with the LF and BS immediate mode commands
because of the following possible sequence of events: the user starts typing
a command string,  then does a ^U to delete it,  then tries to use LF or BS
to position himself in the edit buffer.  If this function were responsible for
LF and BS,  then they would not work.  ReadCS is responsible for handling the
LF and BS immediate mode commands.
*****************************************************************************/
static unsigned char FirstChar()
{
    BOOLEAN		ImdCmd;		/* TRUE if it's '?' or '*' or '/' */
    unsigned char	LclStr[2];	/* local string for * processing */
    DEFAULT		Status;		/* return value for FindQR() */
    charptr		TCBfPt;		/* temporary holder of CBfPtr */
    charptr		TCStEn;		/* temporary holder of CStEnd */
    DEFAULT		TmpChr;		/* temporary character */
    ptrdiff_t		TmpSiz;
#if CURSES
    chtype 		curses_char;
    int 		yy;
    int			xx;
#endif
    do {
	if (EvFlag) {			/* if EV (edit verify) flag is set */
	    DoEvEs(EvFlag);		/* handle EV flag */
	}
#if CURSES
	if (ScroLn > 0 && !skiprefresh)
	    redraw();
#endif
	ZDspCh('*');			/* prompt */
#if CURSES
/*
 * This works on a Sun but may be somewhat system dependent - depending on
 * the type of chtype
 */
	ccs();
	keypad_on();
l1:	getyx(stdscr,yy,xx); 
	curses_char = ZChIn(FALSE); 
	TmpChr = curses_char;
	if (curses_char == KEY_RIGHT) {
	    do_right();
	    move(yy,xx);
	    goto l1;
	} else if (curses_char == KEY_LEFT) {
	    do_left();
	    move(yy,xx);
	    goto l1;
	} else if (curses_char == KEY_DOWN || curses_char == KEY_B2) {
	    do_down();	
	    move(yy,xx); 
	    goto l1;
	} else if (curses_char == KEY_UP) {
	    do_up();
	    move(yy,xx);
	    goto l1;
	} else if ((curses_char == KEY_F(1)) || (curses_char == KEY_ENTER)) {
	    do_seetog();
	    move(yy,xx);
	    goto l1;
	} else if ((curses_char == KEY_SF) || (curses_char == KEY_NPAGE)) {
	    do_sf();
	    move(yy,xx);
	    goto l1;
	} else if ((curses_char == KEY_SR) || (curses_char == KEY_PPAGE)) {
	    do_sr();
	    move(yy,xx);
	    goto l1;
	} else if (curses_char >= 0400) {
	    ZDspCh('?');
	    move(yy,xx);
	    goto l1;
	}
	keypad_off();
#else
	TmpChr = ZChIn(FALSE);		/* get a character */
#endif
	ImdCmd = FALSE;			/* assume not an immediate command */
	switch (TmpChr) {
	    case DELETE:
	    case CTRL_U:
		ZDspCh(CRETRN);
		ImdCmd = TRUE;
		break;
	    case '/':			/* display verbose error message */
		if (LstErr != ERR_XXX) {
		    ZDspCh('/');
		    ErrVrb(LstErr);
		    ImdCmd = TRUE;
		}
		break;
	    case '?':			/* display erroneous command string */
		if (LstErr != ERR_XXX) {  /* if there was an error */
		    ZDspCh('?');
		    TypESt();
		    ImdCmd = TRUE;
		}
		break;
	    case '*':			/* store last cmd string in q-reg */
		ZDspCh('*');		/* echo the "*" */
		TmpChr = ZChIn(FALSE);	/* get a q-register name */
		if (GotCtC) {		/* user typed control-C? */
		    GotCtC = FALSE;
		    return CTRL_C;
		}
		switch (TmpChr) {
		    case CRETRN:
			TmpChr = 0;
			break;
		    case DELETE:
		    case CTRL_U:
		    case BAKSPC:
			ZDspCh(CRETRN);
			break;
		    default:
			EchoIt((unsigned char)TmpChr);
			TCBfPt = CBfPtr;		/* save CBfPtr */
			TCStEn = CStEnd;		/* save CStEnd */
			LclStr[0] = (unsigned char)TmpChr;
			CBfPtr = &LclStr[0];
			if (TmpChr == (DEFAULT)'.') {
			    LclStr[1] = (unsigned char)ZChIn(FALSE);
			    EchoIt((unsigned char)TmpChr);
			    CStEnd = &LclStr[1];
			} else {
			    CStEnd = &LclStr[0];
			}
			Status = FindQR();		/* find q-register */
			CBfPtr = TCBfPt;		/* restore CBfPtr */
			CStEnd = TCStEn;		/* restore CStEnd */
			if (Status == SUCCESS) {
			    if (QR->Start != NULL) {
				ZFree((charptr)QR->Start);
				QR->Start = QR->End_P1 = NULL;
			    }
			    TmpSiz = (CStEnd-CStBeg) + 1;
			    if (TmpSiz > 0) {
				if (MakRom((SIZE_T)TmpSiz) != FAILURE) {
				    MEMMOVE(QR->Start,CStBeg,(SIZE_T)TmpSiz);
				    QR->End_P1 = QR->Start + TmpSiz;
				}
			    }
			    ZDspBf("\r\n", 2);
			}
		} /* end switch */
		ImdCmd = TRUE;
		break;
	} /* end switch */
    } while (ImdCmd);
    return (unsigned char)TmpChr;
}
/*****************************************************************************
	ChkHlp()
	This function checks if the command string buffer contains a HELP
command.  If it does,  the help command is processed and a TRUE is returned.
If the command buffer does not contain a HELP command,  a FALSE is returned.
A HELP command must be the only command in the command string.  It takes
the form
	HELP [/LIBRARY=[=]name] [key-1] [key-2] ...
*****************************************************************************/
static BOOLEAN ChkHlp(HlpEnd)		/* check for HELP command */
charptr HlpEnd;				/* end of help command */
{
    DBGFEN(3, "ChkHlp", NULL);
    if ((*CBfBeg     != 'H' && *CBfBeg     != 'h') ||
	(*(CBfBeg+1) != 'E' && *(CBfBeg+1) != 'e') ||
	(*(CBfBeg+2) != 'L' && *(CBfBeg+2) != 'l') ||
	(*(CBfBeg+3) != 'P' && *(CBfBeg+3) != 'p')) {
	DBGFEX(3, DbgFNm, "FALSE");
	return FALSE;
    }
    if ((*(CBfBeg+4) != '/') &&
	(*(CBfBeg+5) != 'S') && (*(CBfBeg+5) != 's')) {
	ZHelp(CBfBeg+4, HlpEnd-1, FALSE, TRUE);
    } else {
	ZHelp(CBfBeg+6, HlpEnd-1, TRUE, TRUE);
    }
    DBGFEX(3, DbgFNm, "TRUE");
    return TRUE;
}
/*****************************************************************************
	ReadCS()
This function reads a command string from the terminal.  It returns to it's
caller when the command buffer pointed to by CBfBeg contains a command string.
In addition to echoing the characters typed by the user,  this function
handles the following special things:
	1. DEL		delete last character
	2. ^G*		retype command string
	3. ^G<SP>	retype command line
	4. ^G^G		delete command string
	5. ^U		delete command line
	6. ^Z^Z^Z	exit TECO-C
	7. LF		immediate mode: do a 1L1T command
	8. BS		immediate mode: do a -1L1T
	9. *q		immediate mode: store last command string in q
	10. HELP	immediate mode: access HELP subsystem
	11. /		immediate mode: display explanation of last error
	12. ?		immediate mode: display erroneous command string
*****************************************************************************/
#define PRV_CTC		1	/* previous character was control-C */
#define PRV_CTG		2	/* previous character was control-G */
#define PRV_DEF		3	/* previous character wasn't ^Z, ^G or ESC */
#define PRV_ESC		4	/* previous character was escape */
#define PRV_Z1		5	/* previous character was one control-Z */
#define PRV_Z2		6	/* previous character was two control-Z */
VVOID ReadCS()
{
    BOOLEAN         BadSeq;	/* bad escape sequence indicator */
    unsigned char   ctrstr[3];	/* temp buffer to display ^chars as string */
    LONG            HowFar;
    int             PrvChr;	/* previous character flags */
    unsigned char   TmpChr;	/* temporary character */
    charptr         TmpPtr;	/* temporary pointer */
    int             i;
    ctrstr[0] = '^';		/* caret */
    ctrstr[2] = '\0';		/* end-of-string */
    CRCnt = 0;			/* count of carriage-returns */
    PrvChr = PRV_DEF;		/* no previous characters */
    TmpChr = FirstChar();	/* get first character */
    CBfPtr = CBfBeg;		/* make command buffer empty */
    *CBfPtr = TmpChr;
    LstErr = ERR_XXX;		/* last error message is undefined */
    FOREVER {
	if (*CBfPtr == ESCAPE) {
	    if (EtFlag & ET_VT200) {		/* if vt200 mode */
		*CBfPtr = (unsigned char)ZChIn(FALSE);
		BadSeq = FALSE;			/* initialize BadSeq */
		if (*CBfPtr != '[') {		/* if not ESC sequence */
		    BadSeq = TRUE;
		} else {
		    *CBfPtr = (unsigned char)ZChIn(FALSE);
		    switch (*CBfPtr) {
			case '2':		/* f9 - f16 */
			    *CBfPtr = (unsigned char)ZChIn(FALSE);
			    if (*CBfPtr == '4') {
				TmpChr = (unsigned char)ZChIn(FALSE);
				if (TmpChr != '~') {
				    BadSeq = TRUE;
				} else {
				    *CBfPtr = BAKSPC;
				}
			    } else if (*CBfPtr == '5') {
				TmpChr = (unsigned char)ZChIn(FALSE);
				if (TmpChr != '~') {
				    BadSeq = TRUE;
				} else {
				    *CBfPtr = LINEFD;
				}
			    } else {
				BadSeq = TRUE;
			    }
			    break;
#if FALSE
			case 'A':		/* up arrow */
			case 'B':		/* down arrow */
			case 'C':		/* right arrow */
			case 'D':		/* left arrow */
#endif
			default:
			    BadSeq = TRUE;
		    }			/* end of switch */
		}				/* */
		if (BadSeq) {
    ZDspBf("\r\n\n\tInvalid escape sequence.\r\n\n", 31);
    ZDspBf("\tThe 16384 bit of the ET flag is set,  which means\r\n", 52);
    ZDspBf("\tthat you are in VT200 mode.  In this mode,  the\r\n", 50);
    ZDspBf("\tescape character is not used to terminate commands.\r\n", 54);
    ZDspBf("\tIt is used to introduce escape sequences.  This\r\n", 50);
    ZDspBf("\tallows the function keys to take on meanings.  The\r\n", 53);
    ZDspBf("\taccent grave (~) character is the command terminator.\r\n", 56);
    ZDspBf("\tIf you want to turn off VT200 mode,  say 16384,0ET``\r\n", 55);
    ZDspBf("\tNote that the recognition of accent grave as a\r\n", 49);
    ZDspBf("\tcommand terminator is controlled by the 8192 bit\r\n", 51);
    ZDspBf("\tof the ET flag,  separate from the VT200 bit.\r\n\n", 49);
    ZDspBf("\tThere may be a part of the unrecognized escape\r\n", 49);
    ZDspBf("\tsequence in the command string.  The last line of\r\n", 52);
    ZDspBf("\tthe command string is shown to help you recover.\r\n", 51);
		    PrvChr = PRV_CTG;
		    CBfPtr++;
		    *CBfPtr = ' ';
		}				/* end if BadSeq) */
	    } else {				/* else not vt200 mode */
		ZDspCh('$');
		if (PrvChr == PRV_ESC) {
		    ZDspBf("\r\n", 2);
		    CStEnd = CBfPtr;
		    return;
		}
		PrvChr = PRV_ESC;
	    }
	} else {				/* else *CBfPtr != ESCAPE */
	    switch (ChrMsk[(unsigned char)*CBfPtr] & '\17') {
		case RCS_LWR:			/* lowercase */
		    if ((EtFlag & ET_READ_LOWER) == 0) {
			*CBfPtr &= '\137';	/* convert to uppercase */
		    }
		    /* fall through */
		case RCS_DEF:			/* default case */
		    ZDspCh(*CBfPtr);		/* echo the character */
		    PrvChr = PRV_DEF;
		    break;
		case RCS_GRV:			/* accent-grave */
		    ZDspCh('`');
		    if (EtFlag & ET_ACCENT_GRAVE) {
			*CBfPtr = ESCAPE;
			if (PrvChr == PRV_ESC) {
			    ZDspBf("\r\n", 2);
			    CStEnd = CBfPtr;
			    return;
			}
			PrvChr = PRV_ESC;
		    } else {
			PrvChr = PRV_DEF;
		    }
		    break;
		case RCS_SP:			/* space */
		    if (PrvChr == PRV_CTG) {	/* if previous char was ^G */
			CBfPtr--;		/* remove the space */
			TmpPtr = CBfPtr;
			while (TmpPtr > CBfBeg) {
			    TmpPtr--;
			    if (IsEOL(*TmpPtr)) {
				++TmpPtr;
				break;
			    }
			}
			ZDspBf("\r\n", 2);
			if (TmpPtr == CBfBeg) {
			    ZDspCh('*');
			}
			TypBuf(TmpPtr, CBfPtr);
			CBfPtr--;		/* remove ^G */
		    } else {			/* previous char was not ^G */
			ZDspCh(SPACE);		/* just echo space */
		    }
		    PrvChr = PRV_DEF;
		    break;
		case RCS_DEL:			/* delete */
		    InpDel();
		    PrvChr = PRV_DEF;
		    break;
		case RCS_CR:			/* carriage return */
		    ZDspCh(CRETRN);
		    if ((CRCnt == 0) && ChkHlp(CBfPtr)) {
			ZDspCh('*');
			CBfPtr = CBfBeg - 1;
		    } else {
			++CRCnt;
		    }
		    PrvChr = PRV_DEF;
		    break;
		case RCS_LF:			/* line feed */
		    if (CBfPtr == CBfBeg) {	/* if immediate mode */
#if CURSES
			if (ScroLn == 0 || skiprefresh)
#endif
			    if (EtFlag & ET_SCOPE) {
			    	ZDspCh(CRETRN);
			    	ZScrOp(SCR_EEL);
			    }
			HowFar = Ln2Chr((LONG)1);
			MEMMOVE(GapBeg, GapEnd+1, (SIZE_T)HowFar);
			GapBeg += HowFar;
			GapEnd += HowFar;
			if (EvFlag) {
			    DoEvEs(EvFlag);
			} else {
#if CURSES
			    if (ScroLn == 0 || skiprefresh)
#endif
				TypBuf(GapEnd+1, GapEnd+Ln2Chr((LONG)1)+1);
			}
#if CURSES
			if (ScroLn > 0 || skiprefresh)
			    dolf(HowFar);
			if (ScroLn == 0 || skiprefresh)
#endif
			    ZDspCh('*');
#if CURSES
			clrtoeol();
#endif
			CBfPtr = CBfBeg - 1;
		    } else {
			ZDspCh(LINEFD);
		    }
		    PrvChr = PRV_DEF;
		    break;
		case RCS_CTC:			/* control-C */
		    GotCtC = FALSE;		/* clear "stop soon" flag */
		    ctrstr[1] = 'C';
		    ZDspBf(ctrstr, 2);		/* display "^C" */
		    if (PrvChr == PRV_CTC) {	/* if second control-C */
			TAbort(EXIT_SUCCESS);	/* terminate */
		    } else {
			PrvChr = PRV_CTC;
		    }
		    ZDspBf(Prompt, 3);		/* prompt */
		    CBfPtr = CBfBeg - 1;	/* reset cmd-buffer pointer */
		    CRCnt = 0;			/* carriage-return cnt = 0 */
		    break;
		case RCS_BS:			/* backspace */
		    if (CBfPtr == CBfBeg) {	/* if immediate mode */
#if CURSES
			if (ScroLn == 0 || skiprefresh)
#endif
			    if (EtFlag & ET_SCOPE) {
			    	ZDspCh(CRETRN);
			    	ZScrOp(SCR_EEL);
			    }
			HowFar = Ln2Chr(-1L);
			GapBeg += HowFar;
			GapEnd += HowFar;
			MEMMOVE(GapEnd+1, GapBeg, (SIZE_T)-HowFar);
			if (EvFlag) {
			    DoEvEs(EvFlag);
			} else {
#if CURSES
			    if (ScroLn == 0 || skiprefresh)
#endif
				TypBuf(GapEnd+1, GapEnd+Ln2Chr((LONG)1)+1);
			}
#if CURSES
			if (ScroLn > 0 && !skiprefresh)
			    dobs(HowFar);
			if (ScroLn == 0 || skiprefresh)
#endif
			    ZDspCh('*');
#if CURSES
			clrtoeol();
#endif
			CBfPtr = CBfBeg - 1;
		    } else {
			ZDspCh(BAKSPC);
		    }
		    PrvChr = PRV_DEF;
		    break;
		case RCS_CTG:			/* control-G */
		    ZBell();			/* ring the bell (or flash) */
		    ZDspBf("^G", 2);
		    if (PrvChr == PRV_CTG) {	/* if previous char was ^G */
			CBfPtr = CBfBeg - 1;
			ZDspBf(Prompt, 3);
			CRCnt = 0;
			PrvChr = PRV_DEF;
		    } else {
			PrvChr = PRV_CTG;
		    }
		    break;
		case RCS_AST:			/* asterisk */
		    ZDspCh('*');
		    if (PrvChr == PRV_CTG) {
			CBfPtr -= 2;
			ZDspBf(Prompt, 3);
			CRCnt = 0;
			TypBuf(CBfBeg, CBfPtr);
		    }
		    PrvChr = PRV_DEF;
		    break;
		case RCS_CTZ:			/* control-Z */
		    ctrstr[1] = 'Z';
		    ZDspBf(ctrstr, 2);
		    if (PrvChr == PRV_Z2) {
			TAbort(EXIT_SUCCESS);
		    } else if (PrvChr == PRV_Z1) {
			PrvChr = PRV_Z2;
		    } else {
			PrvChr = PRV_Z1;
		    }
		    break;
		case RCS_CCH:			/* other control char */
#if CURSES
		    if (*CBfPtr == CTRL_W) {
		        if (CBfPtr == CBfBeg) {
			    if (ScroLn > 0) {
			        centre();
			    } else {
			        ZDspBf(Prompt, 3);
			    }
			    clrtoeol();
			    CBfPtr = CBfBeg - 1;
			} else {
		            ZDspBf("^W", 2);
		        }
			PrvChr = PRV_DEF;
			break;
		    }
#endif
		    ctrstr[1] = *CBfPtr | '\100';
		    ZDspBf(ctrstr, 2);
		    PrvChr = PRV_DEF;
		    break;
		case RCS_CTU:			/* control-U */
		    while (--CBfPtr >= CBfBeg) {
			if (IsEOL(*CBfPtr)) {
			    break;
			}
		    }
		    ZDspCh(CRETRN);
		    if (EtFlag & ET_SCOPE) {
			ZScrOp(SCR_EEL);	/* erase line */
		    } else {
			ZDspCh(LINEFD);
		    }
		    if (CBfPtr < CBfBeg) {
			ZDspCh('*');
		    }
		    PrvChr = PRV_DEF;
		    break;
		case RCS_VF:			/* vert. tab or form feed */
		    ZDspCh(CRETRN);
		    for (i = 1; i <= FFLINS; i++) {
			ZDspCh(LINEFD);
		    }
		    PrvChr = PRV_DEF;
		    break;
	    }					/* end of switch */
	}
	if (++CBfPtr > CBfEnd) {
	    ZDspBf("command buffer overflow\r\n", 25);	/* ??? */
	    TAbort(EXIT_FAILURE);		/* exit TECO-C */
	}
	*CBfPtr = (unsigned char)ZChIn(FALSE);	/* read a character */
    }						/* end of FOREVER loop */
}
