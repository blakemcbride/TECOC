/*****************************************************************************
	ExeCSt()
	This function executes a TECO command string.  The string is pointed
to by CBfPtr.  The last character in the string is pointed to by CStEnd.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT ExeCSt()		/* execute command string */
{
	DEFAULT status;
	static DEFAULT (*FArray[])(VVOID) = {
/*NUL*/ ExeNul,   /* ^A*/ ExeCtA,   /* ^B*/ ZExCtB,   /* ^C*/ ExeCtC,
/* ^D*/ ExeCtD,   /* ^E*/ ExeCtE,   /* ^F*/ ExeNYI,   /* ^G*/ ExeIll,
/* ^H*/ ZExCtH,   /* ^I*/ ExeCtI,   /* LF*/ ExeNul,   /* VT*/ ExeIll,
/* ^L*/ ExeCtL,   /* CR*/ ExeNul,   /* ^N*/ ExeCtN,   /* ^O*/ ExeCtO,
/* ^P*/ ExeCtP,   /* ^Q*/ ExeCtQ,   /* ^R*/ ExeCtR,   /* ^S*/ ExeCtS,
/* ^T*/ ExeCtT,   /* ^U*/ ExeCtU,   /* ^V*/ ExeCtV,   /* ^W*/ ExeCtW,
/* ^X*/ ExeCtX,   /* ^Y*/ ExeCtY,   /* ^Z*/ ExeCtZ,   /*ESC*/ ExeEsc,
/* ^\*/ ExeIll,   /* ^]*/ ExeIll,   /* ^^*/ ExeCCC,   /* ^_*/ ExeUsc,
/* SP*/ ExeNul,   /* ! */ ExeExc,   /* " */ ExeDQu,   /* # */ ExeOpr,
/* $ */ ExeEsc,   /* % */ ExePrc,   /* & */ ExeOpr,   /* ' */ ExeNul,
/* ( */ ExeOpr,   /* ) */ ExeRtP,   /* * */ ExeOpr,   /* + */ ExeOpr,
/* , */ ExeCom,   /* - */ ExeOpr,   /* . */ ExeDot,   /* / */ ExeOpr,
/* 0 */ ExeDgt,   /* 1 */ ExeDgt,   /* 2 */ ExeDgt,   /* 3 */ ExeDgt,
/* 4 */ ExeDgt,   /* 5 */ ExeDgt,   /* 6 */ ExeDgt,   /* 7 */ ExeDgt,
/* 8 */ ExeDgt,   /* 9 */ ExeDgt,   /* : */ ExeCln,   /* ; */ ExeSCl,
/* < */ ExeLst,   /* = */ ExeEqu,   /* > */ ExeGtr,   /* ? */ ExeQes,
/* @ */ ExeAtS,   /* A */ ExeA,     /* B */ ExeB,     /* C */ ExeC,
/* D */ ExeD,     /* E */ ExeE,     /* F */ ExeF,     /* G */ ExeG,
/* H */ ExeH,     /* I */ ExeI,     /* J */ ExeJ,     /* K */ ExeK,
/* L */ ExeL,     /* M */ ExeM,     /* N */ ExeN,     /* O */ ExeO,
/* P */ ExeP,     /* Q */ ExeQ,     /* R */ ExeR,     /* S */ ExeS,
/* T */ ExeT,     /* U */ ExeU,     /* V */ ExeV,     /* W */ ExeW,
/* X */ ExeX,     /* Y */ ExeY,     /* Z */ ExeZ,     /* [ */ ExeLBr,
/* \ */ ExeBSl,   /* ] */ ExeRBr,   /* ^ */ ExeCrt,   /* _ */ ExeUnd,
/* ` */ ExeIll,   /* a */ ExeA,     /* b */ ExeB,     /* c */ ExeC,
/* d */ ExeD,     /* e */ ExeE,     /* f */ ExeF,     /* g */ ExeG,
/* h */ ExeH,     /* i */ ExeI,     /* j */ ExeJ,     /* k */ ExeK,
/* l */ ExeL,     /* m */ ExeM,     /* n */ ExeN,     /* o */ ExeO,
/* p */ ExeP,     /* q */ ExeQ,     /* r */ ExeR,     /* s */ ExeS,
/* t */ ExeT,     /* u */ ExeU,     /* v */ ExeV,     /* w */ ExeW,
/* x */ ExeX,     /* y */ ExeY,     /* z */ ExeZ,     /* { */ ExeIll,
/* | */ ExeBar,   /* } */ ExeIll,   /* ~ */ ExeIll,   /*DEL*/ ExeIll,
/*129*/ ExeIll,   /*130*/ ExeIll,   /*131*/ ExeIll,   /*132*/ ExeIll,
/*133*/ ExeIll,   /*134*/ ExeIll,   /*135*/ ExeIll,   /*136*/ ExeIll,
/*137*/ ExeIll,   /*138*/ ExeIll,   /*139*/ ExeIll,   /*140*/ ExeIll,
/*141*/ ExeIll,   /*142*/ ExeIll,   /*143*/ ExeIll,   /*144*/ ExeIll,
/*145*/ ExeIll,   /*146*/ ExeIll,   /*147*/ ExeIll,   /*148*/ ExeIll,
/*149*/ ExeIll,   /*150*/ ExeIll,   /*151*/ ExeIll,   /*152*/ ExeIll,
/*153*/ ExeIll,   /*154*/ ExeIll,   /*155*/ ExeIll,   /*156*/ ExeIll,
/*157*/ ExeIll,   /*158*/ ExeIll,   /*159*/ ExeIll,   /*160*/ ExeIll,
/*161*/ ExeIll,   /*162*/ ExeIll,   /*163*/ ExeIll,   /*164*/ ExeIll,
/*165*/ ExeIll,   /*166*/ ExeIll,   /*167*/ ExeIll,   /*168*/ ExeIll,
/*169*/ ExeIll,   /*170*/ ExeIll,   /*171*/ ExeIll,   /*172*/ ExeIll,
/*173*/ ExeIll,   /*174*/ ExeIll,   /*175*/ ExeIll,   /*176*/ ExeIll,
/*177*/ ExeIll,   /*178*/ ExeIll,   /*179*/ ExeIll,   /*180*/ ExeIll,
/*181*/ ExeIll,   /*182*/ ExeIll,   /*183*/ ExeIll,   /*184*/ ExeIll,
/*185*/ ExeIll,   /*186*/ ExeIll,   /*187*/ ExeIll,   /*188*/ ExeIll,
/*189*/ ExeIll,   /*190*/ ExeIll,   /*191*/ ExeIll,   /*192*/ ExeIll,
/*193*/ ExeIll,   /*194*/ ExeIll,   /*195*/ ExeIll,   /*196*/ ExeIll,
/*197*/ ExeIll,   /*198*/ ExeIll,   /*199*/ ExeIll,   /*200*/ ExeIll,
/*201*/ ExeIll,   /*202*/ ExeIll,   /*203*/ ExeIll,   /*204*/ ExeIll,
/*205*/ ExeIll,   /*206*/ ExeIll,   /*207*/ ExeIll,   /*208*/ ExeIll,
/*209*/ ExeIll,   /*210*/ ExeIll,   /*211*/ ExeIll,   /*212*/ ExeIll,
/*213*/ ExeIll,   /*214*/ ExeIll,   /*215*/ ExeIll,   /*216*/ ExeIll,
/*217*/ ExeIll,   /*218*/ ExeIll,   /*219*/ ExeIll,   /*220*/ ExeIll,
/*221*/ ExeIll,   /*222*/ ExeIll,   /*223*/ ExeIll,   /*224*/ ExeIll,
/*225*/ ExeIll,   /*226*/ ExeIll,   /*227*/ ExeIll,   /*228*/ ExeIll,
/*229*/ ExeIll,   /*230*/ ExeIll,   /*231*/ ExeIll,   /*232*/ ExeIll,
/*233*/ ExeIll,   /*234*/ ExeIll,   /*235*/ ExeIll,   /*236*/ ExeIll,
/*237*/ ExeIll,   /*238*/ ExeIll,   /*239*/ ExeIll,   /*240*/ ExeIll,
/*241*/ ExeIll,   /*242*/ ExeIll,   /*243*/ ExeIll,   /*244*/ ExeIll,
/*245*/ ExeIll,   /*246*/ ExeIll,   /*247*/ ExeIll,   /*248*/ ExeIll,
/*249*/ ExeIll,   /*250*/ ExeIll,   /*251*/ ExeIll,   /*252*/ ExeIll,
/*253*/ ExeIll,   /*254*/ ExeIll,   /*255*/ ExeIll,   /*256*/ ExeIll
	};
	DBGFEN(4,"ExeCSt",NULL);
	while (CBfPtr <= CStEnd) {		/* while more commands */
		if (TraceM) {			/* if trace mode is on */
			EchoIt(*CBfPtr);	/* display the command */
		}
		status = (*FArray[*CBfPtr])();	/* execute command */
#if CONSISTENCY_CHECKING
		check_consistency();
#endif
#if CHECKSUM_CODE
		check_code_checksums ();
#endif
		ZCHKKB();			/* kludge for MS-DOS only */
		if (GotCtC) {			/* if got a control-C */
			GotCtC = FALSE;
			ErrMsg(ERR_XAB);	/* XAB = execution aborted */
			DBGFEX(4,DbgFNm,"SUCCESS");
			return SUCCESS;
		}
		if (status == FAILURE) {	/* did command fail? */
			if (EtFlag & ET_MUNG_MODE) {	/* if in mung mode */
				TAbort(EXIT_FAILURE);	/* ... terminate */
			} else {
				DBGFEX(4,DbgFNm,"FAILURE");
				return FAILURE;
			}
		}
		CBfPtr++;			/* move to next command */
	}
	if (LStTop != LStBot) {			/* if unclosed loop */
		ErrUTC();			/* unterminated command */
	}
	DBGFEX(4,DbgFNm,"SUCCESS");
	return SUCCESS;
}
