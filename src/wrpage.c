/*****************************************************************************
	WrPage()
	This function handles all TECOC's file output, except for the E%q
command.  The commands that can cause output are:
	P	write out edit buffer,  with a form feed depending on AddFF
	PW	write out edit buffer,  with a form feed
	m,nPW	write out characters between m and n,  without form feed
	HPW	write out edit buffer,  without a form feed
	This function is passed the index of the file's data block in array
OFiles,  the beginning and ending addresses in the edit buffer of the text
to be output,  and a form feed flag indicating whether a form feed is to be
appended to the output or not.
	The addresses of the text in the edit buffer might specify an area
that spans the edit buffer gap.  In order to avoid writing the edit buffer
gap to the file,  this function temporarily "shuffles" the second half of
the edit buffer,  appending it to the first half to produce a contiguous
buffer for output.  After the text is output,  the text is shuffled back.
	If a form feed is to be appended to the text,  then the character
following the last character of the text area is temporarily overwritten
with a form feed character.  After the text is output,  the character that
was overwritten is restored.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
#include "dchars.h"		/* define identifiers for characters */
#include "deferr.h"		/* define identifiers for error messages */
DEFAULT WrPage(OfIndx, OBfBeg, OBfEnd, AddFF)
DEFAULT	OfIndx;			/* index into OFiles array */
charptr OBfBeg;			/* address of output buffer beginning */
charptr OBfEnd;			/* address of output buffer end */
LONG AddFF;			/* add form feed: -1 is TRUE, 0 is FALSE */
{
	unsigned char	SavChr;	/* saved char overwritten with form feed */
	ptrdiff_t	Shuffl;	/* size of shuffled area */
	DEFAULT		status;
#if DEBUGGING
	static char *DbgFNm = "WrPage";
	sprintf(DbgSBf,"OBfBeg = %ld, OBfEnd = %ld, AddFF = %ld",
		Zcp2ul(OBfBeg), Zcp2ul(OBfEnd), AddFF);
	DbgFEn(2,DbgFNm,DbgSBf);
	sprintf(DbgSBf,"line 2: GapBeg = %ld, GapEnd = %ld",
		Zcp2ul(GapBeg), Zcp2ul(GapEnd));
	DbgFMs(2,DbgFNm,DbgSBf);
#endif
	if (!IsOpnO[OfIndx]) {			/* if no output file */
		ErrMsg(ERR_NFO);		/* no file for output */
		DBGFEX(2,DbgFNm,"FAILURE");
		return FAILURE;
	}
/*
 * If the text area spans the edit buffer gap,  then "shuffle",  which
 * means temporarily copying the second half of the edit buffer to the
 * first half.  Later,  the reverse opertion will be done.
 */
	if ((OBfBeg < GapBeg) && (OBfEnd > GapEnd)) {	/* shuffle? */
		Shuffl = OBfEnd - GapEnd;
		MEMMOVE(GapBeg, GapEnd+1, (SIZE_T)Shuffl);
		OBfEnd = (GapBeg + Shuffl) - 1;
	} else {
		Shuffl = 0;			/* 0 indicates no shuffle */
		if (OBfBeg == GapBeg) {		/* if there's no first half */
			OBfBeg = GapEnd + 1;
		}
		if (OBfEnd == GapEnd) {		/* if no second half */
			OBfEnd = GapBeg - 1;
		}
		if (OBfBeg > OBfEnd) {		/* if both halves are empty */
			OBfBeg = OBfEnd + 1;	/* make it a 0 length area */
		}
	}
/*
 * If we need to append a form feed,  then save the character being
 * overwritten and overwrite it with a form feed.
 */
	if (AddFF) {
		++OBfEnd;
		SavChr = *OBfEnd;
		*OBfEnd = FORMFD;
	}
/*
 * Write the buffer.
 */
	if (OBfBeg <= OBfEnd) {
		status = ZWrBfr(OfIndx, OBfBeg, OBfEnd);
	} else {
		status = SUCCESS;
	}
/*
 * If we overwrote with a form feed,  restore the overwritten character.
 */
	if (AddFF) {
		*OBfEnd = SavChr;
	}
/*
 * If we previously shuffled the edit buffer,  then shuffle it back.
 */
	if (Shuffl) {
		MEMMOVE(GapEnd+1, GapBeg, (SIZE_T)Shuffl);
	}
	DBGFEX(2,DbgFNm,NULL);
	return status;
}
