/*****************************************************************************
	DefExt.h
	This header file declares as EXTERN all the variables declared as
GLOBAL in Tecoc.c for use by everyone else.
*****************************************************************************/
EXTERN	charptr	AraBeg;		/* beginning of text m,n area */
EXTERN	charptr	AraEnd;		/* end of text m,n area */
EXTERN	charptr	ArgPtr;		/* beginning of text argument */
EXTERN	charptr	CBfBeg;		/* command buffer beginning */
EXTERN	charptr	CBfEnd;		/* command buffer end */
EXTERN	charptr	CBfPtr;		/* pointer into command string */
EXTERN	char	CmdMod;		/* command modifiers flags for @, ;, etc. */
EXTERN	DEFAULT	CrType;		/* value of 0:W (terminal type) */
EXTERN	charptr	CStBeg;		/* command string beginning */
EXTERN	charptr	CStEnd;		/* pointer to last char of command string */
EXTERN	DEFAULT	CurInp;		/* index of current input stream in IFiles */
EXTERN	DEFAULT	CurOut;		/* index of current output stream in OFiles */
EXTERN	charptr	DBfBeg;		/* digit buffer beginning */
EXTERN	charptr	DBfPtr;		/* digit buffer pointer */
#if DEBUGGING
EXTERN	char	DbgSBf[];	/* debug message sprintf() buffer */
#endif
EXTERN	charptr	EBfBeg;		/* edit buffer beginning */
EXTERN	charptr	EBfEnd;		/* edit buffer end */
EXTERN	charptr	EBPtr1;		/* pointer to start of found string */
EXTERN	charptr	EBPtr2;		/* pointer to end of found string */
EXTERN	WORD	EdFlag;		/* ED mode control flag */
EXTERN	WORD	EhFlag;		/* EH mode control flag */
EXTERN	charptr	EndSAr;		/* end of search area */
EXTERN	char	ErrTxt[];	/* holds part of error message */
EXTERN	WORD	EsFlag;		/* ES mode control flag */
EXTERN	WORD	EStBot;		/* expression stack bottom */
EXTERN	struct	EStck EStack[]; /* expression stack */
EXTERN	WORD	EStTop;		/* expression stack top */
EXTERN	WORD	EtFlag;		/* ET mode control flag */
EXTERN	WORD	EuFlag;		/* EU mode control flag */
EXTERN	WORD	EvFlag;		/* EV mode control flag */
EXTERN	WORD	EzFlag;		/* EZ mode control flag */
EXTERN	charptr	FBfBeg;		/* beginning of filename buffer */
EXTERN	charptr	FBfEnd;		/* end of filename buffer */
EXTERN	charptr	FBfPtr;		/* pointer into filename buffer */
EXTERN	LONG	FFPage;		/* form feed flag (returned by ^E command) */
EXTERN	charptr	GapBeg;		/* edit buffer gap beginning */
EXTERN	charptr	GapEnd;		/* end of edit buffer gap */
EXTERNV	BOOLEAN	GotCtC;		/* TRUE if the user just hit a control-C */
EXTERN	charptr	IBfEnd;		/* end of input buffer */
EXTERN	charptr	IBfEnd;		/* input buffer end */
EXTERN	int	IniSrM;		/* initial search mode */
EXTERN	BOOLEAN	IsEofI[];	/* input file has reached end of file */
EXTERN	BOOLEAN	IsOpnI[];	/* input file is opened indicators */
EXTERN	BOOLEAN	IsOpnO[];	/* output file is opened indicators */
EXTERN	WORD	LStBot;		/* loop stack bottom */
EXTERN	struct	LStck LStack[];	/* loop stack */
EXTERN	struct	LStck LStack[]; /* loop stack */
EXTERN	WORD	LstErr;		/* number of last error message */
EXTERN	WORD	LStTop;		/* loop stack top */
EXTERN	LONG	MArgmt;		/* m part of m,n numeric argument pair */
EXTERN	BOOLEAN	Matchd;		/* indicates successful search */
EXTERN	struct	MStck MStack[]; /* macro stack */
EXTERN	WORD	MStTop;		/* macro stack top */
EXTERN	LONG	NArgmt;		/* numeric argument */
EXTERN	QRptr	QR;		/* pointer to q-register structure */
EXTERN	struct	QReg QRgstr[];	/* global q-registers */
EXTERN	struct	QReg QStack[];	/* q-register stack */
EXTERN	WORD	QStTop;		/* top of q-register stack */
EXTERN	DEFAULT	Radix;		/* TECO-C's current radix, 2-36 */
EXTERN	LONG	RefLen;		/* value returned by ^S command */
EXTERN	charptr	RhtSid;		/* right-hand side of area to be searched */
EXTERN	charptr	SBfBeg;		/* search string buffer beginning */
EXTERN	charptr	SBfEnd;		/* search string buffer end */
EXTERN	charptr	SBfPtr;		/* end of search string, plus one */
EXTERN	LONG	SIncrm;		/* search increment, 1 (forward), -1 (back) */
EXTERN	WORD	SMFlag;		/* search mode flag */
EXTERN	WORD	SrcTyp;		/* type of search (E_SEARCH, N_SEARCH, etc) */
EXTERN	charptr	SStPtr;		/* pointer into search string */
EXTERN	BOOLEAN	TraceM;		/* trace mode flag */
#if VIDEO
EXTERN DEFAULT	HldFlg;		/* value of 5:W (hold mode flag) */
EXTERN DEFAULT	HtSize;		/* value of 1:W (screen width) */
EXTERN DEFAULT	MrkFlg;		/* value of 4:W */
EXTERN DEFAULT	ScroLn;		/* value of 7:W */
EXTERN DEFAULT	SeeAll;		/* value of 3:W */
EXTERN DEFAULT	TopDot;		/* value of 6:W */
EXTERN DEFAULT	VtSize;		/* value of 2:W (screen height) */
#if CURSES
EXTERN DEFAULT	SpcMrk;		/* value of 8:W */
EXTERN DEFAULT	KeyPad;		/* value of 9:W */
#endif
#endif
