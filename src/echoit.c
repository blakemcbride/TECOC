/*****************************************************************************
	EchoIt()
	This function displays a character in it's printable form, either
as CH, ^CH, or [CH].
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global identifiers */
#include "dchars.h"		/* define identifiers for characters */
VVOID EchoIt(Charac)
unsigned char Charac;
{
	if (Charac > USCHAR && Charac < DELETE) {	/* displayable? */
		ZDspCh(Charac);
	} else if (Charac & '\200') {			/* eighth bit set? */
		if (EtFlag & ET_EIGHTBIT) {		/* term can display */
			ZDspCh(Charac);
		} else {
			ZDspCh('[');			/* display as [ch] */
			MakDBf((LONG)Charac,16);
			*DBfPtr++ = ']';
			ZDspBf(DBfBeg, DBfPtr-DBfBeg);
		}
	} else {
		switch (Charac) {
		case BAKSPC:
		case TABCHR:
		case LINEFD:
		case CRETRN:
			ZDspCh(Charac);
			break;
		case ESCAPE:
			ZDspCh('$');
			break;
		case FORMFD:
			ZDspCh('\r');
			/* a form feed shows as a CR followed by the same
			   blank lines a vertical tab does */
			/* FALLTHROUGH */
		case VRTTAB:
			ZDspBf("\n\n\n\n\n", FFLINS);
			break;
		case DELETE:
			break;
		case CTRL_G:
			ZDspCh(CTRL_G);
			/* ring the bell AND show it as ^G */
			/* FALLTHROUGH */
		default:				/* display as ^ch */
			ZDspCh('^');
			ZDspCh(Charac | '\100');
		}
	}
}
