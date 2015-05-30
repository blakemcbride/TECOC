/*****************************************************************************

	FindQR()

	This function "finds" a q-register.  CBfPtr points to the first
character of the q-register name.  If it is a period (q-register is local to
this macro level),  then CBfPtr is incremented to point to the next
character and,  if this is the first use of a local q-register in this macro
level,  then memory for a local q-register table is allocated.

	This function sets up the QR global variable.

*****************************************************************************/

#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
#include "chmacs.h"		/* define character processing macros */

DEFAULT FindQR()		/* find q-register index */
{
    WORD	i;
    BOOLEAN	LocalQ;
    WORD	QIndex;
    unsigned char QName;

/*
 * If it's a local q-register name (starts with a dot), remember and move on.
 */
    DBGFEN(3,"FindQR",NULL);
    LocalQ = (*CBfPtr == '.');			/* if local q-register */
    if (LocalQ) {				/* if local q-register name */
        if (IncCBP() == FAILURE) {		/* move to next character */
	    DBGFEX(3,DbgFNm,"FAILURE, IncCBP() failed");
	    return FAILURE;
	}
    }

/*
 * Get the index into the QRgstr array into i.
 */
    QName = *CBfPtr;
    if (Is_Upper(QName)) {
        i = 'A' - 10;				/* QRgStr[10..35] */
    } else if (Is_Lower(QName)) {
	i = 'a' - 10;				/* QRgstr[10..35] */
    } else if (Is_Digit(QName)) {
	i = '0';				/* QRgstr[0..9] */
    } else {
        ErrChr(ERR_IQN, QName);
	DBGFEX(3,DbgFNm,"FAILURE, illegal Q-register name");
	return FAILURE;
    }

/*
 * Get a pointer to the structure into QR.
 */
    QIndex = QName - i;				/* index into QRgstr[] */
    if (LocalQ) {				/* if local q-register */
        if (MStTop < 0) {			/* if not in a macro */
	    QR = &QRgstr[QIndex+36];		/* use QRgstr[36..71] */
	} else {				/* else (we're in a macro) */
	    WORD TMIndx;
	    QRptr QRp;
	    for (TMIndx=MStTop; TMIndx>0; TMIndx--) {
		if (MStack[TMIndx].QRgstr != NULL) {
		    QRp = MStack[TMIndx].QRgstr;
		    break;
		}
	    }
	    QR = (TMIndx != 0) ?		/* if we found one */
		    &QRp[QIndex]:		/* use the one we found */ 
		    (QRptr) &QRgstr[QIndex+36];	/* else use main level ones */
	}
    } else {
        QR = &QRgstr[QIndex];
    }

#if DEBUGGING
    sprintf(DbgSBf,"SUCCESS, %sQIndex = %d (QName = '%c')",
	    (LocalQ) ? "local " : "", QIndex, QName);
    DBGFEX(3,DbgFNm,DbgSBf);
#endif
    return SUCCESS;
}
