/*****************************************************************************
	ZPort.h

	This file contains portability identifiers.  It is the only file
that should need modification when porting the code.  It is assumed that the
compiler supports the #elif and "defined()" preprocessor directives,  but
it need not be fully ANSI C compatible in all ways.
*****************************************************************************/

/****************************************************************************
	These identifiers enhance code readability as well as providing the
capability to avoid machine-dependent data type problems.  The following
types work for the VAX/VMS and SAGE/CPM-68K compilers.  If a compiler does
not have 8-bit chars, 16 or 32-bit shorts and 32-bit longs,  then problems
will arise.
	There are many places in TECOC where one pointer is subtracted from
another.  If the result is stored in a variable or sent to a function,  I
have tried to define the type of the destination as ptrdiff_t.
	The mode control flags (like ES, ED) are 16-bit entities defined as
WORD.  The DoEvEs function will not function correctly if WORD does not
produce signed 16-bit variables.
	The MakDBf function will not work properly if LONG does not
produce 32-bit variables.
*****************************************************************************/

typedef	int		BOOLEAN;	/* holds FALSE or TRUE */
typedef int		DEFAULT;	/* signed, at least 16 bits */
typedef	short		WORD;		/* 16 bits signed */
typedef long		LONG;		/* 32 bits signed */
typedef unsigned long	ULONG;		/* 32 bits unsigned */

#define FOREVER		for(;;)		/* Infinite loop declaration */

/*
 * Define TRUE and FALSE if they haven't been defined already.
 */

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

/*****************************************************************************
	The following identifiers determine which compiler is being used.
Only ONE of the following identifiers should be defined.
	The UNKNOWN identifier should only be used when porting TECO-C to a
new environment.  When set,  it will cause stubs to be compiled for all the
system-dependent functions.
*****************************************************************************/

#ifdef OSX
#define LINUX
#define __GNUCC__
#endif

/*#define __GNUC__	automatically defined by FSF GNU C		*/
/*#define __POWERC	automatically defined by Mix Power C		*/
/*#define unix		automatically defined by unix C			*/
/*#define __TURBOC__	automatically defined by Borland Turbo C	*/
/*#define UNKNOWN	unknown compiler				*/
/*#define VAX11C	automatically defined by DEC VAX-11 C		*/
/*#define AMIGA		automatically defined by SAS Institute C	*/
/*#define EMX       defined for OS/2, GCC/EMX compiler */
/*#define WIN32     defined for Borland, 32 bit windows version */

/*****************************************************************************
	There are three things which can be enabled during debugging:

	CHECKSUM_CODE (under Turbo C only) will verify at run-time that
code isn't being corrupted by stray pointers.

	CONSISTENCY_CHECKING will check various pointers after each command
is executed and verify that they are "consistent"; for example, that edit
buffer pointers point into the edit buffer and not elsewhere.

	DEBUGGING enables code included at the entry and exit of every
function so a "function trace" can be displayed.  The unused TECO command
^P sets the level of detail which is displayed by the debugging routines
in TECOC.C.
*****************************************************************************/

#ifndef CHECKSUM_CODE			/* if not defined on command line */
#define CHECKSUM_CODE (FALSE && defined(__TURBOC__))
#endif
#ifndef CONSISTENCY_CHECKING		/* if not defined on command line */
#define CONSISTENCY_CHECKING FALSE	/* check pointer consistency */
#endif
#ifndef DEBUGGING			/* if not defined on command line */
#define DEBUGGING FALSE			/* compile debugging code */
#endif

/*****************************************************************************
	Determine what memory model we're running under Turbo C and set
whether we have small (near, 64K) or large (far, 1MB) code and data 
references.
*****************************************************************************/

#if defined(__TURBOC__)&&!defined(WIN32)

#define TC_SMALL_DATA (defined(__SMALL__) || defined(__MEDIUM__))
#define TC_SMALL_CODE (defined(__SMALL__) || defined(__COMPACT__))

#if DEBUGGING && TC_SMALL_CODE
/* GNU C objects to this #error directive even though it should skip it? */
/* redefining main() should stop the compiler */
/* #error DEBUGGING=TRUE needs large code model */
main() {}
#endif

#else

#define TC_SMALL_DATA FALSE
#define TC_SMALL_CODE FALSE

#endif

/*****************************************************************************
	The debugging code uses the standard library so we need to get
prototypes for the standard functions.  When not debugging, the only TECO-C
functions which need the standard library are in the Z*.C system-specific
code files which explicitly #include these header files.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*****************************************************************************
	Define VIDEO,  which compiles in some kind of video code,  even if
it's just stubs.  Define CURSES to get video support.  Note that the CURSES
is checked in some files before this file is included,  so it HAS to be
defined on the command line.
*****************************************************************************/

#ifdef CURSES			/* if CURSES is defined on command line */
#define VIDEO TRUE
#else
#define CURSES FALSE
#ifndef VIDEO			/* if VIDEO is not defined on command line */
#define VIDEO FALSE
#endif
#endif

/****************************************************************************
	The brain-damaged memory addressing of the IBM PC requires that all
character pointers used in TECO-C be "huge" pointers.  "Huge" pointers are
like "far" pointers in that they are 32-bits (a 16-bit SEGMENT and a 16-bit
OFFSET); however, when you do far pointer arithmetic, only the OFFSET is
affected so adding to a far pointer eventually causes the OFFSET to wrap
around to zero.  When you do huge pointer arithmetic, both the OFFSET and
the SEGMENT are properly updated.  Huge pointers are also normalized so
pointer comparisons work correctly.  For these reasons, all pointers
are declared using the following typedefs,  which are special on PCs.
****************************************************************************/

#if (defined(__TURBOC__) && !TC_SMALL_DATA && !defined(WIN32)) || defined(__POWERC)
#define _Huge huge
#else
#define _Huge /**/
#endif

typedef unsigned char _Huge *	charptr;
#if defined(unix)
typedef unsigned char *		voidptr;
#else
typedef void _Huge *		voidptr;
#endif
typedef struct EStck _Huge *	ESptr;
typedef struct MStck _Huge *	MSptr;
typedef struct QReg  _Huge *	QRptr;

#undef _Huge

/****************************************************************************
	Some compilers allow function prototypes to be defined for functions
even when the function declarations aren't written in prototype form.  This
allows checking of function calls for correctness,  but doesn't guarantee
that prototypes and declarations match.  I choose to leave the actual
function declarations in non-prototype form,  so they work with older
compilers.  Defining USE_PROTOTYPES to TRUE causes prototypes to be defined
in TECOC.H for all functions.  Defining it FALSE causes old-style definitions
in TECOC.H.
	GNU C is one of the compilers that forces consistency:  if you use
prototypes,  then declarations have to match the prototypes.  The only way
I can do this in TECOC code is to surround each function declarations with
"#if USE_PROTOTYPES...#else...#endif" directives.  This looks hideous, so I
just don't use prototypes in GNU C.
****************************************************************************/

#if defined(VAX11C) || defined(__TURBOC__) || defined(__POWERC) || defined(ULTRIX)
#define USE_PROTOTYPES TRUE
#else
#define USE_PROTOTYPES FALSE
#endif

/****************************************************************************
	ANSI C compilers are supposed to concatenate adjacent string literals
in source code.  If they do,  a superior form of the command-line parsing
macro (in CLPARS.H, used by ZPrsCL), is used.  If not,  a klunkier form needs
to be used.
	Note:  The clpars[] string is over 3 Kbytes long, so in addition to
supporting string concatenation, the compiler must support long strings.  The
Power C v1.3.0 compiler, for example, supports ANSI string concatenation, but
does not like the 3 Kbyte clpars[] string.
****************************************************************************/

#if defined(__TURBOC__) || defined(__GNUC__) || defined(WIN32)
#define USE_ANSI_CLPARS TRUE
#else
#define USE_ANSI_CLPARS FALSE
#endif

/*****************************************************************************
	Define ptrdiff_t (the type of things that can hold the result when
one pointer is subtracted from another), size_t (the size of memory objects
and repeat counts), and NULL.
*****************************************************************************/

#if defined(unix) || defined(UNKNOWN) || defined(AMIGA) && !defined(ULTRIX)

#if sun
#ifdef SUNOS4_0
typedef int	ptrdiff_t;	/* not in /sys/types.h yet */
#endif
#include <sys/types.h>		/* size_t (and maybe ptrdiff_t) */
#else /* sun */
#ifdef LINUX
#define ptrdiff_t int
#else
typedef int	ptrdiff_t;
typedef ULONG	size_t;
#endif
#endif /* sun */

#ifdef AMIGA
#define NULL 0L
#else
#ifdef LINUX
#ifndef NULL
#define NULL (void *)0
#endif
#else
#define NULL 0
#endif
#endif

#else
#include <stddef.h>		/* define ptrdiff_t, size_t and NULL */
#endif

/*****************************************************************************
	Define SIZE_T,  which on most machines will be the same as size_t.
There are problems (as usual) on the IBM PC.  In various places,  TECO-C
subtracts two pointers and puts the resulting value in a variable of type
SIZE_T.  Under Turbo C,  size_t is 16 bits,  which won't work,  so we
use unsigned long for SIZE_T for TURBO C.
*****************************************************************************/

#if (defined(__TURBOC__) && !TC_SMALL_DATA && !defined(WIN32)) || defined(__POWERC)
typedef ULONG SIZE_T;		/* PCs suck */
#elif defined(unix)
typedef unsigned int SIZE_T;	/* override their size_t, which is "int" */
#else
typedef size_t SIZE_T;		/* use size_t as defined above */
#endif

/*****************************************************************************
	The values of the following identifiers are system-dependent.

EXTERN	usually "extern",  this identifier lets the more useful keyword
	"globalref" be used to define an external variable in VAX C.
GLOBAL	used for the single definition of a variable.  This identifier
	allows the use of the keyword "globaldef" in VAX C.
EXTERNV used only for GotCtC, this means "extern volatile" for compilers that
	support the "volatile" keyword.
GLOBALV used only for GotCtC, this means "volatile" for compilers that
	support the "volatile" keyword.
VVOID	if the compiler supports the keyword "void",  then this should be
	defined as such,  else it should be meaningless.  It would be nice
	to use "VOID" instead of "VVOID",  but the damn curses.h file defines
	"VOID".

EBFEXP	amount by which the edit buffer is expanded each time the user tries
	to insert too much text.
EBFINIT	initial size of the edit buffer.  This value is added to IBFINIT
	to create a value used to initially allocate both buffers.
ERBFSIZ Size of error message buffer.  A missing tag message for a bad Otag$
	command might be long...
GAPMIN	minimum size of the edit buffer gap.  This value should be set such
	that there's enough room to hold the text of the largest expected
	insertion command.
IBFEXP	amount by which the input buffer is expanded each time it becomes
	too small for a reasonable-length line (IBFMIN) to be read.
IBFINIT	initial size of the input buffer.  This value is added to EBFINIT
	to create a value used to initially allocate both buffers.
IBFMIN	minimum size of the input buffer.  This value is checked before each
	read operation.
NIFDBS	number of input file data blocks.  This should be 3 plus the
	number of levels that EI files may be nested.  8 is a good number.
SBFINIT	size of search string buffer
TBFINIT	size of tag buffer for O command, max size of label to search for
ZBFEXP	amount by which the EI file buffer is expanded when there are less
	than ZBFMIN bytes left in the buffer.
ZBFINIT	initial size of the file buffer used to read in a macro executed by
	an "EIfilespec$$" command.
ZBFMIN	minimum size of the EI file buffer before it is expanded.
*****************************************************************************/

#if defined(VAX11C)

#define EXTERN	globalref	/* to reference an external symbol */
#define GLOBAL	globaldef	/* to define an external symbol */
#define EXTERNV	extern volatile	/* to reference an external volatile symbol */
#define GLOBALV	volatile	/* to define an external volatile symbol */
#define VVOID	void		/* Void function return */

#define CBFINIT		20000	/* command buffer initial size */
#define EBFEXP		64000	/* edit buffer expansion value */
#define EBFINIT		64000	/* edit buffer initial size */
#define ERBFSIZ		1024	/* error message buffer */
#define GAPMIN		64000	/* edit buffer gap minimum size */
#define IBFEXP		64000	/* input buffer expansion size */
#define IBFINIT		64000	/* input buffer initial size */
#define IBFMIN		10000	/* input buffer minimum size */
#define NIFDBS		8	/* number of input file data blocks */
#define SBFINIT		1000	/* size of search string buffer */
#define TBFINIT		1000	/* size of tag buffer for O command */
#define WBFINIT		1000	/* batch input and output buffer size */
#define ZBFEXP		64000	/* EI file buffer expansion value */
#define ZBFINIT		64000	/* EI file buffer initial size */
#define ZBFMIN		1000	/* EI file buffer minimum size */

#define EXS_SIZE	64	/* size of expression stack */
#define LPS_SIZE	32	/* size of loop stack */
#define MCS_SIZE	32	/* size of macro stack */
#define QRS_SIZE	32	/* size of q-register stack */

#elif defined(__TURBOC__) || defined(__POWERC) || defined(unix) || defined(AMIGA) || defined(OSX)  ||  defined(WIN32)

#define EXTERN	extern		/* to reference an external symbol */
#define GLOBAL	/**/		/* to define an external symbol */
#if defined(AMIGA) || defined(unix) && !defined(__GNUC__)
#define EXTERNV	extern		/* to reference an external volatile symbol */
#define GLOBALV	/**/		/* to define an external volatile symbol */
#else
#define EXTERNV	extern volatile	/* to reference an external volatile symbol */
#define GLOBALV	volatile	/* to define an external volatile symbol */
#endif

#if defined(__TURBOC__) || defined(__POWERC) || defined(__GNUC__) || defined(AMIGA) || defined(OSX) || defined(WIN32)
#define VVOID		void	/* Void function return */
#else
#define VVOID		/**/	/* Void function return */
#endif

#if TC_SMALL_DATA

/*
 * with the DEBUGGING #define off, Turbo C can compile teco-c for the small
 * data model which is sometimes easier to debug.  use "make -DMODEL=s or m".
 *
 * define smaller buffer sizes
 */
 
#define CBFINIT		100	/* command buffer initial size */
#define EBFEXP		1000	/* edit buffer expansion value */
#define EBFINIT		8000	/* edit buffer initial size */
#define ERBFSIZ		100	/* error message buffer */
#define GAPMIN		1000	/* edit buffer gap minimum size */
#define IBFEXP		1000	/* input buffer expansion size */
#define IBFINIT		8000	/* input buffer initial size */
#define IBFMIN		1000	/* input buffer minimum size */
#define NIFDBS		6	/* number of input file data blocks */
#define SBFINIT		100	/* size of search string buffer */
#define TBFINIT		100	/* size of tag buffer for O command */
#define ZBFEXP		1000	/* EI file buffer expansion value */
#define ZBFINIT		8000	/* EI file buffer initial size */
#define ZBFMIN		1000	/* EI file buffer minimum size */

/*
 * define far heap management calls used in ZMSDOS.C in terms of
 * near heap functions.
 */

#define farfree(p) free(p)
#define farmalloc(ul) malloc((unsigned int)ul)
#define farrealloc(p,ul) realloc((p),((unsigned int)ul))

#else

#define CBFINIT		20000	/* command buffer initial size */
#define EBFEXP		64000L	/* edit buffer expansion value */
#define EBFINIT		64000L	/* edit buffer initial size */
#define ERBFSIZ		128	/* error message buffer */
#define GAPMIN		64000L	/* edit buffer gap minimum size */
#define IBFEXP		64000L	/* input buffer expansion size */
#define IBFINIT		64000L	/* input buffer initial size */
#define IBFMIN		1000	/* input buffer minimum size */
#define NIFDBS		8	/* number of input file data blocks */
#define SBFINIT		1000	/* size of search string buffer */
#define TBFINIT		1000	/* size of tag buffer for O command */
#define ZBFEXP		8000	/* EI file buffer expansion value */
#define ZBFINIT		16000	/* EI file buffer initial size */
#define ZBFMIN		1000	/* EI file buffer minimum size */

#ifdef WIN32
/*
 * define far heap management calls used in ZMSDOS.C in terms of
 * near heap functions.
 */

#undef farfree
#undef farmalloc
#undef farrealloc

#define farfree(p) free(p)
#define farmalloc(ul) malloc((unsigned int)ul)
#define farrealloc(p,ul) realloc((p),((unsigned int)ul))
#endif

#endif

#define EXS_SIZE	64	/* size of expression stack */
#define LPS_SIZE	32	/* size of loop stack */
#define MCS_SIZE	32	/* size of macro stack */
#define QRS_SIZE	32	/* size of q-register stack */

#elif defined(EMX)

#define EXTERN	extern		/* to reference an external symbol */
#define GLOBAL	/**/		/* to define an external symbol */
#define EXTERNV	extern volatile	/* to reference an external volatile symbol */
#define GLOBALV	volatile	/* to define an external volatile symbol */
#define VVOID	void		/* Void function return */

#define CBFINIT		20000	/* command buffer initial size */
#define EBFEXP		64000	/* edit buffer expansion value */
#define EBFINIT		64000	/* edit buffer initial size */
#define ERBFSIZ		128		/* error message buffer */
#define GAPMIN		64000	/* edit buffer gap minimum size */
#define IBFEXP		64000	/* input buffer expansion size */
#define IBFINIT		64000	/* input buffer initial size */
#define IBFMIN		1000	/* input buffer minimum size */
#define NIFDBS		8		/* number of input file data blocks */
#define SBFINIT		1000	/* size of search string buffer */
#define TBFINIT		1000	/* size of tag buffer for O command */
#define ZBFEXP		8000	/* EI file buffer expansion value */
#define ZBFINIT		16000	/* EI file buffer initial size */
#define ZBFMIN		1000	/* EI file buffer minimum size */

#define EXS_SIZE	64	/* size of expression stack */
#define LPS_SIZE	32	/* size of loop stack */
#define MCS_SIZE	32	/* size of macro stack */
#define QRS_SIZE	32	/* size of q-register stack */

#elif defined(UNKNOWN)

#define EXTERN	extern		/* to reference an external symbol */
#define GLOBAL	/**/		/* to define an external symbol */
#define EXTERNV	extern volatile	/* to reference an external volatile symbol */
#define GLOBALV	volatile	/* to define an external volatile symbol */
#define VVOID	void		/* Void function return */

#define CBFINIT		100	/* command buffer initial size */
#define EBFEXP		100	/* edit buffer expansion value */
#define EBFINIT		100	/* edit buffer initial size */
#define ERBFSIZ		100	/* error message buffer */
#define GAPMIN		100	/* edit buffer gap minimum size */
#define IBFEXP		100	/* input buffer expansion size */
#define IBFINIT		100	/* input buffer initial size */
#define IBFMIN		100	/* input buffer minimum size */
#define NIFDBS		4	/* number of input file data blocks */
#define SBFINIT		100	/* size of search string buffer */
#define TBFINIT		100	/* size of tag buffer for O command */
#define ZBFEXP		100	/* EI file buffer expansion value */
#define ZBFINIT		100	/* EI file buffer initial size */
#define ZBFMIN		100	/* EI file buffer minimum size */

#define EXS_SIZE	5	/* size of expression stack */
#define LPS_SIZE	5	/* size of loop stack */
#define MCS_SIZE	5	/* size of macro stack */
#define QRS_SIZE	5	/* size of q-register stack */

#endif

/*****************************************************************************
	Define ZCHKKB, a macro that does a keyboard check under MS-DOS to keep
the Control-Break (Control-C) handler happy.  Under MS-DOS a Control-C isn't
checked for until you make a DOS call to the keyboard, screen, or printer.
To make loops Control-C'able in ExeCSt we make a call to ZCHKKB before
checking GotCtC so that if a Control-C was pressed in a TECO-C loop which
doesn't make an explicit DOS call, ZCHKKB makes a no-op DOS call, which
allows GotCtC to be set correctly.

	Should we use Turbo C's setcbrk to make sure BREAK is on ???

	Under other operating systems, ZCHKKB() expands to nothing.
*****************************************************************************/

#if defined(__TURBOC__)
#include <conio.h>			/* kbhit() prototype */
#define ZCHKKB() ((void)kbhit())	/* check keyboard */
#else
#define ZCHKKB() /**/			/* expand to nothing */
#endif

/*****************************************************************************
	Define ZMKOFN, a macro called only by ExeEB.  It's needed because the
semicolon and file version number have to be stripped off of the end of the
fully expanded file name that's in FBf before we can call ZOpOut.
*****************************************************************************/

#if defined(VAX11C)
#define ZMKOFN() do { FBfPtr--;} while (*FBfPtr != ';');
#else
#define ZMKOFN() /**/		/* expand to nothing */
#endif

/*****************************************************************************
	Define MEMMOVE,  a macro that copies "len" bytes from "source" to
"dest" correctly,  even if "source" and "dest" overlap.  In most cases,  this
maps to the ANSI C function "memmove".  If it doesn't,  there's C code that
does the job in function ZCpyBl,  which is in the Z*.C system-specific code
files.
*****************************************************************************/

#if defined(sun)

EXTERN VVOID bcopy();
#define MEMMOVE(dest,source,len) bcopy((source),(dest),(len))

#elif defined(unix) || defined(AMIGA) || (defined(UNKNOWN) && !defined(__STDC__))

EXTERN VVOID ZCpyBl();
#define MEMMOVE(dest,source,len) ZCpyBl((dest),(source),(len))

#elif defined(__TURBOC__) || defined(__POWERC)

EXTERN VVOID ZCpyBl(charptr dest, charptr source, SIZE_T len);
#define MEMMOVE(dest,source,len) ZCpyBl((dest),(source),(len))

#else

#define MEMMOVE(dest,source,len) memmove((dest),(source),(len))

#endif

/*****************************************************************************
	Define EXIT_SUCCESS and EXIT_FAILURE
*****************************************************************************/

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

/*****************************************************************************
	Define FILENAME_MAX, which is used in Init() and in the Z*.C files.
This is the maximum size of a filename.  It's supposed to be defined in
stdio.h by ANSI C compilers,  but some compilers don't.

	In VAX C v3.0, STDIO.H defines FILENAME_MAX as 39,  which is the
length of only the name part of a full file specification.  I'm not sure what
use this is to anybody,  because most VAX file names include a file type,  if
not a version number, directory specification or node specification.  So if
we're on a VAX,  we override stdio's definition with the right one here.
*****************************************************************************/

#if defined(unix)

#ifndef FILENAME_MAX
#define FILENAME_MAX 64
#endif

#elif defined(VAX11C)

#undef FILENAME_MAX			/* undefine stdio.h's definition */
#include nam
#define FILENAME_MAX NAM$C_MAXRSS	/* and define the correct one */

#elif defined(__TURBOC__) || defined(__POWERC)

#ifndef FILENAME_MAX
#define FILENAME_MAX 80			/* MAXPATH */
#endif

#elif defined(AMIGA)
/*
 * This is filesystem dependent on the Amiga, and there is no real good
 * way to get at it. This value is true for FileSystem and FastFileSystem,
 * but probably isn't for other things (like mounted MS-DOS partitions...)
 */
#define FILENAME_MAX 255

#elif defined(UNKNOWN)

#define FILENAME_MAX 20

#endif

/*****************************************************************************
	When compiling with Turbo C set to use smaller and faster Pascal
function calling sequences (tcc -p), main() in TECOC.C and CntrlC() in
ZMSDOS.C have to be specifically declared as C functions.  The _Cdecl
#define in STDDEF.H takes care of this for Turbo.  Define it as nothing
for everyone else.
*****************************************************************************/

#if !defined(_Cdecl)
#define _Cdecl /**/
#endif


/*****************************************************************************
	Bit masks for EZ mode control flag.  These are system-dependent.
*****************************************************************************/

#if defined(unix) | defined(OSX)
#define EZ_NO_VER	  1	/* do not do VMS style versions */
#define EZ_ARROW	  8	/* */
#define EZ_AUDIO_BEEP	  16	/* (CURSES only) on = beep , off = flash */
#define EZ_WIN_LINE	  32	/* (CURSES only) line between windows */
#define EZ_FF		  128	/* do not stop read on FF */
#define EZ_UNIXNL	  256	/* use Unix-style newline terminators */
#define EZ_VT100GRAPHICS  512
#define EZ_BTEE		  2048	/* use BTEE instead of DIAMOND */
#define EZ_INVCR	  8192	/* don't show CR in scope - closer to */
				/* TECO-11, but really not as good in */
				/* my opinion (Mark Henderson) */
#if defined(LINUX) | defined(OSX)
#define EZ_NO_STRIP    16384 /* Don't strip the extension */
#endif
#elif defined(AMIGA)
#define EZ_FF		128	/* do not stop read on FF */
#define EZ_UNIXNL	256	/* use Unix-style newline terminators */
#endif
