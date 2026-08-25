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
/*
 * free the local Q-register tables belonging to any macros we are still
 * inside of.  ExeM allocates one per macro level and PopMac frees it on the
 * way out,  but exiting from inside a macro (EX, EG, ^C^C ...) never reaches
 * PopMac,  so they have to be released here.
 */
	for (i = 0; i <= MStTop; ++i) {
		if (MStack[i].QRgstr != NULL) {
			WORD j;
			QRp = MStack[i].QRgstr;
			for (j = 0; j < 36; ++j, ++QRp) {
				if (QRp->Start != NULL) {
					ZFree((voidptr)QRp->Start);
				}
			}
			ZFree((voidptr)MStack[i].QRgstr);
			MStack[i].QRgstr = NULL;
		}
	}
	ZClnUp();			/* do system-dependent cleanup */
#if DEBUGGING
{
/*
 * at this point, there shouldn't be any block malloc'ed by TECO-C, except
 * when we are here because we EX'ed (or EG'ed, or ^C^C'ed) out of a macro or
 * a command file.  Two blocks are unreachable from here in that case: the
 * copy of the macro text ExeM is executing from, and the ZBf buffer ExeEI
 * allocated.  Both are locals of frames that are still on the stack, so they
 * cost nothing -- the process is on its way out -- but they are not freed.
 */
}
#endif
}
