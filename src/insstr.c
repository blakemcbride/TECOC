/*****************************************************************************
	InsStr()
	This function is called to insert text into the edit buffer.  The
edit buffer is expanded to make room for the text if necessary.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
/*
 * The edit buffer gap needs to be expanded. If there's room in the input
 * buffer,  then we can shuffle memory to steal some room from the input
 * buffer. If there's not room to steal from the input buffer,  we have to
 * re-allocate the whole edit/input buffer and then adjust the pointers to
 * reflect the movement.
 */
#if USE_PROTOTYPES
static DEFAULT expand_gap(ptrdiff_t length)
#else
static DEFAULT expand_gap(length)
ptrdiff_t length;			/* amount of gap room needed */
#endif
{
    SIZE_T TmpSiz;
    SIZE_T NewSiz;
    charptr NewBeg;
    DBGFEN(3,"expand_gap",NULL);
/*
 * If the size of the input buffer exceeds IBFMIN, make it IBFMIN and
 * add the leftover space to the edit buffer gap.
 *
 * from:  EbfBeg...GapBeg...GapEnd...EBfEnd<TmpSiz><IBFMIN>IBfEnd
 * to:    EbfBeg...GapBeg...<TmpSiz>GapEnd...EBfEnd<IBFMIN>IBfEnd
 */
    TmpSiz = IBfEnd-EBfEnd;
    if (TmpSiz > IBFMIN) {
	TmpSiz -= IBFMIN;
	MEMMOVE(GapEnd+1+TmpSiz, GapEnd+1, (SIZE_T)(EBfEnd-GapEnd));
	GapEnd += TmpSiz;
	EBfEnd += TmpSiz;
    }
/*
 * If there's still not enough room in the edit buffer gap, expand the gap
 * by reallocating the edit buffer.  Since reallocating is expensive,  take
 * this opportunity to expand it by EBFEXP bytes more than what we need.
 *
 * from:  EbfBeg...GapBeg<GapSiz>GapEnd...EBfEnd...IBfEnd
 * to:    EbfBeg...GapBeg<length-GapSiz+EBFEXP>GapEnd...EBfEnd...IBfEnd
 */
    TmpSiz = GapEnd-GapBeg+1;
    if (TmpSiz < length) {			    /* str length > GapSiz */
	TmpSiz = (length - TmpSiz) + EBFEXP;	    /* amt to expand EBf */
	NewSiz = (IBfEnd-EBfBeg+1) + TmpSiz;	    /* new size of EBf */
	NewBeg = (charptr)ZRaloc(EBfBeg, NewSiz);   /* reallocate EBf */
	if (NewBeg == NULL) {			    /* if failure */
	    ErrMsg(ERR_MEM);			    /* memory overflow */
	    DBGFEX(3,DbgFNm,"FAILURE, couldn't ZRaloc more gap room");
	    return FAILURE;
	}
/*
 * Make sure all the pointers reference the new memory area.
 */
	if (NewBeg != EBfBeg) {
	    GapBeg = NewBeg + (GapBeg - EBfBeg);
	    GapEnd = NewBeg + (GapEnd - EBfBeg);
	    EBfEnd = NewBeg + (EBfEnd - EBfBeg);
	    IBfEnd = NewBeg + (IBfEnd - EBfBeg);
	    EBfBeg = NewBeg;
	}
/*
 * Now open up the gap by moving the second half of the edit buffer
 * down the amount we expanded the edit buffer (TmpSiz), then update the
 * pointers again.
 */
	MEMMOVE(GapEnd+1+TmpSiz, GapEnd+1, (SIZE_T)(EBfEnd - GapEnd));
	GapEnd += TmpSiz;
	EBfEnd += TmpSiz;
	IBfEnd += TmpSiz;
    }
    DBGFEX(3,DbgFNm,NULL);
    return SUCCESS;
}
DEFAULT InsStr(string, length)		/* insert string into edit buffer */
charptr string;
ptrdiff_t length;
{
#if DEBUGGING
	static char *DbgFNm = "InsStr";
	sprintf(DbgSBf,"length = %d", length);
	DbgFEn(3,DbgFNm,DbgSBf);
#endif
	RefLen = -length;		/* ^S = -length of last string */
/*
 * Before trying to insert the text,  we have to be sure there's enough room
 * in the edit buffer gap for the text.
 */
	if ((GapEnd-GapBeg+1) < length) {
	    if (expand_gap(length) == FAILURE) {
		DBGFEX(3,DbgFNm,"FAILURE, expand_gap() failed");
		return FAILURE;
	    }
	}
/*
 * Copy the new text into the edit buffer gap,  and adjust GapBeg to make the
 * text part of the edit buffer.
 */
	MEMMOVE(GapBeg, string, (SIZE_T)length);
	GapBeg += length;
	DBGFEX(3,DbgFNm,"SUCCESS");
	return SUCCESS;
}
