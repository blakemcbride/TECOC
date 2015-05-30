/*****************************************************************************
	DScren.h
	Terminal types (0:W command,  CrType variable).  The strange
numbering is what is used in TECO-11.
*****************************************************************************/
#define	UNTERM	-1	/* unknown */
#define VT52	0	/* VT52 */
#define VT61	1	/* VT61 */
#define VT100V	2	/* VT100 in VT52 mode */
#define VT100	4	/* VT100 in ANSI mode */
#define VT05	6	/* VT05 */
#define VT102	8	/* VT102 */
#define VK100	10	/* VK100 */
#define VT200	11	/* VT200 in VT200 mode */
#define VT200A	12	/* VT200 in ANSI (VT100) mode */
#define VT200V	13	/* VT200 in VT52 mode */
#define IBMPC	14	/* IBM PC console */
/*****************************************************************************
	Screen function codes.  These are used in calls to ZScrOp.
*****************************************************************************/
#define SCR_CUP 1	/* move cursor up 1 row */
#define SCR_EEL 2	/* erase to end of line */
#define SCR_RON 3	/* reverse-video on */
#define SCR_ROF 4	/* reverse-video off */
