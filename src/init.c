/*****************************************************************************
	Init.c
	Initialization routines for Tecoc.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
#include <stdio.h>
#if USE_PROTOTYPES
static VVOID MemIni(void);
#endif
/*****************************************************************************
	MemIni()
	This function allocates memory for the edit buffer,  command buffer,
filename buffer and search string buffer.  System-independent global
variables are also initialized by this function.
*****************************************************************************/
static VVOID MemIni()
{
	WORD	i;
	QRptr	QRp;
	charptr TmpMem;
	DBGFEN(2,"MemIni",NULL);
/*
 * allocate a chunk of memory to start with which is free'd as soon as
 * MemIni exits.  subsequent small memory allocations might be taken from
 * this free'd chunk without fragmenting the end of the heap where the
 * edit buffer resides.
 */
	TmpMem = (charptr)ZAlloc((SIZE_T)(4*1024));
/*
 * allocate memory for the digit buffer.  It needs to be big enough to hold
 * the decimal (signed), octal or hexadecimal representation of the largest
 * number that can be stored in a LONG.  Here, it's assumed that octal takes
 * the most characters.  The calculation is as follows:
 *
 *	sizeof(LONG) * 8 to get bits per long
 *	+2 to round up so /3 doesn't truncate
 *	/3 since there is 3 bits per octal digit
 *	+2 in case ExeEqu() appends a <CR><LF> for display purposes
 */
	DBfBeg = DBfPtr = (charptr)ZAlloc((((sizeof(LONG)*8)+2)/3)+2);
	if (DBfBeg == NULL) {
		ErrMsg(ERR_MEM);		/* MEM = memory overflow */
#if DEBUGGING
		sprintf(DbgSBf,"DBfBeg = ZAlloc(15) failed, calling TAbort()");
		DbgFMs(2,DbgFNm,DbgSBf);
#endif
		TAbort(EXIT_FAILURE);
	}
/*
 * allocate memory for the filename buffer
 */
	FBfBeg = FBfPtr = (charptr)ZAlloc((SIZE_T)FILENAME_MAX);
	if (FBfBeg == NULL) {
		ErrMsg(ERR_MEM);		/* MEM = memory overflow */
#if DEBUGGING
		sprintf(DbgSBf,"FBfBeg = ZAlloc(%ld) failed, calling TAbort()",
			(LONG)FILENAME_MAX);
		DbgFMs(2,DbgFNm,DbgSBf);
#endif
		TAbort(EXIT_FAILURE);
	}
	FBfEnd = FBfBeg + FILENAME_MAX - 1;
/*
 * allocate memory for the command buffer
 */
	CBfBeg = (charptr)ZAlloc((SIZE_T)CBFINIT);
	if (CBfBeg == NULL) {
		ErrMsg(ERR_MEM);		/* MEM = memory overflow */
#if DEBUGGING
		sprintf(DbgSBf,"CbfBeg =ZAlloc(%ld) failed, calling TAbort()",
			(LONG)CBFINIT);
		DbgFMs(2,DbgFNm,DbgSBf);
#endif
		TAbort(EXIT_FAILURE);
	}
	CBfEnd = CBfBeg + CBFINIT - 1;
/*
 * allocate memory for the search string buffer
 */
	SBfBeg = SBfPtr = (charptr)ZAlloc((SIZE_T)SBFINIT);
	if (SBfBeg == NULL) {
		ErrMsg(ERR_MEM);		/* MEM = memory overflow */
#if DEBUGGING
		sprintf(DbgSBf,"SbfBeg = ZAlloc(%ld) failed, calling TAbort()",
			(LONG)SBFINIT);
		DbgFMs(2,DbgFNm,DbgSBf);
#endif
		TAbort(EXIT_FAILURE);
	}
	SBfEnd = SBfBeg + SBFINIT - 1;
/*
 * allocate memory for the edit buffer and input buffer
 */
	EBfBeg = GapBeg = (charptr)ZAlloc((SIZE_T)EBFINIT + (SIZE_T)IBFINIT);
	if (EBfBeg == NULL) {
		ErrMsg(ERR_MEM);		/* MEM = memory overflow */
#if DEBUGGING
		sprintf(DbgSBf,"EbfBeg = ZAlloc(%ld) failed, calling TAbort()",
			(LONG)(EBFINIT + IBFINIT));
		DbgFMs(2,DbgFNm,DbgSBf);
#endif
		TAbort(EXIT_FAILURE);
	}
	EBfEnd = GapEnd = EBfBeg + EBFINIT - 1;
	IBfEnd = EBfEnd + IBFINIT;
/*
 * clear the global [0-35] and main-level local [36-71] Q-registers
 */
	for (QRp = QRgstr, i = 0; i < 72; ++i, ++QRp) {
		QRp->Start = QRp->End_P1 = NULL;
		QRp->Number = 0;
	}
/*
 * Clear the Q-register stack
 */
	for (QRp = QStack, i = 0; i < QRS_SIZE; ++i, ++QRp) {
		QRp->Start = QRp->End_P1 = NULL;
		QRp->Number = 0;
	}
/*
 * Initialize the file-open and end-of-file indicators.
 */
	for (i = 0; i < NIFDBS; i++) {
		IsOpnI[i] = IsEofI[i] = FALSE;
	}
	for (i = 0; i < NOFDBS; i++) {
		IsOpnO[i] = FALSE;
	}
/*
 * Free our small allocation memory pool.
 */
	if (TmpMem) {
		ZFree (TmpMem);
	}
	DBGFEX(2,DbgFNm,NULL);
}
/*****************************************************************************
	Init()
	This function initializes TECO-C.
	If an error is detected in any of the functions called by this
function,  a message is displayed and the program terminates.
*****************************************************************************/
VVOID Init(argc, argv)		/* initialize TECO-C */
int argc;
char **argv;
{
	DBGFEN(2,"Init",NULL);
	ZTrmnl();		/* open terminal for input and output */
/*
 * "ET&128 -- abort-on-error bit.  Initially set,  when TECO starts up;  cleared
 * each time TECO issues its asterisk prompt."  ExeCSt honours the bit in both
 * of the places that matter -- a ^C and a failed command -- but nothing in the
 * tree ever turned it on,  so it could never fire:  "tecoc mung script.tec"
 * printed the error and then dropped into the interactive "*" prompt,  which
 * in a Makefile or a shell script is a hang rather than a failure,  and a ^C
 * during command-line processing did not terminate TECO either.  ZTrmnl has
 * just ASSIGNED EtFlag,  so the bit has to go on after it;  ReadCS clears it
 * where it writes the prompt.  lib/squ.tes:122 already tests the bit on entry
 * and then sets it,  so the shipped squeezer assumes a TECO where this is live.
 */
	EtFlag |= ET_MUNG_MODE;	/* abort on error until the first prompt */
	MemIni();		/* allocate memory and initialize variables */
#if CONSISTENCY_CHECKING
	init_consistency_check();
	check_consistency();
#endif
	ZPrsCL(argc, argv);	/* parse the command line */
	DBGFEX(2,DbgFNm,NULL);
}
