/*****************************************************************************
	ExeF()
	This function executes a F command.
	nF_	Destructive search and replace
	F'	Flow to end of conditional
	F<	Flow to start of iteration
	F>	Flow to end of iteration
	F|	Flow to ELSE part of conditional
	m,nFB	Search between locations m and n
	nFB	Search, bounded by n lines
	m,nFC	Search and replace between locations m and n
	nFC	Search and replace over n lines
	nFD	Search and delete string
	nFK	Search and delete intervening text
	nFN	Global string replace
	FR	Replace last string
	nFS	Local string replace
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "chmacs.h"		/* define character processing macros */
#include "dchars.h"		/* define identifiers for characters */
#include "deferr.h"		/* define identifiers for error messages */
#if USE_PROTOTYPES
static DEFAULT ExeFBr(void);
static DEFAULT ExeFC(void);
static DEFAULT ExeFD(void);
static DEFAULT ExeFGt(void);
static DEFAULT ExeFK(void);
static DEFAULT ExeFLs(void);
static DEFAULT ExeFN(void);
static DEFAULT ExeFR(void);
static DEFAULT ExeFS(void);
static DEFAULT ExeFSQ(void);
static DEFAULT ExeFUn(void);
#endif
/*****************************************************************************
	ExeFBr()
	This function executes a F| (F-bar) command.
*****************************************************************************/
static DEFAULT ExeFBr()			/* execute an F| command */
{
	DBGFEN(1,"ExeFBr",NULL);
	CmdMod = '\0';				/* clear modifiers flags */
	EStTop = EStBot;			/* clear expression stack */
	DBGFEX(1,DbgFNm,"FlowEE()");
	return FlowEE();
}
/*****************************************************************************
	ExeFC()
	This function executes a FC command.
*****************************************************************************/
static DEFAULT ExeFC()			/* execute an FC command */
{
	DBGFEN(1,"ExeFC",NULL);
	if (CmdMod & DCOLON) {			/* if it's ::FC */
		ErrStr(ERR_ILL, "::FC");	/* illegal command "::FC" */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	SrcTyp = FB_SEARCH;
	if (Replac() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
/*****************************************************************************
	ExeFD()
	This function executes a FD command.
*****************************************************************************/
static DEFAULT ExeFD()			/* execute an FD command */
{
	DBGFEN(1,"ExeFD",NULL);
/*
 * The command m,nFD is illegal: the user should use m,nFB
 */
	if (CmdMod & MARGIS) {			/* if it's m,nFD */
		ErrStr(ERR_ILL, "m,nFD");	/* illegal command "m,nFD" */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (CmdMod & DCOLON) {			/* if it's ::FD */
		ErrStr(ERR_ILL, "::FD");	/* illegal command "::FD" */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	SrcTyp = S_SEARCH;
	if (Search(FALSE) == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (Matchd) {				/* if successful search */
		GapBeg += RefLen;		/* delete found string */
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
/*****************************************************************************
	ExeFGt()
	This function executes a F> command.
*****************************************************************************/
static DEFAULT ExeFGt()			/* execute an F> command */
{
	DBGFEN(1,"ExeFGt",NULL);
	if (LStTop == LStBot) {			/* if not in a loop */
/*
 * "If this command is encountered outside of an iteration,  it has the same
 * effect as the <DELIM><DELIM> command." -- Standard TECO manual.  This used
 * to just return FAILURE,  which also abandoned the command string but did it
 * as an error with no message and no LstErr set,  so nothing was printed and a
 * following "/" showed the PREVIOUS error's paragraph.
 */
		CmdMod = '\0';			/* clear modifiers flags */
		EStTop = EStBot;		/* clear expression stack */
		CBfPtr = CStEnd;		/* exit this command string */
		DBGFEX(1,DbgFNm,"SUCCESS, F> outside a loop");
		return SUCCESS;
	}
	if (FlowEL() == FAILURE) {		/* flow to end-of-loop */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	CBfPtr--;				/* backup before the ">" */
	LStTop++;				/* cancel what FlowEL did */
	CmdMod = '\0';				/* clear modifiers flags */
	EStTop = EStBot;			/* clear expression stack */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
/*****************************************************************************
	ExeFK()
	This function executes a FK command.
*****************************************************************************/
static DEFAULT ExeFK()			/* execute an FK command */
{
	DBGFEN(1,"ExeFK",NULL);
	if (CmdMod & MARGIS) {			/* if it's m,nFK */
		ErrStr(ERR_ILL, "m,nFK");	/* illegal command "m,nFK" */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (CmdMod & DCOLON) {			/* if it's ::FK */
		ErrStr(ERR_ILL, "::FK");	/* illegal command "::FK" */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	SrcTyp = FK_SEARCH;
	if (Search(FALSE) == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
/*****************************************************************************
	ExeFLs()
	This function executes a F< command.
*****************************************************************************/
static DEFAULT ExeFLs()			/* execute an F< command */
{
	DBGFEN(1,"ExeFLs",NULL);
	CmdMod = '\0';				/* clear modifiers flags */
	EStTop = EStBot;			/* clear expression stack */
	CBfPtr = (LStTop == LStBot) ? CStBeg-1 : LStack[LStTop].LAddr;
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
/*****************************************************************************
	ExeFN()
	This function executes a FN command.
*****************************************************************************/
static DEFAULT ExeFN()			/* execute an FN command */
{
	DBGFEN(1,"ExeFN",NULL);
/*
 * The command m,nFN is illegal: the user should use m,nFC
 */
	if (CmdMod & MARGIS) {			/* if it's m,nFN */
		ErrStr(ERR_ILL, "m,nFN");	/* illegal command "m,nFN" */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (CmdMod & DCOLON) {			/* if it's ::FN */
		ErrStr(ERR_ILL, "::FN");	/* illegal command "::FN" */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	SrcTyp = N_SEARCH;
	if (Replac() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
/*****************************************************************************
	ExeFR()
	This function executes a FR command.
*****************************************************************************/
static DEFAULT ExeFR()			/* execute an FR command */
{
	DEFAULT	SavMod;			/* FR's own delimiter modifiers */

	DBGFEN(1,"ExeFR",NULL);
/*
 * "nFRtext`" is "nDItext`" and "m,nFRtext`" is "m,nDItext`" -- the manual says
 * so in as many words.  Both forms used to be accepted and then IGNORED:  the
 * argument was thrown away by the "EStTop = EStBot" below and the RefLen
 * deletion happened anyway,  so the command deleted something the user never
 * asked it to.  ExeD implements both shapes already -- it hands an m,n pair on
 * to ExeK -- so let it do the deletion rather than duplicating the range checks
 * here.  ExeD clears CmdMod when it is done and ExeI still needs FR's delimiter
 * convention,  so carry those bits across it;  and ExeD must not see the COLON,
 * or ":D" would push a value instead of reporting ?DTB.
 */
	if ((CmdMod & MARGIS) || (EStTop > EStBot)) {
		SavMod = CmdMod & (COLON | ATSIGN);
		CmdMod &= ~COLON;
		if (ExeD() == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE, ExeD() failed");
			return FAILURE;
		}
		CmdMod = SavMod;
		DBGFEX(1,DbgFNm,"ExeI()");
		return ExeI();
	}
/*
 * RefLen is the NEGATIVE of the length of the last string inserted or found
 * (see InsStr and SSerch),  so the test has to add it,  not subtract it.
 * The character pointer may have been moved since the string was inserted or
 * found,  so the deletion really can run off the front of the edit buffer.
 */
	if ((GapBeg+RefLen) < EBfBeg) {		/* if out of range */
		ErrMsg(ERR_DTB);		/* DTB = "delete too big" */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	GapBeg += RefLen;			/* delete */
/*
 * ExeI parses FR's OWN text argument,  so it needs FR's delimiter convention:
 * masking CmdMod down to COLON threw the ATSIGN bit away and ExeI then called
 * FindES(ESCAPE),  which inserted "@FR/text/"'s delimiters literally and ran
 * the scan on to the next real ESC.  SkpF already routes FR to SkpArg,  which
 * does honour @,  so the skipper and the executor disagreed as well.
 */
	CmdMod &= (COLON | ATSIGN);		/* retain colon and at-sign bits */
	EStTop = EStBot;			/* clear expression stack */
	DBGFEX(1,DbgFNm,"ExeI()");
	return ExeI();
}
/*****************************************************************************
	ExeFS()
	This function executes a FS command.
*****************************************************************************/
static DEFAULT ExeFS()			/* execute an FS command */
{
	DBGFEN(1,"ExeFS",NULL);
/*
 * The command m,nFS is illegal: the user should use m,nFC
 */
	if (CmdMod & MARGIS) {			/* if it's m,nFS */
		ErrStr(ERR_ILL, "m,nFS");	/* illegal command "m,nFS" */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
/*
 * If it's ::FStext$,  it's a compare,  not a search.  The text argument is
 * compared to the characters immediately following the character pointer.
 * It returns -1 if the strings match, else 0.  A ::FStext$ command is
 * equivalent to a .,.+1:FCtext$ command, so that's the way it's implemented.
 */
	if (CmdMod & DCOLON) {			/* if it's ::FS */
		if (CmdMod & MARGIS) {		/* if it's m,n::FS */
			ErrStr(ERR_ILL, "m,n::FS");
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
		if (EStTop > EStBot) {		/* if it's n::FS */
			ErrStr(ERR_ILL, "n::FS");
			DBGFEX(1,DbgFNm,"FAILURE");
			return FAILURE;
		}
		if (GapEnd == EBfEnd) {		/* if nothing to search */
/*
 * The answer is 0,  but the two text arguments still have to be consumed the
 * way Search() would have consumed them.  This shortcut did not consume them,
 * and ExeF leaves CBfPtr on the "S",  so ExeCSt's "++CBfPtr" landed on the
 * first character of the search text and the command scanner read the user's
 * data as commands: "::FSa$IZZZ$" inserted ZZZ into the edit buffer.  The
 * --CBfPtr between the two FindES calls cancels the IncCBP the second one
 * starts with,  exactly as Search() does for a two-argument command.
 */
			if (FindES(ESCAPE) == FAILURE) { /* end of search text */
				DBGFEX(1,DbgFNm,"FAILURE, FindES() failed");
				return FAILURE;
			}
			if (CmdMod & ATSIGN) {	/* if it's at-sign modified */
				--CBfPtr;
			}
			if (FindES(ESCAPE) == FAILURE) { /* end of replace text */
				DBGFEX(1,DbgFNm,"FAILURE, FindES() failed");
				return FAILURE;
			}
			CmdMod = '\0';		/* clear modifiers flags */
			DBGFEX(1,DbgFNm,"PushEx(0)");
			return PushEx(0L,OPERAND);
		}
		CmdMod &= ~DCOLON;		/* clear double-colon bit */
		CmdMod |= COLON;		/* set colon bit */
		CmdMod |= MARGIS;		/* set m defined bit */
		MArgmt = GapBeg - EBfBeg;	/* set m */
		if (PushEx((LONG)((GapBeg-EBfBeg)+1),OPERAND) == FAILURE) {
			DBGFEX(1,DbgFNm,"FAILURE, PushEx() failed");
			return FAILURE;
		}
		DBGFEX(1,DbgFNm,"ExeFC()");
		return ExeFC();			/* execute FC command */
	}
	SrcTyp = S_SEARCH;
	if (Replac() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE, Replac() failed");
		return FAILURE;
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
/*****************************************************************************
	ExeFSQ()
	This function executes a F' command.
*****************************************************************************/
static DEFAULT ExeFSQ()				/* execute an F' command */
{
	DBGFEN(1,"ExeFSQ",NULL);
	CmdMod = '\0';				/* clear modifiers flags */
	EStTop = EStBot;			/* clear expression stack */
	DBGFEX(1,DbgFNm,"FlowEC()");
	return FlowEC();
}
/*****************************************************************************
	ExeFUn()
	This function executes a F_ command.
*****************************************************************************/
static DEFAULT ExeFUn()				/* execute an F_ command */
{
	DBGFEN(1,"ExeFUn",NULL);
/*
 * The command m,nF_ is illegal: the user should use m,nFC
 */
	if (CmdMod & MARGIS) {			/* if it's m,nF_ */
		ErrStr(ERR_ILL, "m,nF_");	/* illegal command "m,nF_" */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	if (CmdMod & DCOLON) {			/* if it's ::F_ */
		ErrStr(ERR_ILL, "::F_");	/* illegal command "::F_" */
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	SrcTyp = U_SEARCH;
	if (Replac() == FAILURE) {
		DBGFEX(1,DbgFNm,"FAILURE");
		return FAILURE;
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(1,DbgFNm,"SUCCESS");
	return SUCCESS;
}
/*****************************************************************************
	ExeF()
	This function executes an F command.
*****************************************************************************/
DEFAULT ExeF()				/* execute one of the F commands */
{
	if (IncCBP() == FAILURE) {
		return FAILURE;
	}
	switch (To_Upper(*CBfPtr)) {
          case 'S':  return ExeFS();
	  case 'N':  return ExeFN();
	  case 'D':  return ExeFD();
	  case 'R':  return ExeFR();
	  case 'B':  return ExeFB();
	  case 'C':  return ExeFC();
	  case 'K':  return ExeFK();
	  case '_':  return ExeFUn();
	  case '>':  return ExeFGt();
	  case '<':  return ExeFLs();
	  case '\'': return ExeFSQ();
	  case '|':  return ExeFBr();
	  default:   ErrChr(ERR_IFC, *CBfPtr);
		     return FAILURE;
	}
}
