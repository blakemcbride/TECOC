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
static DEFAULT SkpArg(void);	/* skip command with text argument */
static DEFAULT SkpCrt(void);	/* skip a ^ (caret) command */
static DEFAULT SkpCtA(void);	/* skip control-A command */
static DEFAULT SkpCtU(void);	/* skip control-U command */
static DEFAULT SkpDAr(void);	/* skip "double text argument" command */
static DEFAULT SkpDqu(void);	/* skip a " (double quote) command */
static DEFAULT SkpE(void);	/* skip one of the E commands */
static DEFAULT SkpExc(void);	/* skip command with text argument */
static DEFAULT SkpF(void);	/* skip one of the F commands */
static DEFAULT SkpOne(void);	/* skip one command character */
static DEFAULT SkpSkp(void);	/* */
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
*****************************************************************************/
static DEFAULT SkpCtU()		/* skip control-U command */
{
	BOOLEAN	TTrace;		/* temp: holds trace flag */
	if (CBfPtr == CStEnd)
		if (MStTop < 0)			/* if not in a macro */
			{
			ErrUTC();		/* unterminated command */
			return FAILURE;
			}
		else
			return SUCCESS;
	++CBfPtr;
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
	return SUCCESS;
}
/*****************************************************************************
	SkpDqu()
*****************************************************************************/
static DEFAULT SkpDqu()		/* skip a " (double quote) command */
{
	DBGFEN(3,"SkpDqu",NULL);
	if (CBfPtr == CStEnd)			/* if end of command string */
		if (MStTop < 0)			/* if not in a macro */
			{
			ErrUTC();		/* unterminated command */
			return FAILURE;
			}
		else
			return SUCCESS;
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
	SkpF()
*****************************************************************************/
static DEFAULT SkpF()			/* skip one of the F commands */
{
	if (CBfPtr == CStEnd)
		if (MStTop < 0)			/* if not in a macro */
			{
			ErrUTC();		/* unterminated command */
			return FAILURE;
			}
		else
			return SUCCESS;
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
		if (MStTop < 0) {		/* if not in a macro */
			ErrUTC();		/* unterminated command */
			DBGFEX(3,DbgFNm,"FAILURE");
			return FAILURE;
		} else {
			DBGFEX(3,DbgFNm,"SUCCESS");
			return SUCCESS;
		}
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
*****************************************************************************/
static DEFAULT SkpCrt()		/* skip a ^ (caret) command */
{
	static DEFAULT (*FCAray[])(VVOID) = {
/* ^A*/ SkpCtA,   /* ^B*/ SkpSkp,   /* ^C*/ SkpSkp,   /* ^D*/ SkpSkp,
/* ^E*/ SkpSkp,   /* ^F*/ SkpSkp,   /* ^G*/ SkpSkp,   /* ^H*/ SkpSkp,
/*TAB*/ SkpArg,   /* LF*/ ExeNul,   /* VT*/ SkpSkp,   /* FF*/ SkpSkp,
/* CR*/ ExeNul,   /* ^N*/ SkpSkp,   /* ^O*/ SkpSkp,   /* ^P*/ SkpSkp,
/* ^Q*/ SkpSkp,   /* ^R*/ SkpSkp,   /* ^S*/ SkpSkp,   /* ^T*/ SkpSkp,
/* ^U*/ SkpCtU,   /* ^V*/ SkpSkp,   /* ^W*/ SkpSkp,   /* ^X*/ SkpSkp,
/* ^Y*/ SkpSkp,   /* ^Z*/ SkpSkp,   /* ^[*/ SkpSkp,   /* ^\*/ SkpSkp,
/* ^]*/ SkpSkp,   /* ^^*/ SkpOne,   /* ^_*/ SkpSkp
	};
	WORD	CDummy;
	if (CBfPtr == CStEnd)			/* if end of command string */
		if (MStTop < 0)			/* if macro stack empty */
			{
			ErrUTC();		/* unterminated command */
			return FAILURE;
			}
		else
			return SUCCESS;
	++CBfPtr;
	if ((*CBfPtr >= 'A') && (*CBfPtr <= '_'))
		CDummy = *CBfPtr - 'A';
	else if (Is_Lower(*CBfPtr))
		CDummy = *CBfPtr - 'a';		/* convert to upper case */
	else {
		ErrMsg(ERR_IUC);		/* illegal char after ^ */
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
	static DEFAULT (*FEAray[])(VVOID) = {
/* A */ SkpSkp,    /* B */ SkpArg,    /* C */ SkpSkp,    /* D */ SkpSkp,
/* E */ 0,         /* F */ SkpSkp,    /* G */ SkpArg,    /* H */ SkpSkp,
/* I */ SkpArg,    /* J */ SkpSkp,    /* K */ SkpSkp,    /* L */ SkpArg,
/* M */ SkpSkp,    /* N */ SkpSkp,    /* O */ SkpSkp,    /* P */ SkpSkp,
/* Q */ SkpOne,    /* R */ SkpArg,    /* S */ SkpSkp,    /* T */ SkpSkp,
/* U */ SkpSkp,    /* V */ SkpSkp,    /* W */ SkpArg,    /* X */ SkpSkp,
/* Y */ SkpSkp,    /* Z */ SkpSkp
	};
	if (CBfPtr == CStEnd)
		if (MStTop < 0)			/* if not in a macro */
			{
			ErrUTC();		/* unterminated command */
			return FAILURE;
			}
		else
			return SUCCESS;
	++CBfPtr;
	TmpChr = To_Upper(*CBfPtr);
	if (TmpChr == '%')
		return SkpOne();
	else if (TmpChr == '_')
		return SkpArg();
	else if (!Is_Upper(TmpChr) || (TmpChr=='E'))
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
*****************************************************************************/
DEFAULT SkpCmd()		/* skip a "command" */
{
	static DEFAULT (*FSAray[])(VVOID) = {
/*NUL*/ ExeNul,   /* ^A*/ SkpCtA,   /* ^B*/ SkpSkp,   /* ^C*/ SkpSkp,
/* ^D*/ SkpSkp,   /* ^E*/ SkpSkp,   /* ^F*/ SkpSkp,   /* ^G*/ SkpSkp,
/* BS*/ SkpSkp,   /*TAB*/ SkpArg,   /* LF*/ ExeNul,   /* VT*/ SkpSkp,
/* FF*/ SkpSkp,   /* CR*/ ExeNul,   /* ^N*/ SkpSkp,   /* ^O*/ SkpSkp,
/* ^P*/ SkpSkp,   /* ^Q*/ SkpSkp,   /* ^R*/ SkpSkp,   /* ^S*/ SkpSkp,
/* ^T*/ SkpSkp,   /* ^U*/ SkpCtU,   /* ^V*/ SkpSkp,   /* ^W*/ SkpSkp,
/* ^X*/ SkpSkp,   /* ^Y*/ SkpSkp,   /* ^Z*/ SkpSkp,   /*ESC*/ SkpSkp,
/* ^\*/ SkpSkp,   /* ^]*/ SkpSkp,   /* ^^*/ SkpOne,   /* ^_*/ SkpSkp,
/* SP*/ ExeNul,   /* ! */ SkpExc,   /* " */ SkpDqu,   /* # */ SkpSkp,
/* $ */ SkpSkp,   /* % */ SkpOne,   /* & */ SkpSkp,   /* ' */ SkpSkp,
/* ( */ SkpSkp,   /* ) */ SkpSkp,   /* * */ SkpSkp,   /* + */ SkpSkp,
/* , */ SkpSkp,   /* - */ SkpSkp,   /* . */ SkpSkp,   /* / */ SkpSkp,
/* 0 */ SkpSkp,   /* 1 */ SkpSkp,   /* 2 */ SkpSkp,   /* 3 */ SkpSkp,
/* 4 */ SkpSkp,   /* 5 */ SkpSkp,   /* 6 */ SkpSkp,   /* 7 */ SkpSkp,
/* 8 */ SkpSkp,   /* 9 */ SkpSkp,   /* : */ SkpSkp,   /* ; */ SkpSkp,
/* < */ SkpSkp,   /* = */ SkpSkp,   /* > */ SkpSkp,   /* ? */ SkpSkp,
/* @ */ ExeAtS,   /* A */ SkpSkp,   /* B */ SkpSkp,   /* C */ SkpSkp,
/* D */ SkpSkp,   /* E */ SkpE,     /* F */ SkpF,     /* G */ SkpOne,
/* H */ SkpSkp,   /* I */ SkpArg,   /* J */ SkpSkp,   /* K */ SkpSkp,
/* L */ SkpSkp,   /* M */ SkpOne,   /* N */ SkpArg,   /* O */ SkpArg,
/* P */ SkpSkp,   /* Q */ SkpOne,   /* R */ SkpSkp,   /* S */ SkpArg,
/* T */ SkpSkp,   /* U */ SkpOne,   /* V */ SkpSkp,   /* W */ SkpSkp,
/* X */ SkpOne,   /* Y */ SkpSkp,   /* Z */ SkpSkp,   /* [ */ SkpOne,
/* \ */ SkpSkp,   /* ] */ SkpOne,   /* ^ */ SkpCrt,   /* _ */ SkpArg,
/* ` */ SkpSkp,   /* a */ SkpSkp,   /* b */ SkpSkp,   /* c */ SkpSkp,
/* d */ SkpSkp,   /* e */ SkpE,     /* f */ SkpF,     /* g */ SkpOne,
/* h */ SkpSkp,   /* i */ SkpArg,   /* j */ SkpSkp,   /* k */ SkpSkp,
/* l */ SkpSkp,   /* m */ SkpOne,   /* n */ SkpArg,   /* o */ SkpArg,
/* p */ SkpSkp,   /* q */ SkpOne,   /* r */ SkpSkp,   /* s */ SkpArg,
/* t */ SkpSkp,   /* u */ SkpOne,   /* v */ SkpSkp,   /* w */ SkpSkp,
/* x */ SkpOne,   /* y */ SkpSkp,   /* z */ SkpSkp,   /* { */ SkpSkp,
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
