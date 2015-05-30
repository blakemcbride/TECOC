/*****************************************************************************
	TECO-C (version number is defined as TVERSION in file TECOC.H)
	Copyright 1983, 1990 by Pete Siemsen. This software is provided to
you free of charge for your own use.  Use it as you see fit; if it doesn't
work, I disclaim all responsibility.  You may re-distribute this software
UNCHANGED only if you include this copy-right notice.  Alternatively, if
you change this software, you may re-distribute the result only if you
include BOTH this copyright notice, AND an additional notice identifying
you and indicating that you have changed the software.
	This program is still under development.  See file PROBLEMS.TXT for
notes I've written to myself about things to do to the program. If you
modify this code to enhance it or fix a bug,  please communicate the changes
to me.  My address is
			Pete Siemsen
			645 Ohio Avenue #302
			Long Beach, Ca. 90814
			(213) 433-3059  (home)
			(213) 740-7391  (work)
			Internet: siemsen@usc.edu
	The file PG.MEM (programmer's guide) contains documentation explaining
algorithms used in TECO-C.  File PROBLEMS.TXT is a notes file listing ideas
and known bugs.
	Global variable declarations for system-independent variables are
in this file.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
#include "dscren.h"		/* define UNTERM */
/*****************************************************************************
        Test for inconsistencies in the identifiers defined in the include
files.  The ANSI C way to complain is to use #error,  but some compilers
will complain even when the tests succeed,  because they don't recognize the
#error directive while they scan for #endif.  The goofy "include" statements
are used to generate a compile-time error.
*****************************************************************************/
#if GAPMIN > EBFINIT
#include ">>> GAPMIN can't be greater than EBFINIT <<<"
#endif
#if IBFMIN > IBFINIT
#include ">>> IBFMIN can't be greater than IBFINIT <<<"
#endif
#if IBFEXP < IBFMIN
#include ">>> IBFEXP can't be less than IBFMIN <<<"
#endif
#if NIFDBS < 3
#include ">>> NIFDBS can't be less than 3 <<<"
#endif
#if ZBFEXP < ZBFMIN
#include ">>> ZBFEXP can't be less than ZBFMIN <<<"
#endif
/*****************************************************************************
	These pointers point to the first and last characters in an "area",
which is specified when an m,n argument pair precedes a TECO command.  For
instance,  when the command 10,25T is executed,  the GetAra function is called
to set these pointers,  which are then used to display the text.
*****************************************************************************/
GLOBAL	charptr	AraBeg;		/* start of m,n area */
GLOBAL	charptr	AraEnd;		/* end of m,n area */
/*****************************************************************************
	ArgPtr points to the first character of the text argument associated
with a TECO-C command.  It is set by the FindES (find end-of-string) function.
When a command that has a text argument is executed,  FindES is called to
locate the end of the text argument.  FindES moves the command pointer CBfPtr
to the end of the text argument,  and remembers where the beginning was using
ArgPtr.
*****************************************************************************/
GLOBAL	charptr	ArgPtr;		/* start of text argument of a cmd */
/*****************************************************************************
	These variables point to the beginning and end of the command string
buffer.  Memory for the command string buffer is allocated in the MemIni
function.  CBfBeg always points to this buffer,  but CStBeg changes when an M
or EI command is executed.
*****************************************************************************/
GLOBAL	charptr	CBfBeg;		/* command buffer beginning */
GLOBAL	charptr	CBfEnd;		/* command buffer end */
/*****************************************************************************
	CBfPtr is the command string buffer pointer,  which moves across a
command string as it is executed.  CBfPtr usually points into the command
string buffer pointed to by CBfBeg,  but it points elsewhere when an M or EI
command is being executed.
*****************************************************************************/
GLOBAL	charptr	CBfPtr;		/* command buffer pointer */
/*****************************************************************************
	When TECO-C is executing a command string,  it looks at each command
(1 or 2 characters) and calls a function to implement the command.  Some
commands can be modified by preceding the command with a modifier character.
CmdMod contains bits which are set when a modifier character is "executed".
This variable is cleared by most commands before they successfully return.
The bits in CmdMod relate to the at-sign (@), colon (:), double-colon (::)
modifiers.  There is also a bit which indicates that a command has 2 numeric
arguments separated by a comma.
*****************************************************************************/
GLOBAL	char	CmdMod;
/*****************************************************************************
	These variables point to the beginning and end of the executing
command string,  whether that command string is in the command buffer, a
q-register or the EI buffer.
*****************************************************************************/
GLOBAL	charptr	CStBeg;		/* start of command string */
GLOBAL	charptr	CStEnd;		/* end of command string */
/*****************************************************************************
	CurInp and CurOut are indices into the IFiles and OFiles arrays,
respectively.  They indicate the entries for the current input stream and
current output stream.
*****************************************************************************/
GLOBAL	DEFAULT	CurInp = PINPFL;
GLOBAL	DEFAULT	CurOut = POUTFL;
/*****************************************************************************
	These variables point to the digit buffer.  When TECO-C needs to
convert a binary number into an ASCII string,  the string is generated in this
buffer.  The buffer is large enough to hold the largest integer represented
by 32 bits,  plus an optional sign and maybe a carriage-return/line-feed pair.
*****************************************************************************/
GLOBAL	charptr	DBfBeg;		/* digit buffer */
GLOBAL	charptr	DBfPtr;		/* digit buffer pointer */
/*****************************************************************************
	These variables point to the first and last characters in the edit
buffer.  See file PG.MEM for a description of memory management.
*****************************************************************************/
GLOBAL	charptr	EBfBeg;		/* first character in edit buffer */
GLOBAL	charptr	EBfEnd;		/* last character in edit buffer */
/*****************************************************************************
	EBPtr1 is used when a search is being performed.  It is adjusted when
the edit buffer is being scanned for the first character of the search string.
When a search command succeeds,  this pointer is left pointing to the first
character of the found string.
*****************************************************************************/
GLOBAL	charptr	EBPtr1;
/*****************************************************************************
	EBPtr2 is used when a search is being performed.  After a character
matching the first character of the search string is found,  this pointer is
adjusted as each remaining character of the search string is compared with
the edit buffer.  When a search command succeeds,  this pointer is left
pointing to the last character of the found string.
*****************************************************************************/
GLOBAL	charptr	EBPtr2;
/*****************************************************************************
	Bits within EdFlag have the following meanings:
        1    Allow caret (^) in search strings
        2    Allow Y and _ commands to destroy edit buffer
        4    Don't arbitrarily expand memory
        16   preserve dot on failing searches
        64   only move dot by one on multiple occurrence searches
*****************************************************************************/
GLOBAL	WORD	EdFlag = 0;		/* ED mode control flag */
/*****************************************************************************
	Bits within EhFlag have the following meanings:
        3    how much error message to display
        4    display failing command string after errors
*****************************************************************************/
GLOBAL	WORD	EhFlag = 0;		/* EH mode control flag */
/*****************************************************************************
	EndSAr is used by search code to point to the end of the area to be
searched.
*****************************************************************************/
GLOBAL	charptr	EndSAr;			/* end of search area */
/*****************************************************************************
	Several TECO error messages take an argument.  For instance,  the
"illegal command"  error message shows the user the illegal command.  When
these kinds of messages are required,  ErrTxt is used to send the text
of the argument to the error display function (ErrMsg).
*****************************************************************************/
GLOBAL	char	ErrTxt[6];		/* holds part of error message */
/*****************************************************************************
	The EsFlag flag controls what is displayed on the terminal after every
successful search command completes.  EsFlag has the following meanings:
        0         don't display anything
        -1        display the line containing the found string
        1-31      display the line containing the found string,
                  with a line feed at the character position
        32-126    display the line containing the found string,
                  with the character whose ASCII code is represented
                  by ES at the character position
        m*265+n   n has the meanings defined above.  m is the number
                  of lines above and below the found string to be
                  displayed.
*****************************************************************************/
GLOBAL	WORD	EsFlag = 0;		/* ES mode control flag */
/*****************************************************************************
	The expression stack contains the components of TECO's numeric
expressions.  Each entry on the expression stack is either an OPERAND or
an OPERATOR (like +, -, /, *).
*****************************************************************************/
GLOBAL	struct	EStck EStack[EXS_SIZE]; /* expression stack */
GLOBAL	WORD	EStBot;			/* expression stack bottom */
GLOBAL	WORD	EStTop;			/* top of expression stack */
/*****************************************************************************
	Bits in EtFlag control TECO's handling of the terminal.  Definitions
of masks for each bit and the meaning of each bit can be found in TECOC.H.
Bits in EtFlag are initialized when the terminal is set up in the ZTrmnl
function.
*****************************************************************************/
GLOBAL	WORD	EtFlag = 0;		/* ET mode control flag */
/*****************************************************************************
	The EuFlag is TECO's case mode control flag.  This flag allows TECO to
be used to input and output upper and lower case characters even if the
terminal being used is capable of displaying only uppercase characters.  If
the EU flag is -1, no case flagging is performed on type-out.  If the EU flag
is 0,  lowercase characters are converted to uppercase on type-out,  and are
preceded by a ' character.  If the EU flag is 1, then lowercase characters
are converted to uppercase on type-out,  but uppercase characters are
preceded by a ' character.
*****************************************************************************/
GLOBAL	WORD	EuFlag = EU_NONE;	/* EU mode control flag */
/*****************************************************************************
	The EV mode control flag controls what is displayed on the terminal
after every successful command string completes.  By default, nothing is
displayed (EV is 0).  EV has the following meanings:
        0         don't display anything
        -1        display the line containing the character position
        1-31      display the line containing the character position,
                  with a line feed at the character position
        32-126    display the line containing the character position,
                  with the character whose ASCII code is represented
                  by ES at the character position
        m*265+n   n has the meanings defined above.  m is the number
                  of lines above and below the character position to be
                  displayed.
*****************************************************************************/
GLOBAL	WORD	EvFlag = 0;		/* EV mode control flag */
/*****************************************************************************
	These variables point to the filename buffer.
*****************************************************************************/
GLOBAL	charptr	FBfBeg;			/* first char of filename buffer */
GLOBAL	charptr	FBfEnd;			/* last chr (+1) of filename buffer */
GLOBAL	charptr	FBfPtr;			/* pointer into file spec buffer */
/*****************************************************************************
	Bits in the EzFlag are system-specific.  See the system-specific
file for each operating system (like ZVMS.C, ZUNIX.C, etc) for the meanings
of these bits.
*****************************************************************************/
GLOBAL	WORD	EzFlag = 0;		/* EZ mode control flag */
/*****************************************************************************
	FFPage is the value which is returned by the ^E command.  It is -1 if
the buffer currently contains a page of text that was terminated by a form
feed in the input file.  It is 0 if the buffer contains only part of a page
from the input file (because the input page filled the text buffer before it
was completely read in).  The FFPage flag is tested by the P command and
related operations to determine if a form feed should be appended to the
contents of the buffer when it is output.
*****************************************************************************/
GLOBAL	LONG	FFPage = 0;		/* form feed flag (see ^E command) */
/*****************************************************************************
	These variables point to the first and last characters in the edit
buffer gap.  See file PG.MEM for a description of memory management.
*****************************************************************************/
GLOBAL	charptr	GapBeg;			/* first char in edit buffer gap */
GLOBAL	charptr	GapEnd;			/* last char in edit buffer gap */
/*****************************************************************************
	GotCtC indicates that the user has interrupted the normal execution
of TECO-C by typing a control-C.  This variable is set by the special
interrupt-handling function executed when a control-C is struck,  and is
cleared before a command string is entered.  It is tested each time a command
terminates and when certain commands execute time-consuming loops.
*****************************************************************************/
GLOBALV	BOOLEAN	GotCtC = FALSE;
/*****************************************************************************
	IBfEnd is the end of the input buffer.  See file PG.MEM for a
description of memory management.
*****************************************************************************/
GLOBAL	charptr	IBfEnd;
/*****************************************************************************
	IniSrM is the initial search mode.  It is used when parsing search
strings and filenames.  The value of this variable is set by ^V and
^W characters and is used to explicitly control the case (upper or lower) of
characters in a string.  It can take on the values LOWER, UPPER or NONE,
which are defined in TECOC.H.
*****************************************************************************/
GLOBAL	int	IniSrM = NONE;
/*****************************************************************************
	IsOpnI and IsOpnO contain indicators about the status of the file
data blocks in the IFiles and OFiles arrays,  respectively.  An element has
the value TRUE when the corresponding file data block reflects an opened file.
IsEofI indicates that the corresponding element of IFiles reflects a file
that has reached the end.
*****************************************************************************/
GLOBAL	BOOLEAN	IsEofI[NIFDBS];
GLOBAL	BOOLEAN	IsOpnI[NIFDBS];
GLOBAL	BOOLEAN	IsOpnO[NOFDBS];
/*****************************************************************************
	LstErr holds the code for the last error reported by TECO-C.  It is
set in the error display function (ErrMsg) and used when the user executes
the ? or / immediate mode commands,  to print out more information about the
last error that occurred.
*****************************************************************************/
GLOBAL	WORD	LstErr = ERR_XXX;	/* number of last error message */
/*****************************************************************************
	The loop stack maintains pointers to the first character of each loop
(the character following the "<").  LStTop is the element of LStack that
relates to the most recently executed "<" command.  LStBot is the index of
the bottom of the stack,  so that if (LStTop == LStBot),  we're not in a loop
in the current macro level.  When a macro is entered,  LStTop and LStBot are
saved on the macro stack,  and LStBot is set to LStTop,  so that LStBot
defines the bottom of the LStack frame for the new macro level.
*****************************************************************************/
GLOBAL	WORD	LStBot;			/* bottom of loop stack */
GLOBAL	struct	LStck LStack[LPS_SIZE]; /* loop stack */
GLOBAL	WORD	LStTop;			/* top of loop stack */
/*****************************************************************************
	MArgmt contains the m part of an m,n argument pair that precedes a
TECO command.  For instance,  MArgmt would contain 10 when the command
10,45T is being executed.
*****************************************************************************/
GLOBAL	LONG	MArgmt;			/* m part of m,n numeric arguments */
/*****************************************************************************
	MAtchd is used to indicate that a match has been found for a search
string.  It is only used when a search command is being executed.
*****************************************************************************/
GLOBAL	BOOLEAN	Matchd;			/* indicates successful search */
/*****************************************************************************
	These variables implement the macro stack.  When TECO executes a
macro,  it must first save the current state,  so that the state can be
restored when the macro has finished executing.  Each entry in the macro
stack preserves the critical variables for one macro call.
*****************************************************************************/
GLOBAL	struct	MStck MStack[MCS_SIZE]; /* macro stack */
GLOBAL	WORD	MStTop = -1;		/* top of macro stack */
/*****************************************************************************
	NArgmt holds the numeric argument that precedes a command.  It is set
by a call to the GetNmA function.  When the command is preceded by an m,n
argument pair,  NArgmt holds the value of the n argument.
*****************************************************************************/
GLOBAL	LONG	NArgmt;			/* n argument (n part of m,n) */
/*****************************************************************************
	QR is a pointer to a structure which defines a q-register.
It is set by calling the FindQR function.  Whenever a command that uses a
q-register is executed,  FindQR is called first to set QR.
*****************************************************************************/
GLOBAL	QRptr	QR;
/*****************************************************************************
	The structures in the QRgstr array represent the global and main-level
local q-registers.  The 0-9 elements are for global digit q-registers (0-9),
the 10-35 elements are for global alphabetic q-registers (a-z or A-Z),  the
36-45 elements are for main-level local digit q-registers (.0-.9) and the
46-71 elements are for main-level local alphabetic q-registers (.a-.z or
.A-.Z).  Local q-registers at macro levels other than the main level are
allocated dynamically.
*****************************************************************************/
GLOBAL	struct	QReg QRgstr[72];
/*****************************************************************************
	QStack is the q-register stack,  accessed via the [ and ] commands.
*****************************************************************************/
GLOBAL	struct	QReg QStack[QRS_SIZE];	/* q-register stack */
GLOBAL	WORD	QStTop = -1;		/* top of q-register stack */
/*****************************************************************************
	RefLen is the length of the last inserted string or found search
string.  It is accessed by the ^S command.
*****************************************************************************/
GLOBAL	LONG	RefLen = 0;		/* returned by ^S */
/*****************************************************************************
	Radix is TECO's radix,  usually 10.  It is set by the ^D, ^O and ^R
commands.
*****************************************************************************/
GLOBAL	DEFAULT	Radix = 10;		/* TECO's current radix */
/*****************************************************************************
	When searching,  this pointer points to the farthest-right character
in the area to be searched.  It is used after the first character of a string
has been found,  to indicate the limit of the area in the edit buffer that
should be checked against the remaining characters in the search string.
*****************************************************************************/
GLOBAL	charptr	RhtSid;
/*****************************************************************************
	These variables point to the search string buffer.  The search buffer
is allocated by function MemIni.
*****************************************************************************/
GLOBAL	charptr	SBfBeg = NULL;		/* start of search buffer */
GLOBAL	charptr	SBfEnd;			/* end search buffer */
GLOBAL	charptr	SBfPtr;			/* end of search string buffer */
/*****************************************************************************
	SIncrm is the value added to the search pointer when the edit buffer
is being searched.  If SIncrm is 1,  then the search proceeds in a forward
direction.  If SIncrm is -1,  then the search proceeds in a backward
direction.  By using this variable,  the rather complex code that implements
searching can easily be used to search in either direction.
*****************************************************************************/
GLOBAL	LONG	SIncrm;			/* search increment */
/*****************************************************************************
	SMFlag is TECOC's search mode flag,  and controls case sensitivity
during searches.  This variable holds the value of the ^X command.
*****************************************************************************/
GLOBAL	WORD	SMFlag = 0;		/* search mode control flag (^X) */
/*****************************************************************************
	SrcTyp is used to "remember" the kind of search command that we're
working on.  The same search code is used by all the search commands,  but
each search command is slightly different than the others.  The common
search code tests this variable to implement the differences.
*****************************************************************************/
GLOBAL	WORD	SrcTyp;			/* type of search (E_SEARCH, etc) */
/*****************************************************************************
	SStPtr points into the search buffer,  and is used only by the search
code.
*****************************************************************************/
GLOBAL	charptr	SStPtr;		/* search string pointer */
/*****************************************************************************
	This table serves two functions.  It is used by the character macros
defined in CHMACS.H.  It is also used by the command line input code in
function ReadCS.
	The top four bits of each mask are used by the functions in CHMACS.H.
The top four bits are masks used to represent "uppercase", "lowercase",
"digit" and "line terminator".
	The bottom four bits of each mask are used by function ReadCS.  They
are treated as a number and are used in a SWITCH statement.  In ReadCS,  the
top four bits of each mask are not used.
*****************************************************************************/
unsigned char ChrMsk[256] =
		{
		RCS_DEF,		/* null */
		RCS_CCH,		/* ^A */
		RCS_CCH,		/* ^B */
		RCS_CTC,		/* ^C */
		RCS_CCH,		/* ^D */
		RCS_CCH,		/* ^E */
		RCS_CCH,		/* ^F */
		RCS_CTG,		/* ^G (bell) */
		RCS_BS,			/* ^H (bs) */
		RCS_DEF,		/* ^I (tab) */
		RCS_LF | CM_LINE_TERM,	/* ^J (lf) */
		RCS_VF | CM_LINE_TERM,	/* ^K (vt) */
		RCS_VF | CM_LINE_TERM,	/* ^L (ff) */
		RCS_CR,			/* ^M (cr) */
		RCS_CCH,		/* ^N */
		RCS_CCH,		/* ^O */
		RCS_CCH,		/* ^P */
		RCS_CCH,		/* ^Q */
		RCS_CCH,		/* ^R */
		RCS_CCH,		/* ^S */
		RCS_CCH,		/* ^T */
		RCS_CTU,		/* ^U */
		RCS_CCH,		/* ^V */
		RCS_CCH,		/* ^W */
		RCS_CCH,		/* ^X */
		RCS_CCH,		/* ^Y */
		RCS_CTZ,		/* ^Z */
		0,			/* escape */
		RCS_CCH,		/* FS */
		RCS_CCH,		/* GS */
		RCS_CCH,		/* RS */
		RCS_CCH,		/* US */
		RCS_SP,			/* space */
		RCS_DEF,		/* ! */
		RCS_DEF,		/* " */
		RCS_DEF,		/* # */
		RCS_DEF,		/* $ */
		RCS_DEF,		/* % */
		RCS_DEF,		/* | */
		RCS_DEF,		/* ' */
		RCS_DEF,		/* ( */
		RCS_DEF,		/* ) */
		RCS_AST,		/* * */
		RCS_DEF,		/* + */
		RCS_DEF,		/* , */
		RCS_DEF,		/* - */
		RCS_DEF,		/* . */
		RCS_DEF,		/* / */
		RCS_DEF | CM_DIGIT,	/* 0 */
		RCS_DEF | CM_DIGIT,	/* 1 */
		RCS_DEF | CM_DIGIT,	/* 2 */
		RCS_DEF | CM_DIGIT,	/* 3 */
		RCS_DEF | CM_DIGIT,	/* 4 */
		RCS_DEF | CM_DIGIT,	/* 5 */
		RCS_DEF | CM_DIGIT,	/* 6 */
		RCS_DEF | CM_DIGIT,	/* 7 */
		RCS_DEF | CM_DIGIT,	/* 8 */
		RCS_DEF | CM_DIGIT,	/* 9 */
		RCS_DEF,		/* : */
		RCS_DEF,		/* ; */
		RCS_DEF,		/* < */
		RCS_DEF,		/* = */
		RCS_DEF,		/* > */
		RCS_DEF,		/* ? */
		RCS_DEF,		/* @ */
		RCS_DEF | CM_UPPER,	/* A */
		RCS_DEF | CM_UPPER,	/* B */
		RCS_DEF | CM_UPPER,	/* C */
		RCS_DEF | CM_UPPER,	/* D */
		RCS_DEF | CM_UPPER,	/* E */
		RCS_DEF | CM_UPPER,	/* F */
		RCS_DEF | CM_UPPER,	/* G */
		RCS_DEF | CM_UPPER,	/* H */
		RCS_DEF | CM_UPPER,	/* I */
		RCS_DEF | CM_UPPER,	/* J */
		RCS_DEF | CM_UPPER,	/* K */
		RCS_DEF | CM_UPPER,	/* L */
		RCS_DEF | CM_UPPER,	/* M */
		RCS_DEF | CM_UPPER,	/* N */
		RCS_DEF | CM_UPPER,	/* O */
		RCS_DEF | CM_UPPER,	/* P */
		RCS_DEF | CM_UPPER,	/* Q */
		RCS_DEF | CM_UPPER,	/* R */
		RCS_DEF | CM_UPPER,	/* S */
		RCS_DEF | CM_UPPER,	/* T */
		RCS_DEF | CM_UPPER,	/* U */
		RCS_DEF | CM_UPPER,	/* V */
		RCS_DEF | CM_UPPER,	/* W */
		RCS_DEF | CM_UPPER,	/* X */
		RCS_DEF | CM_UPPER,	/* Y */
		RCS_DEF | CM_UPPER,	/* Z */
		RCS_DEF,		/* [ */
		RCS_DEF,		/* \ */
		RCS_DEF,		/* ] */
		RCS_DEF,		/* ^ */
		RCS_DEF,		/* _ */
		RCS_GRV,		/* ` */
		RCS_LWR | CM_LOWER,	/* a */
		RCS_LWR | CM_LOWER,	/* b */
		RCS_LWR | CM_LOWER,	/* c */
		RCS_LWR | CM_LOWER,	/* d */
		RCS_LWR | CM_LOWER,	/* e */
		RCS_LWR | CM_LOWER,	/* f */
		RCS_LWR | CM_LOWER,	/* g */
		RCS_LWR | CM_LOWER,	/* h */
		RCS_LWR | CM_LOWER,	/* i */
		RCS_LWR | CM_LOWER,	/* j */
		RCS_LWR | CM_LOWER,	/* k */
		RCS_LWR | CM_LOWER,	/* l */
		RCS_LWR | CM_LOWER,	/* m */
		RCS_LWR | CM_LOWER,	/* n */
		RCS_LWR | CM_LOWER,	/* o */
		RCS_LWR | CM_LOWER,	/* p */
		RCS_LWR | CM_LOWER,	/* q */
		RCS_LWR | CM_LOWER,	/* r */
		RCS_LWR | CM_LOWER,	/* s */
		RCS_LWR | CM_LOWER,	/* t */
		RCS_LWR | CM_LOWER,	/* u */
		RCS_LWR | CM_LOWER,	/* v */
		RCS_LWR | CM_LOWER,	/* w */
		RCS_LWR | CM_LOWER,	/* x */
		RCS_LWR | CM_LOWER,	/* y */
		RCS_LWR | CM_LOWER,	/* z */
		RCS_DEF,		/* { */
		RCS_DEF,		/* | */
		RCS_DEF,		/* } */
		RCS_DEF,		/* ~ */
		RCS_DEL,		/* delete */
	RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,
	RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,
	RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,
	RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,
	RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,
	RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,
	RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,
	RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,
	RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,
	RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,
	RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,
	RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,
	RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,
	RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,
	RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,
	RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF,RCS_DEF
	};
/*****************************************************************************
	TraceM is the trace mode flag.  This variable is set to TRUE when
TECO's trace mode is on.  When it is set,  commands are echoed to the terminal
as they are executed.  This mode is used to aid in debugging TECO macros,  and
is toggled by the ? command.
	If you want to trace the command line parsing macro executed in
ZPrsCL(), set this to TRUE; otherwise, set it to FALSE.
*****************************************************************************/
GLOBAL	BOOLEAN	TraceM = FALSE;		/* trace mode flag */
/*****************************************************************************
	CrType is the terminal type.  It is used by the ZScrOp function to
drive the few screen capabilities of TECOC.  See ZScrOp for a list of the
values that CrType can take.
*****************************************************************************/
GLOBAL	DEFAULT	CrType = UNTERM;	/* terminal type */
#if VIDEO
GLOBAL DEFAULT	HldFlg;			/* value of 5:W (hold mode flag) */
GLOBAL DEFAULT	HtSize;			/* value of 1:W (screen width) */
GLOBAL DEFAULT	MrkFlg;			/* value of 4:W */
GLOBAL DEFAULT	ScroLn;			/* value of 7:W */
GLOBAL DEFAULT	SeeAll;			/* value of 3:W */
GLOBAL DEFAULT	TopDot;			/* value of 6:W */
GLOBAL DEFAULT	VtSize;			/* value of 2:W (screen height) */
#if CURSES
GLOBAL DEFAULT	SpcMrk = 0;		/* value of 8:W */
GLOBAL DEFAULT	KeyPad;			/* value of 9:W */
#endif
#endif
/*****************************************************************************
	S t a r t   H e r e
*****************************************************************************/
int _Cdecl main(argc, argv)		/* _Cdecl defined in ZPORT.H */
int argc;
char **argv;
{
#if CHECKSUM_CODE			/* MS-DOS debugging code */
	init_code_checksums ();		/* calculate code checksums */
#endif
	Init(argc, argv);		/* initialize TECO */
/*
 * read a command string and then execute it.  This loop is infinite.
 * TECO-C is exited explicitly when a lower-level function calls TAbort.
 */
	CStBeg = CStEnd = CBfBeg;
	CStEnd--;
	FOREVER {			/* loop forever */
		ReadCS();		/* read command string */
		CBfPtr = CBfBeg;	/* initialize command string ptr */
		CmdMod = '\0';		/* clear modifiers flags */
		EStTop = EStBot =	/* clear expression stack */
		LStTop = LStBot =	/* clear loop stack */
		MStTop = -1;		/* clear macro stack */
		ExeCSt();		/* execute command string */
	}
}
/*****************************************************************************
	The following code is support routines for the imbedded debugging code
in TECO-C.  If the "DEBUGGING" macro in ZPORT.H is set to "FALSE", then none
of this code is compiled.
*****************************************************************************/
#if DEBUGGING
#include <ctype.h>		/* isprint() macro */
#include "dchars.h"		/* define identifiers for characters */
static	DEFAULT	DbgInd = 0;	/* debugging indent level */
static	DEFAULT	DbgLvl = 0;	/* debugging message level */
#define DBGSBF	4000		/* huge buffer to avoid overruns */
GLOBAL	char DbgSBf[DBGSBF+1];	/* sprintf() buffer for debugging messages */
#if USE_PROTOTYPES
static VVOID DbgDMs(		/* debugging, display message */
	int DbgFLv,		/*   function display level */
	char *DbgFNm,		/*   function name */
	char *DbgWht,		/*   "called", "returning", or blank */
	char *DbgMsg);		/*   message to display */
static VVOID DbgDsA(void);	/* display address variables */
static VVOID DbgEBf(void);	/* display edit buffer details */
static VVOID DbgHlp(void);	/* display help message */
static VVOID DbgLSt(void);	/* display loop stack details */
static VVOID DbgMSt(void);	/* display macro stack details */
static VVOID DbgPrP(		/* display a pointer */
	char *s,
	voidptr ptr);
static VVOID DbgQRg(void);	/* display Q-register's */
static VVOID DbgQSt(void);	/* display Q-register stack */
static VVOID DbgSLv(void);	/* set debugging message level */
#endif
/*****************************************************************************
	DbgDsA()
	This function displays details about address variables for 1^P.
*****************************************************************************/
static VVOID DbgPrP (s, ptr)	/* display a pointer */
char *s;
voidptr ptr;
{
	printf ("%s %lu", s, Zcp2ul(ptr));
}
static VVOID DbgDsA()
{
	int	length;
	DbgPrP ("\r\nAraBeg =", AraBeg);
	DbgPrP ("\tEBfBeg =",   EBfBeg);
	DbgPrP ("\tGapBeg =",   GapBeg);
	DbgPrP ("\r\nAraEnd =", AraEnd);
	DbgPrP ("\tEBfEnd =",   EBfEnd);
	DbgPrP ("\tGapEnd =",   GapEnd);
	DbgPrP ("\r\nCBfBeg =", CBfBeg);
	DbgPrP ("\tCStBeg =",   CStBeg);
	DbgPrP ("\tFBfBeg =",   FBfBeg);
	DbgPrP ("\r\nCBfEnd =", CBfEnd);
	DbgPrP ("\tCStEnd =",   CStEnd);
	DbgPrP ("\tFBfEnd =",   FBfEnd);
	DbgPrP ("\r\nSBfBeg =", SBfBeg);
	DbgPrP ("\tEBPtr1 =",   EBPtr1);
	DbgPrP ("\tEBPtr2 =",   EBPtr2);
	DbgPrP ("\r\nSBfPtr =", SBfPtr);
	DbgPrP ("\tCBfPtr =",   CBfPtr);
	DbgPrP ("\r\nSBfEnd =", SBfEnd);
	DbgPrP ("\tSStPtr =",   SStPtr);
	printf ("\r\nsearch buffer = \"");
	if (SBfPtr != NULL) {
		length = (int) (SBfPtr-SBfBeg);
		if (length > 40) {
			length = 40;
		}
		printf ("%.*s", length, SBfBeg);
	} else {
		length = 0;
	}
	printf("\"\r\n");
}
/*****************************************************************************
	DbgEBf()
	This function displays details about the edit buffer for 2^P.
*****************************************************************************/
static VVOID DbgEBf()
{
	charptr	DbgPtr;
	DbgPrP ("\r\nEBfBeg =", EBfBeg);
	DbgPrP ("\tGapBeg =",   GapBeg);
	printf (" (%ld bytes)", (LONG) (GapBeg - EBfBeg));
	DbgPrP ("\r\nGapBeg =", GapBeg);
	DbgPrP ("\tGapEnd =",   GapEnd);
	printf (" (%ld bytes)", (LONG) (GapEnd - GapBeg));
	DbgPrP ("\r\nGapEnd =", GapEnd);
	DbgPrP ("\tEBfEnd =",   EBfEnd);
	printf (" (%ld bytes)", (LONG) (EBfEnd - GapEnd));
	DbgPrP ("\r\nEBfEnd =", EBfEnd);
	DbgPrP ("\tIBfEnd =",   IBfEnd);
	printf (" (%ld bytes)", (LONG) (IBfEnd - EBfEnd));
	DbgPrP ("\r\nEBfBeg =", EBfBeg);
	DbgPrP ("\tIBfEnd =",   IBfEnd);
	printf (" (%ld bytes)", (LONG) (IBfEnd - EBfBeg));
	for (DbgPtr = EBfBeg; DbgPtr < GapBeg; ++DbgPtr) {
		DbgPrP ("\r\nDbgEBf: first half, char at", DbgPtr);
		printf((isprint(*DbgPtr) ? " '%c'" : " <%d>"), *DbgPtr);
	}
	for (DbgPtr = (GapEnd+1); DbgPtr <= EBfEnd; ++DbgPtr) {
		DbgPrP ("\r\nDbgEBf: second half, char at", DbgPtr);
		printf((isprint(*DbgPtr) ? " '%c'" : " <%d>"), *DbgPtr);
	}
	puts ("\r");
}
/*****************************************************************************
	DbgHlp()
	This function displays help on ^P commands for ^P with no arguments.
*****************************************************************************/
static VVOID DbgHlp()
{
	puts("\r\n\tArguments to ^P have the following meanings:\r\n");
	puts("\t\t^P\tno arguments - display help about ^P\r");
	puts("\t\tm,0\tset debugging message level to m (0-5)\r");
	puts("\t\t1\tdisplay global address variables\r");
	puts("\t\t2\tdisplay details about the edit buffer\r");
	puts("\t\t3\tdisplay the loop stack\r");
	puts("\t\t4\tdisplay the macro stack\r");
	puts("\t\t5\tdisplay details about the q-registers\r");
	puts("\t\t6\tdisplay the q-register stack\r\r\r");
}
/*****************************************************************************
	DbgLSt ()
	This function displays details about the loop stack for 3^P.
*****************************************************************************/
static VVOID DbgLSt()
{
	int	i;
	printf ("\r\nLoop stack:  LStBot = %d, LstTop = %d\r\n\n", LStBot, LStTop);
	printf (" #     LIndex      LAddr\r\n");
	printf ("-- ---------- ----------\r\n");
	for (i = 0; i <= LStTop; ++i) {
		printf("%2d %10ld %10lu\r\n",
			i, LStack[i].LIndex, Zcp2ul(LStack[i].LAddr));
	}
}
/*****************************************************************************
	DbgDMs()
	This function displays a debugging message at the right indentation
level, in reverse video, if the debugging level is appropriate for the
message.  It also optionally calls the consistency_check() and the
check_code_checksums() routines while debugging.
	The debugging levels are:
	1	command functions
	2	functions called by command functions
	3	everything but stuff that's too noisy
	4	everything
*****************************************************************************/
static VVOID DbgDMs(DbgFLv, DbgFNm, DbgWht, DbgMsg)
int DbgFLv;				/* function display level */
char *DbgFNm;				/* function name */
char *DbgWht;				/* "called", "returning", or blank */
char *DbgMsg;				/* message to display */
{
	if (DbgFLv <= DbgLvl) {			/* is message at right level */
		printf ("%*s", DbgInd, "");	/* indent message */
		ZScrOp(SCR_RON);		/* turn on reverse video */
		printf ("%s: %s %s\r\n",
			DbgFNm,
			DbgWht,
			(DbgMsg == NULL) ? "" : DbgMsg);
		ZScrOp(SCR_ROF);		/* turn off reverse video */
	}
	if (DbgMsg == DbgSBf) {			/* clear sprintf() buffer */
		DbgSBf[0] = '\0';
	}
	if (DbgSBf[DBGSBF] != '\0') {		/* check sprintf() overrun */
		puts("DbgSBf[] overrun");
		exit(EXIT_FAILURE);
	}
/*
 * If you enable consistency checking and code checksumming here, pointers
 * and code will be checked on every function entry and exit in addition to
 * being checked after every successful command execution in ExeCSt().
 *
 * Enabling code checksumming here r-e-a-l-l-y slows things down.
 */
#if CONSISTENCY_CHECKING
	check_consistency ();
#endif
#if 0 && CHECKSUM_CODE				/* MS-DOS debugging code */
	check_code_checksums ();
#endif
}
/*****************************************************************************
	DbgFEn()
	This function is called as the first thing upon entry to a function.
*****************************************************************************/
VVOID DbgFEn(DbgFLv, DbgFNm, DbgMsg)	/* debugging, function entry */
int DbgFLv;				/*   function display level */
char *DbgFNm;				/*   function name */
char *DbgMsg;				/*   a function entry message */
{
	DbgInd += 2;
	DbgDMs(DbgFLv, DbgFNm, "called", DbgMsg);
}
/*****************************************************************************
	DbgFEx()
	This function is called as the last thing before a function returns.
*****************************************************************************/
VVOID DbgFEx(DbgFLv, DbgFNm, DbgMsg)
int DbgFLv;
char *DbgFNm;
char *DbgMsg;
{
	DbgDMs(DbgFLv, DbgFNm, "returning", DbgMsg);
	DbgInd -= 2;
}
/*****************************************************************************
	DbgFMs()
	This function is called in the middle of a routine if the routine
has something special to say.
*****************************************************************************/
VVOID DbgFMs(DbgFLv, DbgFNm, DbgMsg)	/* debugging, function message */
int DbgFLv;				/*   function display level */
char *DbgFNm;				/*   function name */
char *DbgMsg;				/*   a debugging message */
{
	DbgDMs(DbgFLv, DbgFNm, "", DbgMsg);
}
/*****************************************************************************
	DbgMSt()
	This function displays details about the macro stack for 4^P.
*****************************************************************************/
static VVOID DbgMSt()
{
	int	i;
	MSptr	MSp;
	printf("Macro Stack, MStTop = %d\r\n\n", MStTop);
	printf(" #   CStBeg   CBfPtr   CStEnd EStBot EStTop LStBot LStTop   QRgstr\r\n");
	printf("-- -------- -------- -------- ------ ------ ------ ------ --------\r\n");
	for (MSp = MStack, i = 0; i <= MStTop; ++i, ++MSp) {
		printf("%2d %8lu %8lu %8lu %6d %6d %6d %6d %8lu\r\n",
			i,
			Zcp2ul(MSp->CStBeg),
			Zcp2ul(MSp->CBfPtr),
			Zcp2ul(MSp->CStEnd),
			MSp->EStBot,
			MSp->EStTop,
			MSp->LStBot,
			MSp->LStTop,
			Zcp2ul(MSp->QRgstr));
	}
}
/*****************************************************************************
	DbgQRg()
	This function displays details about global Q-registers for 5^P.
*****************************************************************************/
static VVOID DbgQRg()
{
	int	i;
	int	length;
	charptr	cp;
	QRptr	QRp;
	printf ("Non-empty Q-registers:\r\n\n");
	printf (" #  Number   Start  End_P1 Text (possibly truncated)\r\n");
	printf ("-- ------- ------- ------- --------------------------------------------------\r\n");
	for (i = 0, QRp = QRgstr; i < 72; ++i, ++QRp) {
		if (QRp->Number != 0 || QRp->Start != NULL) {
			printf ("%c%c %7ld %7lu %7lu \"",
				(i < 36) ? ' ' : '.',
				i + ((i <= 9) ? '0' : 'A' - 10),
				QRp->Number,
				Zcp2ul(QRp->Start),
				Zcp2ul(QRp->End_P1));
			if (QRp->Start != NULL) {
				length = (int) (QRp->End_P1 - QRp->Start);
				if (length > 50) {
					length = 50;
				}
				cp = QRp->Start;
				while (length-- > 0) {
					if (*cp < ' ') {
						ZDspCh ('^');
						ZDspCh (*cp + '@');
						--length;
					} else {
						ZDspCh (*cp);
					}
					++cp;
				}
			}
			printf ("\"\r\n");
		}
	}
	printf ("\r\n");
}
/*****************************************************************************
	DbgQSt()
	This function displays details about the Q-register stack for 6^P.
*****************************************************************************/
static VVOID DbgQSt()
{
	int	i;
	int	length;
	charptr	cp;
	QRptr	QRp;
	printf("Q-register Stack, QStTop = %d\r\n\n", QStTop);
	printf ("#  Number   Start  End_P1 Text (possibly truncated)\r\n");
	printf ("-- ------- ------- ------- --------------------------------------------------\r\n");
	for (QRp = QStack, i = 0; i <= QStTop; ++i, ++QRp) {
		printf ("%2d %7ld %7lu %7lu \"",
			i,
			QRp->Number,
			Zcp2ul(QRp->Start),
			Zcp2ul(QRp->End_P1));
		if (QRp->Start != NULL) {
			length = (int) (QRp->End_P1 - QRp->Start);
			if (length > 50) {
				length = 50;
			}
			cp = QRp->Start;
			while (length-- > 0) {
				if (*cp < ' ') {
					ZDspCh ('^');
					ZDspCh (*cp + '@');
				} else {
					ZDspCh (*cp);
				}
				++cp;
			}
		}
		printf ("\"\r\n");
	}
	printf ("\r\n");
}
/*****************************************************************************
	DbgSLv()
	This function sets DbgLvl,  which controls how much debugging
information is displayed as TECO executes.  If DbgLvl is 0,  no debugging
messages are displayed.  If (DbgLvl>=1),  then messages are displayed by
all "Exexxx" functions.  This means that a message is displayed for each
entry and exit of execution of a single command.  If (DbgLvl>=1) then in
addition to the messages displayed for entry and exit of Exexxx functions,
a message is displayed for the "next level down" functions.  This scheme
is continued for DbgLvl >= 3, DbgLvl >= 4, etc.
	In most cases,  setting DbgLvl to 1 and then executing a TECO
command string will reveal an error.  For more detail, DbgLvl can be set
to successively higher values.  Using DbgLvl,  you can trade-off the amount
of time you wait for the messages to go by on the screen with the amount
of detail that's needed.
	By using this system,  debug messages that are placed into new code
in order to debug it can be left in the code so they're useful later.
*****************************************************************************/
static VVOID DbgSLv()
{
	DbgLvl = (DEFAULT)MArgmt;
}
/*****************************************************************************
	DbgDsp()
	This function provides control of the imbedded debugging system in
TECO-C.  An unused command character (currently control-P), when executed by
the user, causes this function to be executed.
	Arguments to ^P have the following meanings:
		no argument - display help about ^P
		m,0	set debugging message level to m
		1	display global address variables
		2	display details about the edit buffer
		3	display the loop stack
		4	display the macro stack
		5	display details about the q-registers
*****************************************************************************/
DEFAULT DbgDsp()
{
	if (EStTop == EStBot) {			/* if no numeric argument */
		ZScrOp(SCR_RON);		/* turn on reverse video */
		DbgHlp();			/* display debugging help */
		ZScrOp(SCR_ROF);		/* turn off reverse video */
		return SUCCESS;
	}
	if (GetNmA() == FAILURE) {		/* get numeric argument */
		return FAILURE;
	}
	if (CmdMod & MARGIS) {			/* if it's m,n^P */
		DbgSLv();			/* set DbgLvl */
		return SUCCESS;
	}
	ZScrOp(SCR_RON);		/* turn on reverse video */
	switch ((int)NArgmt) {
	    case 1: DbgDsA(); break;	/* display address variable details */
	    case 2: DbgEBf(); break;	/* display edit buffer details */
	    case 3: DbgLSt(); break;	/* display loop stack */
	    case 4: DbgMSt(); break;	/* display macro stack */
	    case 5: DbgQRg(); break;	/* display q-registers */
	    case 6: DbgQSt(); break;	/* display q-register stack */
	    default:
		printf("bad argument to ^P command\r\n");
		ZScrOp(SCR_ROF);	/* turn off reverse video */
		return FAILURE;
	}
	ZScrOp(SCR_ROF);		/* turn off reverse video */
	return SUCCESS;
}
#endif	/* #if DEBUGGING */
/*****************************************************************************
	The following code was needed while debugging TECO-C on the IBM PC
with it's brain-damaged memory addressing scheme.  This code checks the
"consistency" of various pointers, making sure some don't change throughout
the execution of TECO-C and making sure others are in the right order.
	After init_consistency_check() is called in Init(), the
check_consistency() code is called in ExeCSt() after the successful
completion of each command.  For more frequent checking, it can also
be called in the DbgDMs() routine above but then the "DEBUGGING" macro
in ZPORT.H has to be set to "TRUE" as well.
	If the "CONSISTENCY_CHECKING" macro in ZPORT.H is set to "FALSE",
 then none of this code is compiled.
*****************************************************************************/
#if CONSISTENCY_CHECKING
#if USE_PROTOTYPES
static int errprt(char *str, voidptr p1, voidptr p2);
#endif
static	charptr	ss_CBfBeg;
static	charptr	ss_CBfEnd;
static	charptr	ss_FBfBeg;
static	charptr	ss_FBfEnd;
static	charptr	ss_SBfBeg;
static	charptr	ss_SBfEnd;
VVOID init_consistency_check()
{
	ss_CBfBeg = CBfBeg;
	ss_CBfEnd = CBfEnd;
	ss_FBfBeg = FBfBeg;
	ss_FBfEnd = FBfEnd;
	ss_SBfBeg = SBfBeg;
	ss_SBfEnd = SBfEnd;
}
static int errprt(str, p1, p2)
char *str;
voidptr p1;
voidptr p2;
{
	printf ("%s (%lu,%lu)\r\n", str, Zcp2ul(p1), Zcp2ul(p2));
	return EXIT_FAILURE;
}
#if defined(__TURBOC__) && (__TURBOC__ >= 0x0295)
#include <alloc.h>		/* prototype for heapcheck() */
#endif
VVOID check_consistency()
{
    int ex = EXIT_SUCCESS;
/*
 * is the heap corrupted?
 */
#if defined(__TURBOC__) && (__TURBOC__ >= 0x0295)
	if (heapcheck () < 0) {
		puts ("check_consistency: heapcheck failed");
		ex = EXIT_FAILURE;
	}
#endif
/*
 * the following point to the start and end of various buffers which are
 * initialized at program startup and should not have changed.
 */
    if (ss_CBfBeg != CBfBeg) ex=errprt("ss_CBfBeg != CBfBeg",ss_CBfBeg,CBfBeg);
    if (ss_CBfEnd != CBfEnd) ex=errprt("ss_CBfEnd != CBfEnd",ss_CBfEnd,CBfEnd);
    if (ss_FBfBeg != FBfBeg) ex=errprt("ss_FBfBeg != FBfBeg",ss_FBfBeg,FBfBeg);
    if (ss_FBfEnd != FBfEnd) ex=errprt("ss_FBfEnd != FBfEnd",ss_FBfEnd,FBfEnd);
    if (ss_SBfBeg != SBfBeg) ex=errprt("ss_SBfBeg != SBfBeg",ss_SBfBeg,SBfBeg);
    if (ss_SBfEnd != SBfEnd) ex=errprt("ss_SBfEnd != SBfEnd",ss_SBfEnd,SBfEnd);
/*
 * make sure the pointers into the above buffers are between the start
 * and end of the buffers
 *
 * Note: while doing an EI or M command, CBfPtr can point outside of
 * [CBfBeg..CBfEnd] but it is always within [CStBeg..CStEnd].
 */
    if (CBfPtr < CStBeg) ex=errprt("CBfPtr < CStBeg",CBfPtr,CStBeg);
    if (CBfPtr > CStEnd) ex=errprt("CBfPtr > CStEnd",CBfPtr,CStEnd);
    if (FBfPtr < FBfBeg) ex=errprt("FBfPtr < FBfBeg",FBfPtr,FBfBeg);
    if (FBfPtr > FBfEnd) ex=errprt("FBfPtr > FBfEnd",FBfPtr,FBfEnd);
    if (SBfPtr < SBfBeg) ex=errprt("SBfPtr < SBfBeg",SBfPtr,SBfBeg);
    if (SBfPtr > SBfEnd) ex=errprt("SBfPtr > SBfEnd",SBfPtr,SBfEnd);
/*
 * The order of pointers into the editing buffer should be:
 *
 *   EBfBeg <= GapBeg <= GapEnd <= EBfEnd <= IBfEnd
 */
    if (EBfBeg > GapBeg) ex=errprt("EBfBeg > GapBeg",EBfBeg,GapBeg);
    if (GapBeg > GapEnd) ex=errprt("GapBeg > GapEnd",GapBeg,GapEnd);
    if (GapEnd > EBfEnd) ex=errprt("GapEnd > EBfEnd",GapEnd,EBfEnd);
    if (EBfEnd > IBfEnd) ex=errprt("EBfEnd > IBfEnd",EBfEnd,IBfEnd);
    if (ex == EXIT_FAILURE) {
	exit(EXIT_FAILURE);
    }
}
#endif	/* #if CONSISTENCY_CHECKING */
