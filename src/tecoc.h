/*****************************************************************************
	Tecoc.h

	This file contains most of the definitions needed by TECO-C files.
*****************************************************************************/

#define TVERSION 147		/* version number of TECO-C */

/*****************************************************************************
	Bit masks for ED mode control flag.  The ones that are commented out
are not used in TECO-C.  They are included here because they are used by
TECO-11, and may be implemented in TECO-C someday.

*****************************************************************************/

#define ED_CARET_OK	1	/* don't use "^" as control-char prefix */
#define ED_YANK_OK	2	/* let yanks clear text buffer */
/*#define ED_NO_EXP	4*/	/* don't allow arbitrary expansions */
#define ED_PRES_DOT	16	/* preserve "dot" on failing searches */
/*#define ED_IMMED_OK	32*/	/* allow immediate escape-sequence commands */
#define ED_DOT_BY_ONE	64	/* move dot by one on iter. search failure */
/*#define ED_NO_W	128*/	/* don't do automatic "^W" before prompt */
#define ED_FF           128     /* do not stop read on FF */
#if defined(unix) || defined(AMIGA)
#define ED_UNIXNL	256	/* use Unix-style newline line terminators */
#endif
#define ED_VTGR		512
#define ED_NOSCDEL	1024	/* old style delete handler */
#define ED_BTEE		2048	/* use BTEE instead of DIAMOND */
#define ED_STDOUT	4096	/* use standout for doeves pointer */
#define ED_INVCR	8192	/* don't show CR in scope - closer to TECO-11
	but really not as good in my opinion (Mark Henderson)*/

/*****************************************************************************
	Value for EH flag.
*****************************************************************************/

#define EH_COMMAND	4	/* on error,  output failing command string */

/*****************************************************************************
	Bit masks for ET mode control flag.  The ones that are commented out
are not used in TECO-C.  They are included here because they are used by
TECO-11.  The 2048 bit is listed as "reserved for TECO-8" in the TECO-11
source code.  I needed another bit, so I used it.
*****************************************************************************/

#define ET_IMAGE_MODE	1	/* output without doing conversions */
#define ET_SCOPE	2	/* use scope for delete and control-U */
#define ET_READ_LOWER	4	/* accept lowercase input */
#define ET_NO_ECHO	8	/* don't echo input on control-T */
/*#define ET_CAN_CTRL_O	16*/	/* (TECO-11) cancel control-O on output */
#define ET_NO_WAIT	32	/* read with no wait on control-T */
/*#define ET_DETACH	64*/	/* process detached flag */
#define ET_MUNG_MODE	128	/* mung mode */
/*#define ET_TRUNCATE	256*/	/* (TECO-11) truncate lines on output */
#define ET_WAT_SCOPE	512	/* "WATCH" code exists, terminal is scope */
#define ET_WAT_REFRESH	1024	/* "WATCH" code exists, terminal is refresh */
#define ET_BKSP_IS_DEL	2048	/* treat BACKSPACE like DEL on input */
#define ET_EIGHTBIT	4096	/* it's an eight-bit terminal */
#define ET_ACCENT_GRAVE	8192	/* accept ` as escape (for VT220 terminals) */
#define ET_VT200	16384	/* special VT200 mode */
#define ET_TRAP_CTRL_C	0100000	/* trap control-C */

/*****************************************************************************
	Values for EU flag.
*****************************************************************************/

#define EU_NONE		-1	/* no case flagging of any type */
#define EU_LOWER	0	/* flag lowercase characters on output */
#define EU_UPPER	1	/* flag uppercase characters on output */

/*****************************************************************************
	These identifiers are bit masks for elements of the ChrMsk array.
The ChrMsk array is used by the macros defined in the ChrMac.h file and by
the ReadCS function to make the character input code fast.  See the comment
above the definition of the ChrMsk array in file TECOC.C.
*****************************************************************************/

#define RCS_AST		'\1'	/* asterisk */
#define RCS_BS		'\2'	/* backspace */
#define RCS_CCH		'\3'	/* control char other than ^G, ^U or ^Z */
#define RCS_CR		'\4'	/* carriage return */
#define RCS_CTG		'\5'	/* control-G */
#define RCS_CTU		'\6'	/* control-U */
#define RCS_CTZ		'\7'	/* control-Z */
#define RCS_DEF		'\10'	/* char other than RCS_AST, RCS_BS, etc. */
#define RCS_DEL		'\11'	/* delete */
#define RCS_GRV		'\12'	/* accent grave */
#define RCS_LF		'\13'	/* line feed */
#define RCS_LWR		'\14'	/* lowercase character */
#define RCS_SP		'\15'	/* space */
#define RCS_VF		'\16'	/* vertical tab or form feed */
#define RCS_CTC		'\17'	/* control-C */
#define CM_DIGIT	'\20'	/* digits */
#define CM_LINE_TERM	'\40'	/* line terminators (lf, vt, ff) */
#define CM_LOWER	'\100'	/* lowercase */
#define CM_UPPER	'\200'	/* uppercase */

/*****************************************************************************
	These identifiers are set by the search commands.  They provide a way
for the search commands to all rely on common code,  even when the commands
behave slightly differently.  It's a terrible way to code,  but it was only
done in the search code.
*****************************************************************************/

#define E_SEARCH  1	/* E_ command */
#define FB_SEARCH 2	/* FB command */
#define FK_SEARCH 3	/* FK command */
#define N_SEARCH  4	/* N command */
#define S_SEARCH  5	/* S command */
#define U_SEARCH  6	/* _ command */

/*****************************************************************************
	Identifiers for indices into the IFiles and OFiles arrays.
*****************************************************************************/

#define	NOFDBS	3	/* number of output file data blocks */

#define	POUTFL	0	/* primary output stream */
#define	SOUTFL	1	/* secondary output stream */
#define	EPRCFL	2	/* output stream used by E% command */

#define	PINPFL	0	/* primary input stream */
#define	SINPFL	1	/* secondary input stream */
#define	EQFL	2	/* input stream used by EQ command */
#define EIFL	3	/* first input stream used by EI command */

/*****************************************************************************
	Identifiers for :EG special function types.
*****************************************************************************/

#define EG_INI		1	/* inititalization macro */
#define EG_LIB		2	/* default macro library directory */
#define EG_MEM		3	/* last-file-edited memory */
#define EG_VTE		4	/* filename of video macro */
#define EG_OTHER	5	/* system-specific :EG command */
#define CLEAR_VAL	1
#define GET_VAL		2
#define SET_VAL		3

/*****************************************************************************
	Function return codes.
*****************************************************************************/

#define FAILURE   (-1)	/* function return status */
#define SUCCESS   0	/* function return status */
#define FILENF    3	/* "file not found", returned by ZSWild and ZOpInp */

/*****************************************************************************
	CmdMod Bits.
*****************************************************************************/

#define ATSIGN    '\1'	/* mask of "at sign" bit in CmdMod */
#define COLON     '\2'	/* mask of "colon" bit in CmdMod */
#define DCOLON    '\4'	/* mask of "double colon" bit in CmdMod */
#define MARGIS    '\10'	/* mask of "m defined" bit in CmdMod */

/*****************************************************************************
	Arguments to ZSetTT function.
*****************************************************************************/

#define TT8BIT	 1	/* whether terminal can handle 8-bit characters */
#define TTWIDTH  2	/* number of columns of charactes */
#define TTHEIGHT 3	/* number of rows of characters */

/*****************************************************************************
	Miscellaneous identifiers.
*****************************************************************************/

#define FFLINS    5	/* number of blank lines displayed for form feed */
#define INFINITE  -1L	/* loop count value for infinite loops */

#define PRIMARY   '\0'	/* primary input or output stream specifier */
#define SECNDARY  '\1'	/* secondary input or output stream specifier */

#define OPERAND   0	/* means element is an integer */
#define OPERATOR  1	/* means element is + - * / & or | */

#define LOWER     0	/* case conversion mode specifier, for ^V, ^W */
#define NONE      1	/* case conversion mode specifier */
#define UPPER     2	/* case conversion mode specifier, for ^V, ^W */

/*****************************************************************************
	define structure for expression stack
*****************************************************************************/

struct EStck {
	LONG Elemnt;		/* integer or operand character */
	int ElType;		/* Entry's type: OPERATOR or OPERAND */
};

/*****************************************************************************
	define structure for loop stack
*****************************************************************************/

struct LStck {
	LONG	LIndex;		/* loop repetition count */
	charptr LAddr;		/* pointer to start of loop in command buf */
};

/*****************************************************************************
	define structure for q-registers
*****************************************************************************/

struct QReg  {
	charptr	Start;		/* start of text */
	charptr	End_P1;		/* end of text, plus one */
	LONG	Number;		/* numeric part of q-register */
};

/*****************************************************************************
	define structure for macro stack
*****************************************************************************/

struct MStck {
	charptr	CStBeg;		/* saves CStBeg */
	charptr	CBfPtr;		/* saves CBfPtr */
	charptr	CStEnd;		/* saves CStEnd */
	WORD	EStBot;		/* saves EStBot */
	WORD	EStTop;		/* saves EStTop */
	WORD	LStBot;		/* saves LStBot */
	WORD	LStTop;		/* saves LStTop */
	QRptr	QRgstr;		/* local q-register table */
};

/*****************************************************************************
	Debugging macros
*****************************************************************************/

#if DEBUGGING
#define DBGFEN(lv,fnm,msg) static char *DbgFNm = fnm; DbgFEn(lv, DbgFNm, msg)
#define DBGFEX(lv,fnm,msg) DbgFEx(lv, fnm, msg)
#define DBGFMS(lv,fnm,msg) DbgFMs(lv, fnm, msg)
#else
#define DBGFEN(lv,fnm,msg)
#define DBGFEX(lv,fnm,msg)
#define DBGFMS(lv,fnm,msg)
#endif

/*****************************************************************************
	function definitions
*****************************************************************************/

#if USE_PROTOTYPES

#if CHECKSUM_CODE
VVOID	init_code_checksums(void);
VVOID	check_code_checksums(void);
#endif

#if CONSISTENCY_CHECKING
VVOID	init_consistency_check(void);
VVOID	check_consistency(void);
#endif

DEFAULT	BakSrc(void);		/* backwards search to match 1st character */
DEFAULT BldStr(			/* build a string */
	charptr XBfBeg,		/* beginning of build-string buffer */
	charptr XBfEnd,		/* end of build-string buffer */
	charptr (*XBfPtr));	/* pointer into build-string buffer */
VVOID	ClenUp(void);		/* clean up for TECO-C abort */
DEFAULT	CMatch(			/* match a character */
	BOOLEAN *SamChr);	/* returned match indicator */
#if DEBUGGING
DEFAULT	DbgDsp(void);		/* debugging, control debugging display (^P) */
VVOID	DbgFEn(			/* debugging, function entry */
	int DbgFLv,             /*   function display level */
	char *DbgFNm,		/*   function name */
	char *DbgMsg);		/*   an entry message, usually parameters */
VVOID	DbgFEx(			/* debugging, function exit */
	int DbgFLv,		/*   function display level */
	char *DbgFNm,		/*   function name */
	char *DbgMsg);		/*   an exit message, usually return value */
VVOID	DbgFMs(			/* debugging, function message */
	int DbgFLv,		/*   function display level */
	char *DbgFNm,		/*   function name */
	char *DbgMsg);		/*   an message */
#endif
DEFAULT	DoCJR(			/* do C, J or R stuff */
	LONG HowFar);		/* positive or negative displacement */
VVOID	DoEvEs(			/* do an EV or ES mode control flag function */
	WORD Flag);
DEFAULT	DoFlag(			/* handle mode control flag */
	WORD *Flag);
VVOID	EchoIt(			/* echo a character to to terminal */
	unsigned char Charac);
VVOID	ErrChr(			/* display error message with character arg */
	WORD ErrNum,		/* the error number */
	unsigned char EChr);	/* character to imbed in message */
VVOID	ErrMsg(			/* display error message */
	WORD ErrNum);		/* the error number */
VVOID	ErrPSt(			/* display error message with string arg */
	WORD ErrNum,		/* the error number */
	charptr EBeg,		/* string to imbed in message */
	charptr EPtr);		/* ptr to end-of-string, plus one */
VVOID	ErrStr(			/* display error message with string arg */
	WORD ErrNum,		/* the error number */
	char *EStr);		/* string to imbed in message */
VVOID	ErrUTC(void);		/* unterminated command or macro */
VVOID	ErrVrb(			/* display a verbose error message */
	WORD ErrNum);		/* error message number */
DEFAULT	ExeA(void);		/* execute an A command */
DEFAULT	ExeAtS(void);		/* execute an at-sign (@) command */
DEFAULT	ExeB(void);		/* execute a B command */
DEFAULT	ExeBar(void);		/* execute a bar (|) command */
DEFAULT	ExeBSl(void);		/* execute a backslash (\) command */
DEFAULT	ExeC(void);		/* execute a C command */
DEFAULT	ExeCCC(void);		/* execute a control-^ command */
DEFAULT	ExeCln(void);		/* execute a colon (:) command */
DEFAULT	ExeCom(void);		/* execute a comma (,) command */
DEFAULT	ExeCrt(void);		/* execute a caret (^) command */
DEFAULT	ExeCSt(void);		/* execute a command string */
DEFAULT	ExeCtA(void);		/* execute a control-A command */
DEFAULT	ExeCtC(void);		/* execute a control-C command */
DEFAULT	ExeCtD(void);		/* execute a control-D command */
DEFAULT	ExeCtE(void);		/* execute a control-E command */
DEFAULT	ExeCtI(void);		/* execute a control-I (tab) command */
DEFAULT	ExeCtL(void);		/* execute a control-L (form feed) command */
DEFAULT	ExeCtN(void);		/* execute a control-N command */
DEFAULT	ExeCtO(void);		/* execute a control-O command */
DEFAULT	ExeCtP(void);		/* execute a control-P command */
DEFAULT	ExeCtQ(void);		/* execute a control-Q command */
DEFAULT	ExeCtR(void);		/* execute a control-R command */
DEFAULT	ExeCtS(void);		/* execute a control-S command */
DEFAULT	ExeCtT(void);		/* execute a control-T command */
DEFAULT	ExeCtU(void);		/* execute a control-U command */
DEFAULT	ExeCtV(void);		/* execute a control-V command */
DEFAULT	ExeCtW(void);		/* execute a control-W command */
DEFAULT	ExeCtX(void);		/* execute a control-X command */
DEFAULT	ExeCtY(void);		/* execute a control-Y command */
DEFAULT	ExeCtZ(void);		/* execute a control-Z command */
DEFAULT	ExeD(void);		/* execute a D command */
DEFAULT	ExeDgt(void);		/* execute a digit command */
DEFAULT	ExeDot(void);		/* execute a dot (.) command */
DEFAULT	ExeDQu(void);		/* execute a double-quote (") command */
DEFAULT	ExeE(void);		/* execute on of the E commands */
DEFAULT	ExeEqu(void);		/* execute an equals-sign (=) command */
DEFAULT	ExeEsc(void);		/* execute an ESCAPE command */
DEFAULT	ExeExc(void);		/* execute an exclamation-mark (!) command */
DEFAULT	ExeEY(void);		/* execute an EY command */
DEFAULT	ExeF(void);		/* execute one of the F commands */
DEFAULT	ExeFB(void);		/* execute an FB command */
DEFAULT	ExeG(void);		/* execute a G command */
DEFAULT	ExeGtr(void);		/* execute a greater-than (>) command */
DEFAULT	ExeH(void);		/* execute an H command */
DEFAULT	ExeI(void);		/* execute an I command */
DEFAULT	ExeIll(void);		/* execute an "illegal" command */
DEFAULT	ExeJ(void);		/* execute a J command */
DEFAULT	ExeK(void);		/* execute a K command */
DEFAULT	ExeK(void);		/* execute a K command */
DEFAULT	ExeL(void);		/* execute an L command */
DEFAULT	ExeLBr(void);		/* execute a left-bracket ([) command */
DEFAULT	ExeLst(void);		/* execute a less-than (<) command */
DEFAULT	ExeM(void);		/* execute an M command */
DEFAULT	ExeN(void);		/* execute a N command */
DEFAULT	ExeNul(void);		/* execute a null command */
DEFAULT	ExeNYI(void);		/* "not yet implemented" function */
DEFAULT	ExeO(void);		/* execute an O command */
DEFAULT	ExeOpr(void);		/* execute a operator (+,-,*, etc.) command */
DEFAULT	ExeP(void);		/* execute a P or PW command */
DEFAULT	ExePrc(void);		/* execute a percent {%} command */
DEFAULT	ExePW(void);		/* execute a PW command */
DEFAULT	ExeQ(void);		/* execute a Q command */
DEFAULT	ExeQes(void);		/* execute a question-mark (?) command */
DEFAULT	ExeR(void);		/* execute an R command */
DEFAULT	ExeRBr(void);		/* execute a right-bracket (]) command */
DEFAULT	ExeRtP(void);		/* execute a right-parenthesis command */
DEFAULT	ExeS(void);		/* execute an S command */
DEFAULT	ExeSCl(void);		/* execute a semi-colon (;) command */
DEFAULT	ExeT(void);		/* execute a T command */
DEFAULT	ExeU(void);		/* execute a U command */
DEFAULT	ExeUnd(void);		/* execute an underbar (_) command */
DEFAULT	ExeUsc(void);		/* execute control-_ command */
DEFAULT	ExeV(void);		/* execute a V command */
DEFAULT	ExeW(void);		/* execute a W command */
DEFAULT	ExeX(void);		/* execute a X command */
DEFAULT	ExeY(void);		/* execute a Y command */
DEFAULT	ExeZ(void);		/* execute a Z command */
DEFAULT	FindES(			/* find end of string */
	unsigned char TrmChr);	/* termination char if no @ modifier */
DEFAULT	FindQR(void);		/* find a q-register index */
DEFAULT	FlowEC(void);		/* flow to end of conditional */
DEFAULT	FlowEE(void);		/* flow to | or ' */
DEFAULT	FlowEL(void);		/* flow to end of loop */
DEFAULT	GetAra(void);		/* get m,n area */
DEFAULT	GetNmA(void);		/* get numeric argument */
DEFAULT	IncCBP(void);		/* increment CBfPtr */
VVOID	Init(			/* initialize TECO-C */
	int argc,
	char *argv[]);
DEFAULT	InsStr(			/* insert string into edit buffer */
	charptr string,
	ptrdiff_t length);
BOOLEAN	IsRadx(			/* is the character in the radix set? */
	unsigned char Charac);
LONG	Ln2Chr(			/* convert line offset to character offset */
	LONG Value);
VVOID	MakDBf(			/* make digit buffer */
	LONG Binary,		/* binary number to be converted */
	DEFAULT NRadix);	/* radix to be used: 8, 10 or 16 */
DEFAULT	MakRom(			/* make room in q-register */
	SIZE_T Amount);		/* how much room to make */
DEFAULT	PopMac(void);		/* pop variables after macro call */
DEFAULT	PshMac(			/* push variables for macro call */
	charptr Start,		/* start of new command string */
	charptr End);		/* end of new command string, plus one */
DEFAULT	PushEx(			/* push onto expression stack */
	LONG Item,
	DEFAULT EType);
DEFAULT	RdLine(			/* read a line */
	BOOLEAN *EBfFul);	/* returned "edit buffer is full" indicator */
DEFAULT	RdPage(void);		/* read a page from a file */
VVOID	ReadCS(void);		/* read command string */
DEFAULT	Replac(void);		/* replace a string */
DEFAULT	Search(			/* top level search */
	BOOLEAN	RepCmd);	/* TRUE if the command has two arguments */
DEFAULT	SinglP(void);		/* execute a single P function */
DEFAULT	SkpCmd(void);		/* skip a single command */
DEFAULT	SrcLop(void);		/* search loop */
DEFAULT	SSerch(void);		/* single search */
VVOID	TAbort(			/* terminate TECO-C */
	DEFAULT tstat);
VVOID	TypBuf(			/* type a buffer on the terminal */
	charptr YBfBeg,
	charptr YBfEnd);
VVOID	TAbort(			/* cleanup for TECO-C abort */
	DEFAULT tstat);
VVOID	TypESt(void);		/* type error string on terminal */
VVOID	UMinus(void);		/* handle unary minus */
DEFAULT	WrPage(			/* write to output file */
	DEFAULT	OfIndx,		/* index into OFiles array */
	charptr OBfBeg,		/* address of output buffer beginning */
	charptr OBfEnd,		/* address of output buffer end */
	LONG AddFF);		/* -1 means add form feed, 0 means don't */
voidptr ZAlloc(			/* like standard malloc function */
	SIZE_T MemSiz);
VVOID	ZBell(void);		/* ring the terminal bell */
DEFAULT ZChIn(			/* input a character from the terminal */
	BOOLEAN	NoWait);	/* return immediately? */
LONG	ZClnEG(			/* execute special :EG command */
	DEFAULT EGWhat,		/* what to get/set/clear: MEM, LIB, etc. */
	DEFAULT EGOper,		/* operation: get, set or clear */
	charptr TxtPtr);	/* if setting,  value to set */
VVOID	ZClnUp(void);		/* cleanup for TECO-C abort */
#if DEBUGGING || CONSISTENCY_CHECKING
ULONG	Zcp2ul(			/* convert voidptr to unsigned long */
	voidptr cp);
#endif
VVOID	ZDoCmd(			/* terminate and pass command string to OS */
	charptr GBfBeg,
	charptr GBfPtr);	
VVOID	ZDspBf(			/* output a buffer to the terminal */
	charptr buffer,
	SIZE_T length);
VVOID	ZDspCh(			/* display a character */
	char Charac);
DEFAULT	ZExCtB(void);		/* execute a ^B command */
DEFAULT	ZExCtH(void);		/* execute a ^H command */
DEFAULT	ZExeEJ(void);		/* execute an EJ command */
VVOID	ZExit(			/* terminate TECO-C */
	DEFAULT estat);
VVOID	ZFree(			/* like standard free() function */
	voidptr pointer);
DEFAULT	ZFrSrc(void);		/* forward search to match 1st character */
VVOID	ZHelp(			/* display a help message */
	charptr HlpBeg,		/* first char of help request */
	charptr HlpEnd,		/* last character of help request */
	BOOLEAN SysLib,		/* use default HELP library? */
	BOOLEAN	Prompt);	/* enter interactive help mode? */
VVOID	ZIClos(			/* close input file */
	DEFAULT	IfIndx);	/* index into IFiles array */
VVOID	ZOClDe(			/* close and delete output file */
	DEFAULT	OfIndx);	/* index into OFiles array */
VVOID	ZOClos(			/* close output file */
	DEFAULT	OfIndx);	/* index into OFiles array */
DEFAULT	ZOpInp(			/* open input file */
	DEFAULT IfIndx,		/* index into file data block array IFiles */
	BOOLEAN	EIFile,		/* is it a macro file? (hunt for it) */
	BOOLEAN RepFNF);	/* report "file not found" error? */
DEFAULT ZOpOut(			/* open output file */
	DEFAULT	OfIndx,		/* output file indicator */
	BOOLEAN RepErr,		/* report errors? */
	BOOLEAN Backup);	/* create backup? TAA MOD */
VVOID	ZPrsCL(			/* parse the command line */
	int argc,
	char **argv);
DEFAULT	ZPWild(void);		/* preset the wildcard lookup filespec */
voidptr	ZRaloc(			/* re-allocate memory*/
	voidptr OldBlk,
	SIZE_T NewSiz);
DEFAULT ZRdLin(			/* read a line */
	charptr ibuf,		/* where to put string */
	ptrdiff_t ibuflen,	/* max length of buf */
	int IfIndx,		/* index into IFiles[] */
	DEFAULT *retlen);	/* returned length of string */
VVOID	ZScrOp(			/* do a screen operation */
	int OpCode);		/* code for operation */
DEFAULT	ZSetTT(			/* set a terminal parameter */
	DEFAULT TTWhat,		/* what terminal parameter to set */
	DEFAULT TTVal);		/* new value for the terminal parameter */
DEFAULT	ZSWild(void);		/* search for next wildcard filename */
VVOID	ZTrmnl(void);		/* open terminal input and output */
VVOID	ZVrbos(			/* display verbose form of an error message */
	WORD ErrNum,		/* error number (see DEFERR.h) */
	char *ErMnem);		/* error mnemonic */
DEFAULT	ZWrBfr(			/* write a buffer */
	DEFAULT	OfIndx,		/* index into OFiles array */
	charptr BfrBeg,		/* address of output buffer beginning */
	charptr BfrEnd);	/* address of output buffer end */


#else				/* else (not USE_PROTOTYPES) */

#if CHECKSUM_CODE
VVOID	init_code_checksums ();
VVOID	check_code_checksums ();
#endif

#if CONSISTENCY_CHECKING
VVOID	init_consistency_check();
VVOID	check_consistency();
#endif

DEFAULT	BakSrc();	/* backwards search to match 1st character */
DEFAULT	BldStr();	/* build a string */
VVOID	ClenUp();	/* clean up for TECO-C abort */
DEFAULT	CMatch();	/* match a character */
#if DEBUGGING
DEFAULT	DbgDsp();	/* debugging, control debugging display */
VVOID	DbgFEn();	/* debugging, function entry */
VVOID	DbgFEx();	/* debugging, function exit */
VVOID	DbgFMs();	/* debugging, function message */
#endif
DEFAULT	DoCJR();	/* do C, J or R stuff */
VVOID	DoEvEs();	/* do an EV or ES mode control flag function */
DEFAULT	DoFlag();	/* handle mode control flag */
VVOID	EchoIt();	/* echo a character to to terminal */
VVOID	ErrChr();	/* display error message with character argument */
VVOID	ErrMsg();	/* display error message */
VVOID	ErrPSt();	/* display error message with string argument */
VVOID	ErrStr();	/* display error message with string argument */
VVOID	ErrUTC();	/* unterminated command or macro */
VVOID	ErrVrb();	/* display a verbose error message */
DEFAULT	ExeA();		/* execute an A command */
DEFAULT	ExeAtS();	/* execute an at-sign (@) command */
DEFAULT	ExeB();		/* execute a B command */
DEFAULT	ExeBar();	/* execute a bar (|) command */
DEFAULT	ExeBSl();	/* execute a backslash (\) command */
DEFAULT	ExeC();		/* execute a C command */
DEFAULT	ExeCCC();	/* execute a control-^ command */
DEFAULT	ExeCln();	/* execute a colon (:) command */
DEFAULT	ExeCom();	/* execute a comma (,) command */
DEFAULT	ExeCrt();	/* execute a caret (^) command */
DEFAULT	ExeCSt();	/* execute a command string */
DEFAULT	ExeCtA();	/* execute a control-A command */
DEFAULT	ExeCtC();	/* execute a control-C command */
DEFAULT	ExeCtD();	/* execute a control-D command */
DEFAULT	ExeCtE();	/* execute a control-E command */
DEFAULT	ExeCtI();	/* execute a control-I (tab) command */
DEFAULT	ExeCtL();	/* execute a control-L (form feed) command */
DEFAULT	ExeCtN();	/* execute a control-N command */
DEFAULT	ExeCtO();	/* execute a control-O command */
DEFAULT	ExeCtP();	/* execute a control-P command */
DEFAULT	ExeCtQ();	/* execute a control-Q command */
DEFAULT	ExeCtR();	/* execute a control-R command */
DEFAULT	ExeCtS();	/* execute a control-S command */
DEFAULT	ExeCtT();	/* execute a control-T command */
DEFAULT	ExeCtU();	/* execute a control-U command */
DEFAULT	ExeCtV();	/* execute a control-V command */
DEFAULT	ExeCtW();	/* execute a control-W command */
DEFAULT	ExeCtX();	/* execute a control-X command */
DEFAULT	ExeCtY();	/* execute a control-Y command */
DEFAULT	ExeCtZ();	/* execute a control-Z command */
DEFAULT	ExeD();		/* execute a D command */
DEFAULT	ExeDgt();	/* execute a digit command */
DEFAULT	ExeDot();	/* execute a dot (.) command */
DEFAULT	ExeDQu();	/* execute a double-quote (") command */
DEFAULT	ExeE();		/* execute on of the E commands */
DEFAULT	ExeEqu();	/* execute an equals-sign (=) command */
DEFAULT	ExeEsc();	/* execute an ESCAPE command */
DEFAULT	ExeExc();	/* execute an exclamation-mark (!) command */
DEFAULT	ExeEY();	/* execute an EY command */
DEFAULT	ExeF();		/* execute one of the F commands */
DEFAULT	ExeFB();	/* execute an FB command */
DEFAULT	ExeG();		/* execute a G command */
DEFAULT	ExeGtr();	/* execute a greater-than (>) command */
DEFAULT	ExeH();		/* execute an H command */
DEFAULT	ExeI();		/* execute an I command */
DEFAULT	ExeIll();	/* execute an "illegal" command */
DEFAULT	ExeJ();		/* execute a J command */
DEFAULT	ExeK();		/* execute a K command */
DEFAULT	ExeK();		/* execute a K command */
DEFAULT	ExeL();		/* execute an L command */
DEFAULT	ExeLBr();	/* execute a left-bracket ([) command */
DEFAULT	ExeLst();	/* execute a less-than (<) command */
DEFAULT	ExeM();		/* execute an M command */
DEFAULT	ExeN();		/* execute a N command */
DEFAULT	ExeNul();	/* execute a null command */
DEFAULT	ExeNYI();	/* "not yet implemented" function */
DEFAULT	ExeO();		/* execute an O command */
DEFAULT	ExeOpr();	/* execute a operator (+,-,*, etc.) command */
DEFAULT	ExeP();		/* execute a P or PW command */
DEFAULT	ExePrc();	/* execute a percent {%} command */
DEFAULT	ExePW();	/* execute a PW command */
DEFAULT	ExeQ();		/* execute a Q command */
DEFAULT	ExeQes();	/* execute a question-mark (?) command */
DEFAULT	ExeR();		/* execute an R command */
DEFAULT	ExeRBr();	/* execute a right-bracket (]) command */
DEFAULT	ExeRtP();	/* execute a right-parenthesis command */
DEFAULT	ExeS();		/* execute an S command */
DEFAULT	ExeSCl();	/* execute a semi-colon (;) command */
DEFAULT	ExeT();		/* execute a T command */
DEFAULT	ExeU();		/* execute a U command */
DEFAULT	ExeUnd();	/* execute an underbar (_) command */
DEFAULT	ExeUsc();	/* execute control-_ command */
DEFAULT	ExeV();		/* execute a V command */
DEFAULT	ExeW();		/* execute a W command */
DEFAULT	ExeX();		/* execute a X command */
DEFAULT	ExeY();		/* execute a Y command */
DEFAULT	ExeZ();		/* execute a Z command */
DEFAULT	FindES();	/* find end of string */
DEFAULT	FindQR();	/* find a q-register index */
DEFAULT	FlowEC();	/* flow to end of conditional */
DEFAULT	FlowEE();	/* flow to | or ' */
DEFAULT	FlowEL();	/* flow to end of loop */
DEFAULT	GetAra();	/* get m,n area */
DEFAULT	GetNmA();	/* get numeric argument */
DEFAULT	IncCBP();	/* increment CBfPtr */
VVOID	Init();		/* initialize TECO-C */
DEFAULT	InsStr();	/* insert string into edit buffer */
BOOLEAN	IsRadx();	/* is the character in the radix set? */
LONG	Ln2Chr();	/* convert line offset to character offset */
VVOID	MakDBf();	/* make digit buffer */
DEFAULT	MakRom();	/* make room in q-register */
DEFAULT	PopMac();	/* pop variables after macro call */
DEFAULT	PshMac();	/* push variables for macro call */
DEFAULT	PushEx();	/* push onto expression stack */
DEFAULT	RdLine();	/* read a line */
DEFAULT	RdPage();	/* read a page from a file */
VVOID	ReadCS();	/* read command string */
DEFAULT	Replac();	/* replace a string */
DEFAULT	Search();	/* top level search */
DEFAULT	SinglP();	/* execute a single P function */
DEFAULT	SkpCmd();	/* skip a single command */
DEFAULT	SrcLop();	/* search loop */
DEFAULT	SSerch();	/* single search */
VVOID	TAbort();	/* terminate TECO-C */
VVOID	TypBuf();	/* type a buffer on the terminal */
VVOID	TypESt();	/* type error string on terminal */
VVOID	UMinus();	/* handle unary minus */
DEFAULT	WrPage();	/* write to output file */
voidptr ZAlloc();	/* like standard malloc function */
VVOID	ZBell();	/* ring the terminal bell */
DEFAULT ZChIn();	/* input a character from the terminal */
LONG	ZClnEG();	/* execute special :EG command */
VVOID	ZClnUp();	/* cleanup for TECO-C abort */
#if DEBUGGING || CONSISTENCY_CHECKING
ULONG	Zcp2ul();	/* convert charptr to unsigned long */
#endif
VVOID	ZDoCmd();	/* terminate and pass command string to OS */
VVOID	ZDspBf();	/* output a buffer to the terminal */
VVOID	ZDspCh();	/* display a character */
DEFAULT	ZExCtB();	/* execute a ^B command */
DEFAULT	ZExCtH();	/* execute a ^H command */
DEFAULT	ZExeEJ();	/* execute an EJ command */
VVOID	ZExit();	/* terminate TECO-C */
VVOID	ZFree();	/* like standard free() function */
DEFAULT	ZFrSrc();	/* forward search to match 1st character */
VVOID	ZHelp();	/* display a help message */
VVOID	ZIClos();	/* close input file */
VVOID	ZOClDe();	/* close and delete output file */
VVOID	ZOClos();	/* close output file */
DEFAULT	ZOpInp();	/* open input file */
DEFAULT ZOpOut();	/* open output file */
VVOID	ZPrsCL();	/* parse the command line */
DEFAULT	ZPWild();	/* preset the wildcard lookup filespec */
voidptr	ZRaloc();	/* re-allocate memory*/
DEFAULT	ZRdLin();	/* read a line */
VVOID	ZScrOp();	/* do a screen operation */
DEFAULT	ZSetTT();	/* set a terminal parameter */
DEFAULT	ZSWild();	/* search for next wildcard filename */
VVOID	ZTrmnl();	/* open terminal input and output */
VVOID	ZVrbos();	/* display verbose form of an error message */
DEFAULT	ZWrBfr();	/* write line to file */
#endif
#if CURSES
void Scope();
void centre();
void dolf();
void dobs();
void ccs();
void do_right();
void do_left();
void do_sf();
void do_sr();
void do_down();
void do_up();
void do_seetog();
void redraw();
void keypad_on();
void keypad_off();
#endif
