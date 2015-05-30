/*****************************************************************************
	ClenUp()
	This function "cleans up" in preparation for terminating TECO-C. All
open files are closed and memory is freed.
*****************************************************************************/
#include "zport.h"		/* define portability identifiers */
#include "tecoc.h"		/* define general identifiers */
#include "defext.h"		/* define external global variables */
VVOID ClenUp (VVOID)		/* cleanup for TECO-C abort */
{
	QRptr	QRp;
	int	i;
	ZIClos(PINPFL);		/* Close the primary and secondary... */
	ZIClos(SINPFL);		/* ...input streams,  if they're open */
	ZOClDe(POUTFL);		/* Close the primary and secondary... */
	ZOClDe(SOUTFL);		/* ...output streams,  if they're open */
/*
 * free dynamically allocated buffers
 */
	if (CBfBeg) ZFree((voidptr)CBfBeg);	/* the command buffer */
	if (DBfBeg) ZFree((voidptr)DBfBeg);	/* the digit buffer */
	if (EBfBeg) ZFree((voidptr)EBfBeg);	/* the edit buffer */
	if (FBfBeg) ZFree((voidptr)FBfBeg);	/* the filename buffer */
	if (SBfBeg) ZFree((voidptr)SBfBeg);	/* the search string buffer */
/*
 * free the global [0-35] and main-level local [36-71] Q-registers,
 * and then the Q-register stack.
 */
	for (QRp = QRgstr, i = 0; i < 72; ++i, ++QRp) {
		if (QRp->Start != NULL) {
			ZFree((voidptr)QRp->Start);
		}
	}
	for (QRp = QStack, i = 0; i < QRS_SIZE; ++i, ++QRp) {
		if (QRp->Start != NULL) {
			ZFree((voidptr)QRp->Start);
		}
	}
	ZClnUp();			/* do system-dependent cleanup */
#if DEBUGGING
{
/*
 * at this point, there shouldn't be any block malloc'ed by TECO-C.
 * except if we're here because we EX'ed out of a macro, and we can't
 * get to the ZBf buffer allocated in ExeEI,  so the ZBf buffer is
 * left hanging.
 */
}
#endif
}
