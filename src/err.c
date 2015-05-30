/*****************************************************************************
	ErrChr(), ErrMsg(), ErrStr(), ErrUTC(), ErrVrb()
	These functions display TECO's error messages.
	ErrChr	display a message that contains a special character
	ErrMsg	display a message given the message number
	ErrStr	display a message that contains a special string
	ErrUTC	display UTC or UTM based on macro stack depth
	ErrVrb	display the verbose form of an error message
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define error message codes */
#include "dchars.h"		/* define identifiers for characters */
#include <string.h>		/* prototype for strlen() */
#if USE_PROTOTYPES
static	VVOID	ErrDsp(WORD ErrNum, unsigned char *ELine);
static	VVOID	ToErr(unsigned char Charac, charptr *PtrPtr);
#endif
/****
   IMPORTANT: if you change this list,  you must also change DEFERR.H,
   TECOC.RNH and VRBMSG.H !!!
****/
static char *Errors[] = {
/*  0*/  "ARG   Improper arguments",
/*  1*/  "BNI   > not in iteration",
/*  2*/  "DTB   Delete too big",
/*  3*/  "FNF   File not found \"%s\"",
/*  4*/  "ICE   Illegal ^E command in search argument",
/*  5*/  "IEC   Illegal character \"%s\" after E",
/*  6*/  "IFC   Illegal character \"%s\" after F",
/*  7*/  "IIA   Illegal insert argument",
/*  8*/  "ILL   Illegal command \"%s\"",
/*  9*/  "ILN   Illegal number",
/* 10*/  "IPA   Negative or 0 argument to P",
/* 11*/  "IQC   Illegal \" character",
/* 12*/  "IQN   Illegal Q-register name \"%s\"",
/* 13*/  "IRA   Illegal radix argument to ^R",
/* 14*/  "ISA   Illegal search argument",
/* 15*/  "ISS   Illegal search string",
/* 16*/  "IUC   Illegal character \"%s\" following ^",
/* 17*/  "MAP   Missing '",
/* 18*/  "MEM   Memory overflow",
/* 19*/  "NAB   No argument before ^_ ",
/* 20*/  "NAC   No argument before ,",
/* 21*/  "NAE   No argument before =",
/* 22*/  "NAP   No argument before )",
/* 23*/  "NAQ   No argument before \"",
/* 24*/  "NAS   No argument before ;",
/* 25*/  "NAU   No argument before U",
/* 26*/  "NCA   Negative argument to ,",
/* 27*/  "NFI   No file for input",
/* 28*/  "NFO   No file for output",
/* 29*/  "NYA   Numeric argument with Y",
/* 30*/  "NYI   Not yet implemented",
/* 31*/  "OFO   Output file already open",
/* 32*/  "PDO   Push-down list overflow",
/* 33*/  "PES   Attempt to pop an empty stack",
/* 34*/  "POP   Attempt to move pointer off page with \"%s\"",
/* 35*/  "SNI   ; not in iteration",
/* 36*/  "SRH   Search failure \"%s\"",
/* 37*/  "STL   String too long",
/* 38*/  "TAG   Missing tag !%s!",
/* 39*/  "UTC   Unterminated command",
/* 40*/  "UTM   Unterminated macro",
/* 41*/  "XAB   Execution aborted",
/* 42*/  "YCA   Y command aborted",
/* 43*/  "IFE   ill-formed numeric expression",
/* 44*/  "SYS   %s",
/* 45*/  "UCD   unable to close and delete output file %s",
/* 46*/  "UCI   unable to close input file",
/* 47*/  "UCO   unable to close output file",
/* 48*/  "UFI   unable to open file %s for input",
/* 49*/  "UFO   unable to open file %s for output",
/* 50*/  "URC   unable to read character from terminal",
/* 51*/  "URE   unable to read TECO command file",
/* 52*/  "URL   unable to read line from input file",
/* 53*/  "UWL   unable to write line to output file"
};
/*****************************************************************************
	ErrUTC()
	Simply call ErrMsg with either ERR_UTC (unterminated command) or
ERR_UTM (unterminated macro) depending on whether we're in a macro or not.
*****************************************************************************/
VVOID ErrUTC()
{
	ErrMsg((MStTop < 0) ? ERR_UTC : ERR_UTM);
}
/*****************************************************************************
	ErrVrb()
	This function displays the verbose form of an error message.
The verbose form is a short paragraph explaining the error condition in
some detail.  This function is called from ErrDsp when the least significant
two bits of the EH flag are set to 3,  or from FrstCh when the user types
a / immediately after receiving a short error message.
	Verbose explanations of error messages are stored in the HELP system
so that they can be accessed using the HELP command.  Therefore this function
calls ZHelp to display the verbose messages.
*****************************************************************************/
VVOID ErrVrb(ErrNum)		/* display verbose error message */
WORD ErrNum;			/* error message number */
{
	DBGFEN(3,"ErrVrb",NULL);
	ZVrbos(ErrNum, Errors[ErrNum]);
	DBGFEX(3,DbgFNm,NULL);
}
/*****************************************************************************
	ErrDsp()
	This function displays a TECO error message on the terminal screen.
This function is passed the number of the error message and the text of the
error message.
	The EH flag controls how verbose the error message will be.  The two
least significant bits of the EH flag control how verbose the error message
is.  The third bit (mask is EH_COMMAND) causes the erroneous command string up
to and including the erroneous command to be displayed.
	The low two bits of EH have the following meanings:
	0	same as 2
	1	only the three letter code is output
	2	three letter code and one-line error message
	3	three letter code, one-lien message and paragraph
*****************************************************************************/
static VVOID ErrDsp(ErrNum, ELine)
WORD ErrNum;			/* the error number */
unsigned char *ELine;		/* error message text */
{
	WORD HlpLvl;
	ptrdiff_t lenn;
	HlpLvl = (EhFlag & 3) ? (EhFlag & 3) : 2;
	lenn = (HlpLvl < 2) ? 4 : strlen((char *)ELine);
	ZDspBf(ELine, lenn);
	if (HlpLvl == 3) {			/* if paragraph */
		ErrVrb(ErrNum);			/* display paragraph */
	}
	ZDspBf("\r\n", 2);
	if (EhFlag & EH_COMMAND) {	/* if error string display is on */
		TypESt();
	}
	LstErr = ErrNum;
}
/*****************************************************************************
	ToErr()
	Given a character and a buffer pointer,  this function puts the
displayable representation for the character into the buffer and updates
the buffer position.  For most characters,  the displayable representation
is the character itself.
*****************************************************************************/
static VVOID ToErr(Charac, PtrPtr)
unsigned char Charac;
charptr *PtrPtr;
{
	char SBuf[5];
	char *SPtr;
	size_t SLen;
	switch (Charac) {
	    case TABCHR:	SPtr = "<TAB>";	SLen = 5; break;
	    case LINEFD:	SPtr = "<LF>";	SLen = 4; break;
	    case VRTTAB:	SPtr = "<VT>";	SLen = 4; break;
	    case FORMFD:	SPtr = "<FF>";	SLen = 4; break;
	    case CRETRN:	SPtr = "<CR>";	SLen = 4; break;
	    case ESCAPE:	SPtr = "<ESC>";	SLen = 5; break;
	    case '\0':		SPtr = "<NUL>";	SLen = 5; break;
	    default:		if (Charac >= DELETE) {
					SPtr = SBuf;
					MakDBf((LONG)Charac, 16);
					SLen = (size_t) (DBfPtr - DBfBeg);
					SBuf[0] = '[';
					MEMMOVE(&SBuf[1], DBfBeg, SLen);
					SBuf[++SLen] = ']';
					++SLen;
				} else if (Charac < SPACE) {
					SPtr = SBuf;
					MEMMOVE(SBuf, "<^x>", 4);
					SBuf[2] = Charac + '@';
					SLen = 4;
				} else {
					SPtr = (char *)&Charac;
					SLen = 1;
				}
				break;
	}
	MEMMOVE(*PtrPtr, SPtr, SLen);
	*PtrPtr += SLen;
}
/*****************************************************************************
*****************************************************************************/
VVOID ErrChr(ErrNum, EChr)	/* display error message with character arg */
WORD ErrNum;			/* the error number */
unsigned char EChr;		/* character to imbed in message */
{
	char *TmpPtr;
	unsigned char ErrBuf[ERBFSIZ];
	charptr	EPtr = ErrBuf;
	*EPtr++ = '?';
	TmpPtr = Errors[ErrNum];
	while (*TmpPtr != '%') {	/* copy up to "%" */
		*EPtr++ = *TmpPtr++;
	}
	ToErr(EChr, &EPtr);		/* copy the %c character */
	TmpPtr++;			/* skip % */
	TmpPtr++;			/* skip c */
	while (*TmpPtr) {		/* copy rest of string */
		*EPtr++ = *TmpPtr++;
	}
	*EPtr = '\0';
	ErrDsp(ErrNum, ErrBuf);
}
/*****************************************************************************
*****************************************************************************/
VVOID ErrMsg(ErrNum)		/* display error message */
WORD ErrNum;			/* the error number */
{
	char *TmpPtr;
	unsigned char ErrBuf[ERBFSIZ];
	charptr EPtr = ErrBuf;
	*EPtr++ = '?';
	TmpPtr = Errors[ErrNum];
	while (*TmpPtr) {		/* copy rest of string */
		*EPtr++ = *TmpPtr++;
	}
	*EPtr = '\0';
	ErrDsp(ErrNum, ErrBuf);
}
/*****************************************************************************
*****************************************************************************/
VVOID ErrStr(ErrNum, EStr)	/* display error message with string arg */
WORD ErrNum;			/* the error number */
char *EStr;			/* string to imbed in message */
{
	char *TmpPtr;
	unsigned char ErrBuf[ERBFSIZ];
	charptr EPtr = ErrBuf;
	*EPtr++ = '?';
	TmpPtr = Errors[ErrNum];
	while (*TmpPtr != '%') {	/* copy up to "%" */
		*EPtr++ = *TmpPtr++;
	}
	while (*EStr) {			/* copy %s string */
		ToErr((unsigned char)*EStr++, &EPtr);
	}
	TmpPtr++;			/* skip % */
	TmpPtr++;			/* skip s */
	while (*TmpPtr) {		/* copy rest of string */
		*EPtr++ = *TmpPtr++;
	}
	*EPtr = '\0';
	ErrDsp(ErrNum, ErrBuf);
}
/*****************************************************************************
*****************************************************************************/
VVOID ErrPSt(ErrNum, EBeg, EEnd) /* display error message with string arg */
WORD ErrNum;			/* the error number */
charptr EBeg;			/* string to imbed in message */
charptr EEnd;			/* ptr to char after last char */
{
	char *TmpPtr;
	unsigned char ErrBuf[ERBFSIZ];
	charptr EPtr = ErrBuf;
	*EPtr++ = '?';
	TmpPtr = Errors[ErrNum];
	while (*TmpPtr != '%') {	/* copy up to "%" */
		*EPtr++ = *TmpPtr++;
	}
	while (EBeg < EEnd) {		/* copy %s string */
		ToErr(*EBeg++, &EPtr);
	}
	TmpPtr++;			/* skip % */
	TmpPtr++;			/* skip s */
	while (*TmpPtr) {		/* copy rest of string */
		*EPtr++ = *TmpPtr++;
	}
	*EPtr = '\0';
	ErrDsp(ErrNum, ErrBuf);
}
