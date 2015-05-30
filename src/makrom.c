/*****************************************************************************
	MakRom()
	This function "makes room" in a q-register text area for new text.
If the text area was empty,  this function simply calls ZAlloc to allocate
new memory.  If the text area was not empty,  this function calls ZRaloc to
add more memory to the text area.  In either case, the Amount argument
specifies how much memory to add to the existing text area.
	On entry,  QR->Start points to the start of the text area (or NULL if
the text area was empty) and QR->End_P1 points to the end of the text area.
On return,  QR->Start points to the start of the text area and QR->End_P1
points to the BEGINNING of the newly allocated memory.  This is because the
caller knows where the end is and is expected to reset QR->End_P1 after it
is used as the destination of the new text.
	Note:  the Amount argument is assumed to be greater than 0.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT MakRom(Amount)		/* make room in q-register */
SIZE_T Amount;			/* how much additional room to make */
{
	ptrdiff_t	OldSiz;		/* old size of text area */
	charptr		NewBlk;		/* new text area */
#if DEBUGGING
	static char *DbgFNm = "MakRom";
	sprintf(DbgSBf,"Amount = %ld bytes", (LONG)Amount);
	DbgFEn(2,DbgFNm,DbgSBf);
#endif
	if (QR->Start == NULL) {		/* if it was empty */
		OldSiz = 0;
		NewBlk = (charptr)ZAlloc(Amount);
	} else {
		OldSiz = QR->End_P1 - QR->Start;
		NewBlk = (charptr)ZRaloc(QR->Start, OldSiz + Amount);
	}
	if (NewBlk == NULL) {			/* if ZAlloc/ZRaloc failed */
		ErrMsg(ERR_MEM);		/* ERR = memory overflow */
		DBGFEX(2,DbgFNm,"FAILURE");
		return FAILURE;
	}
	QR->Start  = NewBlk;
	QR->End_P1 = NewBlk + OldSiz;
	DBGFEX(2,DbgFNm,"SUCCESS");
	return SUCCESS;
}
