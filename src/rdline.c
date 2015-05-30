/*****************************************************************************
	RdLine()
	This function reads a line from the current input file into the
edit buffer.  The input is placed into the edit buffer by appending it
to the edit buffer and adjusting the EBfEnd pointer.  IBfEnd points to the
last byte of allocated, unused memory at the end of the edit buffer.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
#include "dchars.h"		/* define identifiers for characters */
/*
 * The input buffer needs to be expanded. If there's room in the edit buffer
 * gap,  then we can shuffle memory to steal some room from the gap.  If
 * there's still not enough room in the input buffer we re-allocate the
 * entire edit/input buffer and then adjust the pointers to reflect the
 * reallcatin.  If the reallocation fails,  set EBfFul to TRUE.
 */
#if USE_PROTOTYPES
static void expand_ibf(BOOLEAN *EBfFul)
#else
static void expand_ibf(EBfFul)
BOOLEAN *EBfFul;			/* indicates edit buffer full */
#endif
{
    SIZE_T	TmpSiz;
    SIZE_T	NewSiz;
    charptr	NewBeg;
    DBGFEN(3,"expabd_ibf",NULL);
/*
 * If the size of the edit buffer gap exceeds GAPMIN, make it GAPMIN and
 * add the leftover space to the input buffer.
 *
 * from:  EbfBeg...GapBeg<GAPMIN><slack>GapEnd...EBfEnd...IBfEnd
 * to:    EbfBeg...GapBeg<GAPMIN>GapEnd...EBfEnd...<slack>IBfEnd
 */
    TmpSiz = GapEnd-GapBeg+1;
    if (TmpSiz > GAPMIN) {
	TmpSiz -= GAPMIN;
	MEMMOVE(GapEnd+1-TmpSiz, GapEnd+1, (SIZE_T)(EBfEnd-GapEnd));
	GapEnd -= TmpSiz;
	EBfEnd -= TmpSiz;
    }
/*
 * If there's still not enough room in the input buffer, expand the input
 * buffer by reallocating everything.  Since reallocating is expensive,
 * take this opportunity to expand it by IBFEXP bytes more than what we
 * need.
 *
 * from:  EbfBeg...GapBeg...GapEnd...EBfEnd<TmpSiz>IBfEnd
 * to:    EbfBeg...GapBeg...GapEnd...EBfEnd<IBFMIN+IBFEXP>IBfEnd
 */
    TmpSiz = IBfEnd-EBfEnd;
    if (TmpSiz < IBFMIN) {
        NewSiz = (IBfEnd-EBfBeg+1) + (IBFMIN-TmpSiz) + IBFEXP;
#if DEBUGGING
	sprintf(DbgSBf,"ZRaloc-ing EBf, NewSiz = %ld", NewSiz);
	DbgFMs(3,DbgFNm,DbgSBf);
#endif
	NewBeg = (charptr)ZRaloc(EBfBeg, NewSiz);
	if (NewBeg == NULL) {
	    *EBfFul = TRUE;		/* we're full: stop reading lines */
	} else {
	    if (NewBeg != EBfBeg) {	/* if ZRaloc moved us, adjust ptrs */
		GapBeg = NewBeg + (GapBeg - EBfBeg);
		GapEnd = NewBeg + (GapEnd - EBfBeg);
		EBfEnd = NewBeg + (EBfEnd - EBfBeg);
		EBfBeg = NewBeg;
	    }
	    IBfEnd = (NewBeg + NewSiz) - 1;
	}
    }
    DBGFEX(3,DbgFNm,NULL);
}
DEFAULT RdLine(EBfFul)		/* read a line */
BOOLEAN *EBfFul;
{
    DEFAULT length;
    DBGFEN(3,"RdLine",NULL);
    FFPage = 0;				/* clear the form-feed flag */
    if (!IsOpnI[CurInp]) {		/* if current input stream not open */
	ErrMsg(ERR_NFI);		/* NFI = "no file for input" */
	DBGFEX(3,DbgFNm,"FAILURE, no file for input");
	return FAILURE;
    }
    if (IsEofI[CurInp]) {		/* if already at end-of-file */
	DBGFEX(3,DbgFNm,"SUCCESS, already at eof");
	return SUCCESS;
    }
/*
 * Before trying to read the next line from the input file,  we have to be
 * sure there's enough room for the record.  IBFMIN is the minimum amount of
 * room to provide when reading a record. If there's not IBFMIN bytes
 * available,  we have to get more room.
 */
    if ((IBfEnd-EBfEnd) < IBFMIN) {	/* if not enough room */
        expand_ibf(EBfFul);
    }
/*
 * Read the line from the input file.
 */
    if (*EBfFul == FALSE) {
	if (ZRdLin(EBfEnd+1, IBfEnd-EBfEnd, CurInp, &length) == FAILURE) {
	    ErrMsg(ERR_URL);		/* unable to read line */
	    DBGFEX(3,DbgFNm,"FAILURE, ZRdLin() failed");
	    return FAILURE;
	}
	if (!IsEofI[CurInp]) {		/* if not end-of-file */
	    EBfEnd += length;
	}
    }
    DBGFEX(3,DbgFNm,"SUCCESS, final");
    return SUCCESS;
}
