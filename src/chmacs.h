/*****************************************************************************
	Chmacs.h
	These are tested relational expressions. The ones defined for some
C's, like isalnum, tolower, etc do not always work, because they rely on bit
patterns within ASCII.  These work.
	Note: if this file is included,  the tecoc.h file must precede it.
*****************************************************************************/
extern unsigned char ChrMsk[];
#define Is_Upper(ch) (ChrMsk[(ch)] & CM_UPPER)
#define Is_Lower(ch) (ChrMsk[(ch)] & CM_LOWER)
#define Is_Digit(ch) (ChrMsk[(ch)] & CM_DIGIT)
#define Is_Alpha(ch) (ChrMsk[(ch)] & (CM_UPPER | CM_LOWER))
#define Is_Alnum(ch) (ChrMsk[(ch)] & (CM_UPPER | CM_LOWER | CM_DIGIT))
#define To_Lower(ch) (Is_Upper(ch) ? (ch)+' ' : (ch) )
#define To_Upper(ch) (Is_Lower(ch) ? (ch)-' ' : (ch) )
/*****************************************************************************
	these are TECO-specific. The above are not.
*****************************************************************************/
/* is line terminator */
#define IsEOL(ch) (ChrMsk[(ch)] & CM_LINE_TERM)
/* is symbol constituent */
#define Is_SyCon(ch) (Is_Alnum(ch) || ch == '.' || ch == '_' || ch == '$')
