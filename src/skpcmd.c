/*****************************************************************************
	SkpCmd.c
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "chmacs.h"		/* define character processing macros */
#include "dchars.h"		/* define identifiers for characters */
#include "deferr.h"		/* define identifiers for error messages */
#if USE_PROTOTYPES
static DEFAULT SkpAgV(void);	/* SkpArg for a value-producing command */
static DEFAULT SkpArg(void);	/* skip command with text argument */
static DEFAULT SkpCrt(void);	/* skip a ^ (caret) command */
static DEFAULT SkpCtA(void);	/* skip control-A command */
static DEFAULT SkpCtU(void);	/* skip control-U command */
static DEFAULT SkpCUV(void);	/* SkpCtU for a value-producing command */
static DEFAULT SkpDAr(void);	/* skip "double text argument" command */
static DEFAULT SkpDqu(void);	/* skip a " (double quote) command */
static DEFAULT SkpE(void);	/* skip one of the E commands */
static DEFAULT SkpExc(void);	/* skip command with text argument */
static DEFAULT SkpF(void);	/* skip one of the F commands */
static DEFAULT SkpMac(void);	/* skip an Mq command */
static DEFAULT SkpOne(void);	/* skip one command character */
static DEFAULT SkpQRC(void);	/* skip a Q-register name, clear CmdMod */
static DEFAULT SkpQRg(void);	/* skip a Q-register name */
static DEFAULT SkpQRV(void);	/* skip a Q-register name, keep MARGIS */
static DEFAULT SkpSkp(void);	/* */
static DEFAULT SkpVal(void);	/* skip a value-producing command */
#endif
/*****************************************************************************
	SkpArg()
*****************************************************************************/
static DEFAULT SkpArg()		/* skip command with text argument */
{
	BOOLEAN TTrace;		/* temp: holds trace flag */
	TTrace = TraceM;			/* save trace mode flag */
	TraceM = FALSE;				/* keep FindES from tracing */
	if (FindES(ESCAPE) == FAILURE)		/* find end of string */
		{
		TraceM = TTrace;		/* restore trace mode flag */
		return FAILURE;
		}
	TraceM = TTrace;			/* restore trace mode flag */
	CmdMod = '\0';				/* clear modifiers flags */
	return SUCCESS;
}
/*****************************************************************************
	SkpAgV()
	SkpArg for a command that has a VALUE-PRODUCING form:  EB, EG, EI, EN,
ER and EW.  ":EBfile$", ":EGcmd$", ":EIfile$", ":ENfile$", ":ERfile$" and
":EWfile$" all end in a PushEx and so leave MARGIS alone,  while the plain forms
clear the lot.  A skipper cannot tell the colon form from the plain one -- the
same problem A, C, J and R have in SkpVal -- and the value-producing form is
again the one that can stand between a modifier and its command.  FindES needs
the at-sign while it scans,  so SkpArg does the work and the "m," is put back
afterwards.  The at-sign itself is NOT put back:  these commands have a text
argument of their own and have just used it on that.
*****************************************************************************/
static DEFAULT SkpAgV()		/* skip a value-producing text-arg command */
{
	char SvMArg;
	SvMArg = CmdMod & MARGIS;		/* the "m," belongs to the... */
	if (SkpArg() == FAILURE)		/* ...command AFTER this one */
		return FAILURE;
	CmdMod |= SvMArg;
	return SUCCESS;
}
/*****************************************************************************
	The Skp* functions below all have to stop when they reach the last
character of the command string.  They used to return SUCCESS -- without
advancing -- when that happened inside a macro,  on the theory that the caller
would notice.  That is the same trap IncCBP used to have: it hands a command
back to a scanning loop that has already consumed everything, and the loop's
"++CBfPtr" then steps past CStEnd.  The Standard TECO manual requires a
command sequence in a Q-register to be complete within the Q-register,  and
ErrUTC() already reports ?UTC or ?UTM according to MStTop,  so failing here is
both correct and one iteration earlier than the callers would have managed.
*****************************************************************************/
/*****************************************************************************
	SkpQRg()
	This function skips a Q-register name.  On entry CBfPtr points at the
command character;  on return it points at the last character of the name.
	A Q-register name is not always one character: FindQR accepts a leading
"." for a register local to the current macro level and consumes two.  These
commands used to be skipped with SkpOne,  which stepped over the "." and left
the register letter to be rescanned as a command -- and ".I", ".N", ".O" and
".S" are commands with a text argument,  so the scan then swallowed everything
up to the next ESC.  A macro that used local Q-registers therefore could not
safely contain a null loop,  a conditional,  or an O command.
*****************************************************************************/
static DEFAULT SkpQRg()		/* skip a Q-register name */
{
	if (CBfPtr == CStEnd) {
		ErrUTC();			/* unterminated command */
		return FAILURE;
	}
	++CBfPtr;
	if (*CBfPtr == '.') {			/* if local Q-register name */
		if (CBfPtr == CStEnd) {
			ErrUTC();		/* unterminated command */
			return FAILURE;
		}
		++CBfPtr;			/* step over the register letter */
	}
	return SUCCESS;
}
/*****************************************************************************
	SkpQRC()
	Skip a Q-register name and then clear CmdMod,  which is what the
executors of %, G, M, Q, U and X all do (ExeM clears everything but MARGIS,
and so does its skipper -- see below).  SkpQRg itself cannot do the clearing,
because it is also the skipper for "[" and "]",  whose executors leave CmdMod
alone;  the manual calls those two "transparent to numeric arguments".
	Without this the skipper and the executor stopped in different places:
"@Ga" executed consumed its at-sign and "@Ga" skipped did not,  so SkpArg's
FindES then looked for an ESCAPE instead of honouring the delimiter and the
skip ran through the end of the loop body it was supposed to stop in.
*****************************************************************************/
static DEFAULT SkpQRC()		/* skip a Q-register name, clear CmdMod */
{
	if (SkpQRg() == FAILURE)
		return FAILURE;
	CmdMod = '\0';				/* clear modifiers flags */
	return SUCCESS;
}
/*****************************************************************************
	SkpMac()
	Skip an Mq command.  ExeM clears everything but MARGIS -- "m,nMq" is
how a macro is handed two arguments -- so the skipper leaves the same bits.
*****************************************************************************/
static DEFAULT SkpMac()		/* skip an Mq command */
{
	if (SkpQRg() == FAILURE)
		return FAILURE;
	CmdMod &= MARGIS;			/* clear modifiers flags */
	return SUCCESS;
}
/*****************************************************************************
	SkpQRV()
	Skip a Q-register name for one of the VALUE-PRODUCING Q-register
commands,  % and Q.  Every arm of ExePrc and of ExeQ ends in a PushEx,  so both
leave MARGIS and ATSIGN alone -- the "m," of "m,QqT" and the "@" of "@QqS/x/"
belong to the command that takes the result -- and the skipper has to leave the
same bits behind.  SkpMac keeps MARGIS for a different reason:  "m,nMq" is how
a macro is handed two arguments.
*****************************************************************************/
static DEFAULT SkpQRV()		/* skip a Q-register name, keep MARGIS */
{
	if (SkpQRg() == FAILURE)
		return FAILURE;
	CmdMod &= (MARGIS | ATSIGN);		/* keep those two, clear the rest */
	return SUCCESS;
}
/*****************************************************************************
	SkpCtA()
*****************************************************************************/
static DEFAULT SkpCtA()		/* skip control-A command */
{
	BOOLEAN	TTrace;		/* temp: holds trace flag */
	TTrace = TraceM;			/* save trace mode flag */
	TraceM = FALSE;				/* keep FindES from tracing */
	if (FindES(CTRL_A) == FAILURE)		/* find end of string */
		{
		TraceM = TTrace;		/* restore trace mode flag */
		return FAILURE;
		}
	TraceM = TTrace;			/* restore trace mode flag */
	CmdMod = '\0';				/* clear modifiers flags */
	return SUCCESS;
}
/*****************************************************************************
	SkpCtU()
	Skip a Q-register name followed by a text argument.  That is what ^U
consumes,  and also what EQ and E% consume,  so SkpE sends those here too.
*****************************************************************************/
static DEFAULT SkpCtU()		/* skip control-U command */
{
	BOOLEAN	TTrace;		/* temp: holds trace flag */
	if (SkpQRg() == FAILURE)		/* skip the Q-register name */
		return FAILURE;
	TTrace = TraceM;			/* save trace mode flag */
	TraceM = FALSE;				/* keep FindES from tracing */
	if (FindES(ESCAPE) == FAILURE)		/* find end of string */
		{
		TraceM = TTrace;		/* restore trace mode flag */
		return FAILURE;
		}
	TraceM = TTrace;			/* restore trace mode flag */
	CmdMod = '\0';				/* clear modifiers flags */
	return SUCCESS;
}
/*****************************************************************************
	SkpCUV()
	SkpCtU for E%,  whose colon form ":E%qfile$" ends in a PushEx and so
leaves MARGIS alone.  See SkpAgV above.
*****************************************************************************/
static DEFAULT SkpCUV()		/* skip a value-producing ^U-shaped command */
{
	char SvMArg;
	SvMArg = CmdMod & MARGIS;		/* the "m," belongs to the... */
	if (SkpCtU() == FAILURE)		/* ...command AFTER this one */
		return FAILURE;
	CmdMod |= SvMArg;
	return SUCCESS;
}
/*****************************************************************************
	SkpDAr()
*****************************************************************************/
static DEFAULT SkpDAr()		/* skip "double text argument" command */
{
	BOOLEAN TTrace;		/* saves TraceM temporarily */
	TTrace = TraceM;			/* save trace mode flag */
	TraceM = FALSE;				/* keep FindES from tracing */
	if (FindES(ESCAPE) == FAILURE)
		{
		TraceM = TTrace;		/* restore trace mode flag */
		return FAILURE;
		}
	if (CmdMod & ATSIGN)			/* if at-sign modified */
		--CBfPtr;
	if (FindES(ESCAPE) == FAILURE)
		{
		TraceM = TTrace;		/* restore trace mode flag */
		return FAILURE;
		}
	TraceM = TTrace;			/* restore trace mode flag */
/*
 * Every other Skp* function that consumes a text argument clears CmdMod here.
 * This one did not,  so an @-modified FS/FN/FC/F_ in skipped code left ATSIGN
 * set and FindES took the FOLLOWING command's first text character as its
 * delimiter -- mis-parsing the rest of a null loop body,  of the untaken arm of
 * a conditional,  or of an O command's tag search.
 */
	CmdMod = '\0';				/* clear modifiers flags */
	return SUCCESS;
}
/*****************************************************************************
	SkpDqu()
*****************************************************************************/
static DEFAULT SkpDqu()		/* skip a " (double quote) command */
{
	DBGFEN(3,"SkpDqu",NULL);
	if (CBfPtr == CStEnd) {			/* if end of command string */
		ErrUTC();			/* unterminated command */
		return FAILURE;
	}
	++CBfPtr;
	switch (To_Upper(*CBfPtr)) {
		case 'A':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'U':
		case '=':
		case 'G':
		case '>':
		case 'L':
		case 'S':
		case 'T':
		case '<':
		case 'N':
		case 'R':
		case 'V':
		case 'W':
			break;
		default:
			ErrMsg(ERR_IQC);    /* ill. char. after " */
			DBGFEX(3,DbgFNm,"FAILURE");
			return FAILURE;
	}
	CmdMod = '\0';				/* clear modifiers flags */
	DBGFEX(3,DbgFNm,"SUCCESS");
	return SUCCESS;
}
/*****************************************************************************
	SkpExc()
*****************************************************************************/
static DEFAULT SkpExc()		/* skip command with text argument */
{
	BOOLEAN	TTrace;		/* temp: holds trace flag */
	TTrace = TraceM;			/* save trace mode flag */
	TraceM = FALSE;				/* keep FindES from tracing */
	if (FindES('!') == FAILURE)		/* find end of string */
		{
		TraceM = TTrace;		/* restore trace mode flag */
		return FAILURE;
		}
	TraceM = TTrace;			/* restore trace mode flag */
	CmdMod = '\0';				/* clear modifiers flags */
	return SUCCESS;
}
/*****************************************************************************
	SkpSkp()
*****************************************************************************/
static DEFAULT SkpSkp()		/* */
{
	CmdMod = '\0';				/* clear ATSIGN modifier */
	return SUCCESS;
}
/*****************************************************************************
	SkpVal()
	SkpSkp for a command that has a VALUE-PRODUCING form:  A, C, J and R.
":A", ":C", ":J" and ":R" all end in a PushEx and so leave MARGIS and ATSIGN
alone,  and a skipper cannot tell the colon form from the plain one -- the same
problem ^R and ^T have,  and the value-producing form is again the one that can
stand between a modifier and its command.
	^E, ^Q, ^X, \, D, P, Y, ED, EH, ES, ET, EU, EV, EZ and EY are here too.
^Q and the read form of \ are pure value producers;  the flags have a READ form
that is one -- DoFlag's read path and ExeCtE's read arm both end in a PushEx
now -- and a write form that consumes its argument;  ":D", ":P" and ":Y" are the
value-producing forms of the other three.  As SkpSkp they cleared an "m," the
executor keeps,  which is the divergence round 7 section 4 forbids.
	The at-sign is kept for the same reason as the "m,":  none of these
commands has a text argument for an "@" to modify,  so an at-sign written in
front of one belongs to the command that takes the result.  Clearing it made
"@EDS/x/" search for the three characters "/x/".  The colon still goes,  as it
does for every skipped command:  a value producer either has a colon FORM,  in
which case the bit is its own and it has just been used to pick this arm,  or it
has none and the bit has nowhere to go.
*****************************************************************************/
static DEFAULT SkpVal()		/* skip a value-producing command */
{
	CmdMod &= (MARGIS | ATSIGN);		/* keep those two, clear the rest */
	return SUCCESS;
}
/*****************************************************************************
	SkpF()
*****************************************************************************/
static DEFAULT SkpF()			/* skip one of the F commands */
{
	if (CBfPtr == CStEnd) {
		ErrUTC();			/* unterminated command */
		return FAILURE;
	}
	++CBfPtr;
	switch (To_Upper(*CBfPtr)) {
		case 'S':
		case 'N':
		case 'C':
		case '_':	return SkpDAr();
		case 'D':
		case 'R':
		case 'B':
		case 'K':	return SkpArg();
		case '>':
		case '<':
		case '\'':
		case '|':	return SkpSkp();
		default:
		  		ErrChr(ERR_IFC, *CBfPtr);
				return FAILURE;
	}
}
/*****************************************************************************
	SkpOne()
*****************************************************************************/
static DEFAULT SkpOne()		/* skip one command character */
{
#if DEBUGGING
	static char *DbgFNm = "SkpOne";
	sprintf(DbgSBf,"*CBfPtr = '%c'", *CBfPtr);
	DbgFEn(3,DbgFNm,DbgSBf);
#endif
	if (CBfPtr == CStEnd) {
		ErrUTC();			/* unterminated command */
		DBGFEX(3,DbgFNm,"FAILURE");
		return FAILURE;
	}
	++CBfPtr;
#if DEBUGGING
	sprintf(DbgSBf,"*CBfPtr = '%c'", *CBfPtr);
	DbgFEx(3,DbgFNm,DbgSBf);
#endif
	return SUCCESS;
}
/*****************************************************************************
	SkpCrt()
	See the comment above FSAray for why the value-producing commands are
ExeNul here rather than SkpSkp:  ZExCtB, ZExCtH, ExeCtN, ExeCtS, ExeCtY,
ExeCtZ and ExeUsc all return WITHOUT touching CmdMod -- they have to, since
they are operands and can stand between a modifier and its command -- so a
skipper that cleared it stopped somewhere execution would not have.  ^R and ^T
are here for the same reason:  their argument-less read forms are operands too.
*****************************************************************************/
static DEFAULT SkpCrt()		/* skip a ^ (caret) command */
{
	static DEFAULT (*FCAray[])(VVOID) = {
/* ^A*/ SkpCtA,   /* ^B*/ ExeNul,   /* ^C*/ SkpSkp,   /* ^D*/ SkpSkp,
/* ^E*/ SkpVal,   /* ^F*/ SkpSkp,   /* ^G*/ SkpSkp,   /* ^H*/ ExeNul,
/*TAB*/ SkpArg,   /* LF*/ ExeNul,   /* VT*/ SkpSkp,   /* FF*/ SkpSkp,
/* CR*/ ExeNul,   /* ^N*/ ExeNul,   /* ^O*/ SkpSkp,   /* ^P*/ SkpSkp,
/* ^Q*/ SkpVal,   /* ^R*/ ExeNul,   /* ^S*/ ExeNul,   /* ^T*/ ExeNul,
/* ^U*/ SkpCtU,   /* ^V*/ SkpSkp,   /* ^W*/ SkpSkp,   /* ^X*/ SkpVal,
/* ^Y*/ ExeNul,   /* ^Z*/ ExeNul,   /* ^[*/ SkpSkp,   /* ^\*/ SkpSkp,
/* ^]*/ SkpSkp,   /* ^^*/ SkpOne,   /* ^_*/ ExeNul
	};
	WORD	CDummy;
	if (CBfPtr == CStEnd) {			/* if end of command string */
		ErrUTC();			/* unterminated command */
		return FAILURE;
	}
	++CBfPtr;
	if ((*CBfPtr >= 'A') && (*CBfPtr <= '_'))
		CDummy = *CBfPtr - 'A';
	else if (Is_Lower(*CBfPtr))
		CDummy = *CBfPtr - 'a';		/* convert to upper case */
	else {
/*
 * ErrChr,  not ErrMsg:  Errors[ERR_IUC] is "Illegal character \"%s\" following
 * ^",  and ErrMsg has no argument to substitute,  so a bad caret command inside
 * skipped code showed the user the format specifier itself.  ExeCrt and BldStr
 * report the same message with ErrChr.
 */
		ErrChr(ERR_IUC, *CBfPtr);	/* illegal char after ^ */
		return FAILURE;
	}
	return (*FCAray[CDummy])();
}
/*****************************************************************************
	SkpE()
*****************************************************************************/
static DEFAULT SkpE()			/* skip one of the E commands */
{
	unsigned char TmpChr;
/*
 * ENfilespec$ always takes a text argument -- ExeEN starts with a BldStr,
 * exactly like EB, EI, ER and EW -- but the table below skipped it as if it
 * took none,  which handed the filespec back to the scanning loop to be read
 * as commands.  A "<", ">", "!", '"', "'" or "|" in a wildcard filespec then
 * really did change where the skip stopped.
 *	EJ and EO are SkpVal rather than SkpSkp:  both end in a PushEx,  so
 * both are value producers that keep MARGIS and ATSIGN,  and the skipper has to
 * leave the same bits behind as the executor.  See SkpVal.
 */
	static DEFAULT (*FEAray[])(VVOID) = {
/* A */ SkpSkp,    /* B */ SkpAgV,    /* C */ SkpSkp,    /* D */ SkpVal,
/* E */ SkpSkp,    /* F */ SkpSkp,    /* G */ SkpAgV,    /* H */ SkpVal,
/* I */ SkpAgV,    /* J */ SkpVal,    /* K */ SkpSkp,    /* L */ SkpArg,
/* M */ SkpSkp,    /* N */ SkpAgV,    /* O */ SkpVal,    /* P */ SkpSkp,
/* Q */ SkpCtU,    /* R */ SkpAgV,    /* S */ SkpVal,    /* T */ SkpVal,
/* U */ SkpVal,    /* V */ SkpVal,    /* W */ SkpAgV,    /* X */ SkpSkp,
/* Y */ SkpVal,    /* Z */ SkpVal
	};
	if (CBfPtr == CStEnd) {
		ErrUTC();			/* unterminated command */
		return FAILURE;
	}
	++CBfPtr;
	TmpChr = To_Upper(*CBfPtr);
/*
 * EQq<filespec>$ and E%q<filespec>$ each take a Q-register name AND a text
 * argument;  ExeEQ and ExeEPr both do IncCBP,  FindQR,  BldStr.  Skipping them
 * with SkpOne stepped over the register name only and handed the filespec back
 * to the scanning loop,  which then read it as commands -- and a "<", ">", "!",
 * '"', "'" or "|" in a file name really does change what the loop does.
 * SkpCtU skips exactly what those two commands consume.
 */
	if (TmpChr == '%')
		return SkpCUV();
	else if (TmpChr == '_')
		return SkpArg();
	else if (!Is_Upper(TmpChr))	/* EE is ?NYI now, not ?IEC */
		{
		ErrChr(ERR_IEC, *CBfPtr);
		return FAILURE;
		}
	return (*FEAray[TmpChr-'A'])();
}
/*****************************************************************************
	SkpCmd()
	This function "skips" TECO commands. It is used when TECO needs to
skip forward under one of these conditions:
	1. flow to the end of a conditional
	2. flow to the else clause of a conditional
	3. flow to the end of a loop
	4. flow while searching for a tag
	When this function is called, CBfPtr points to a command.  When this
function returns, CBfPtr is left pointing to the last character of the
command. In the case of a command like T, CBfPtr is not changed.  In the
case of a command like Stext<ESC>, CBfPtr is left pointing to the <ESC>.
	Note which table entries below are NOT Skp* functions.  SkpSkp clears
CmdMod,  which is what a skipped command must do,  but the table is indexed by
every character,  not just by commands,  and the characters that come BETWEEN a
modifier and its command must leave CmdMod alone.  The at-sign has always been
ExeAtS -- the real executor -- so that skipping "@" sets the bit the way
executing it would;  the colon is ExeCln for the same reason,  and the digits,
",", "-" and "+" are ExeNul,  which returns SUCCESS without touching CmdMod.
With those as SkpSkp,  "@:S/text/" lost its at-sign before SkpArg ever saw it,
FindES looked for an ESCAPE instead of honouring the "/" delimiter,  and the
skip ran to the wrong place.
	That rule is not about the digits:  it is about every character that can
stand between a modifier and its command,  which is every VALUE-PRODUCING
command as well.  ExeDot, ExeB, ExeZ, ExeH, ExeOpr, ExeRtP, ExeUsc, ExeCtN,
ExeCtS, ExeCtY, ExeCtZ, ZExCtB and ZExCtH all return without touching CmdMod --
they have to,  since ExeCom sets MARGIS and the commonest m,n idiom in TECO is
".,.+5T",  which would lose its "m" the instant "." cleared the flag.  So ".",
"B", "H", "Z", "(", ")", "*", "/", "#", "&" and the control characters above
are ExeNul here too;  as SkpSkp they threw away an at-sign the executor would
have kept,  and "0<@.+1S/x/>" reported ?UTC while "1<@.+1S/x/>" ran.
	The other direction is the six commands that take a Q-register name:
ExePrc, ExeG, ExeM, ExeQ, ExeU and ExeX all END by clearing CmdMod,  while
SkpQRg left it alone.  Those use SkpQRC (SkpMac for M,  SkpQRV for % and Q)
below.  "[" and "]" keep the plain SkpQRg:  their executors leave CmdMod
alone,  so it already agrees.
	% and Q are in that group only for the colon.  Both are value
producers themselves,  so both KEEP MARGIS AND ATSIGN -- SkpQRV -- and so do
A, C, J and R,  whose colon forms return a value:  those four are SkpVal.
Clearing MARGIS for them threw away an "m" they had never taken,  so "m,QqK"
deleted the whole buffer and the shipped lib/dir.tec stored an empty
Q-register;  clearing ATSIGN threw away an at-sign they had never taken
either,  so "1UA@QAS/b/" searched for the three characters "/b/".
	"\" is in that group too -- the read form of the backslash command,
"the number in its ASCII representation in the edit buffer",  ends in a PushEx
like the rest of them,  and only the "n\" write form consumes anything.  As
SkpSkp it cleared an "m," the executor keeps,  so it is SkpVal now.
	^R,  ^T and "'" belong to the same first group and were missed by that
pass.  ExeCtR and ExeCtT both have two paths:  the argument-less READ form is a
pure value producer that returns PushEx() without touching CmdMod,  and only
the n^R / n^T write form clears it.  A skipper cannot tell the two apart,  but
the read form is the one that can stand between a modifier and its command,  so
ExeNul is the entry that matches -- inside skipped code the "n" of n^R is not
on any stack to be consumed anyway.  "'",  the closing quote of a conditional,
is ExeNul in FArray:  it does nothing at all.  As SkpSkp all three threw away
an at-sign,  so "0<@^RS/x/>" reported ?UTC while "1<@^RS/x/>" ran.
*****************************************************************************/
DEFAULT SkpCmd()		/* skip a "command" */
{
	static DEFAULT (*FSAray[])(VVOID) = {
/*NUL*/ ExeNul,   /* ^A*/ SkpCtA,   /* ^B*/ ExeNul,   /* ^C*/ SkpSkp,
/* ^D*/ SkpSkp,   /* ^E*/ SkpVal,   /* ^F*/ SkpSkp,   /* ^G*/ SkpSkp,
/* BS*/ ExeNul,   /*TAB*/ SkpArg,   /* LF*/ ExeNul,   /* VT*/ SkpSkp,
/* FF*/ SkpSkp,   /* CR*/ ExeNul,   /* ^N*/ ExeNul,   /* ^O*/ SkpSkp,
/* ^P*/ SkpSkp,   /* ^Q*/ SkpVal,   /* ^R*/ ExeNul,   /* ^S*/ ExeNul,
/* ^T*/ ExeNul,   /* ^U*/ SkpCtU,   /* ^V*/ SkpSkp,   /* ^W*/ SkpSkp,
/* ^X*/ SkpVal,   /* ^Y*/ ExeNul,   /* ^Z*/ ExeNul,   /*ESC*/ SkpSkp,
/* ^\*/ SkpSkp,   /* ^]*/ SkpSkp,   /* ^^*/ SkpOne,   /* ^_*/ ExeNul,
/* SP*/ ExeNul,   /* ! */ SkpExc,   /* " */ SkpDqu,   /* # */ ExeNul,
/* $ */ SkpSkp,   /* % */ SkpQRV,   /* & */ ExeNul,   /* ' */ ExeNul,
/* ( */ ExeNul,   /* ) */ ExeNul,   /* * */ ExeNul,   /* + */ ExeNul,
/* , */ ExeNul,   /* - */ ExeNul,   /* . */ ExeNul,   /* / */ ExeNul,
/* 0 */ ExeNul,   /* 1 */ ExeNul,   /* 2 */ ExeNul,   /* 3 */ ExeNul,
/* 4 */ ExeNul,   /* 5 */ ExeNul,   /* 6 */ ExeNul,   /* 7 */ ExeNul,
/* 8 */ ExeNul,   /* 9 */ ExeNul,   /* : */ ExeCln,   /* ; */ SkpSkp,
/* < */ SkpSkp,   /* = */ SkpSkp,   /* > */ SkpSkp,   /* ? */ SkpSkp,
/* @ */ ExeAtS,   /* A */ SkpVal,   /* B */ ExeNul,   /* C */ SkpVal,
/* D */ SkpVal,   /* E */ SkpE,     /* F */ SkpF,     /* G */ SkpQRC,
/* H */ ExeNul,   /* I */ SkpArg,   /* J */ SkpVal,   /* K */ SkpSkp,
/* L */ SkpSkp,   /* M */ SkpMac,   /* N */ SkpArg,   /* O */ SkpArg,
/* P */ SkpVal,   /* Q */ SkpQRV,   /* R */ SkpVal,   /* S */ SkpArg,
/* T */ SkpSkp,   /* U */ SkpQRC,   /* V */ SkpSkp,   /* W */ SkpSkp,
/* X */ SkpQRC,   /* Y */ SkpVal,   /* Z */ ExeNul,   /* [ */ SkpQRg,
/* \ */ SkpVal,   /* ] */ SkpQRg,   /* ^ */ SkpCrt,   /* _ */ SkpArg,
/* ` */ SkpSkp,   /* a */ SkpVal,   /* b */ ExeNul,   /* c */ SkpVal,
/* d */ SkpVal,   /* e */ SkpE,     /* f */ SkpF,     /* g */ SkpQRC,
/* h */ ExeNul,   /* i */ SkpArg,   /* j */ SkpVal,   /* k */ SkpSkp,
/* l */ SkpSkp,   /* m */ SkpMac,   /* n */ SkpArg,   /* o */ SkpArg,
/* p */ SkpVal,   /* q */ SkpQRV,   /* r */ SkpVal,   /* s */ SkpArg,
/* t */ SkpSkp,   /* u */ SkpQRC,   /* v */ SkpSkp,   /* w */ SkpSkp,
/* x */ SkpQRC,   /* y */ SkpVal,   /* z */ ExeNul,   /* { */ SkpSkp,
/* | */ SkpSkp,   /* } */ SkpSkp,   /* ~ */ SkpSkp,   /*DEL*/ SkpSkp,
/*129*/ SkpSkp,   /*130*/ SkpSkp,   /*131*/ SkpSkp,   /*132*/ SkpSkp,
/*133*/ SkpSkp,   /*134*/ SkpSkp,   /*135*/ SkpSkp,   /*136*/ SkpSkp,
/*137*/ SkpSkp,   /*138*/ SkpSkp,   /*139*/ SkpSkp,   /*140*/ SkpSkp,
/*141*/ SkpSkp,   /*142*/ SkpSkp,   /*143*/ SkpSkp,   /*144*/ SkpSkp,
/*145*/ SkpSkp,   /*146*/ SkpSkp,   /*147*/ SkpSkp,   /*148*/ SkpSkp,
/*149*/ SkpSkp,   /*150*/ SkpSkp,   /*151*/ SkpSkp,   /*152*/ SkpSkp,
/*153*/ SkpSkp,   /*154*/ SkpSkp,   /*155*/ SkpSkp,   /*156*/ SkpSkp,
/*157*/ SkpSkp,   /*158*/ SkpSkp,   /*159*/ SkpSkp,   /*160*/ SkpSkp,
/*161*/ SkpSkp,   /*162*/ SkpSkp,   /*163*/ SkpSkp,   /*164*/ SkpSkp,
/*165*/ SkpSkp,   /*166*/ SkpSkp,   /*167*/ SkpSkp,   /*168*/ SkpSkp,
/*169*/ SkpSkp,   /*170*/ SkpSkp,   /*171*/ SkpSkp,   /*172*/ SkpSkp,
/*173*/ SkpSkp,   /*174*/ SkpSkp,   /*175*/ SkpSkp,   /*176*/ SkpSkp,
/*177*/ SkpSkp,   /*178*/ SkpSkp,   /*179*/ SkpSkp,   /*180*/ SkpSkp,
/*181*/ SkpSkp,   /*182*/ SkpSkp,   /*183*/ SkpSkp,   /*184*/ SkpSkp,
/*185*/ SkpSkp,   /*186*/ SkpSkp,   /*187*/ SkpSkp,   /*188*/ SkpSkp,
/*189*/ SkpSkp,   /*190*/ SkpSkp,   /*191*/ SkpSkp,   /*192*/ SkpSkp,
/*193*/ SkpSkp,   /*194*/ SkpSkp,   /*195*/ SkpSkp,   /*196*/ SkpSkp,
/*197*/ SkpSkp,   /*198*/ SkpSkp,   /*199*/ SkpSkp,   /*200*/ SkpSkp,
/*201*/ SkpSkp,   /*202*/ SkpSkp,   /*203*/ SkpSkp,   /*204*/ SkpSkp,
/*205*/ SkpSkp,   /*206*/ SkpSkp,   /*207*/ SkpSkp,   /*208*/ SkpSkp,
/*209*/ SkpSkp,   /*210*/ SkpSkp,   /*211*/ SkpSkp,   /*212*/ SkpSkp,
/*213*/ SkpSkp,   /*214*/ SkpSkp,   /*215*/ SkpSkp,   /*216*/ SkpSkp,
/*217*/ SkpSkp,   /*218*/ SkpSkp,   /*219*/ SkpSkp,   /*220*/ SkpSkp,
/*221*/ SkpSkp,   /*222*/ SkpSkp,   /*223*/ SkpSkp,   /*224*/ SkpSkp,
/*225*/ SkpSkp,   /*226*/ SkpSkp,   /*227*/ SkpSkp,   /*228*/ SkpSkp,
/*229*/ SkpSkp,   /*230*/ SkpSkp,   /*231*/ SkpSkp,   /*232*/ SkpSkp,
/*233*/ SkpSkp,   /*234*/ SkpSkp,   /*235*/ SkpSkp,   /*236*/ SkpSkp,
/*237*/ SkpSkp,   /*238*/ SkpSkp,   /*239*/ SkpSkp,   /*240*/ SkpSkp,
/*241*/ SkpSkp,   /*242*/ SkpSkp,   /*243*/ SkpSkp,   /*244*/ SkpSkp,
/*245*/ SkpSkp,   /*246*/ SkpSkp,   /*247*/ SkpSkp,   /*248*/ SkpSkp,
/*249*/ SkpSkp,   /*250*/ SkpSkp,   /*251*/ SkpSkp,   /*252*/ SkpSkp,
/*253*/ SkpSkp,   /*254*/ SkpSkp,   /*255*/ SkpSkp,   /*256*/ SkpSkp
	};
	DEFAULT Status;
#if DEBUGGING
	static char *DbgFNm = "SkpCmd";
	sprintf(DbgSBf,"*CBfPtr = '%c', at_sign is %s",
		*CBfPtr, (CmdMod & ATSIGN) ? "TRUE" : "FALSE");
	DbgFEn(3,DbgFNm,DbgSBf);
#endif
	Status = (*FSAray[*CBfPtr])();
#if DEBUGGING
	sprintf(DbgSBf,"*CBfPtr = '%c'", *CBfPtr);
	DbgFEx(3,DbgFNm,DbgSBf);
#endif
	return Status;
}
