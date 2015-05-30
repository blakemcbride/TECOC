	AAREADME.TXT

	This directory contains TECO-C,  a version of TECO written in C.
It was written in C so the author could move comfortably from VAX/VMS
to various other machines,  including MicroVaxes,  which couldn't execute
TECO-11 prior to VMS 5.0 because they don't support PDP-11 compatibility
mode.  TECO32,  distributed with v5.0,  solved this problem.

	TECO-C is meant to be a complete implementation of TECO as defined
by the Standard TECO User's Guide,  which is in file TECO.DOC.  There is no
manual for TECO-C itself,  but the Standard TECO manual was the specification
for TECO-C,  so it serves as an excellent manual.  TECO-C departs from the
Standard manual in only a few places (like no video mode),  listed in file
PROBLEMS.TXT.  There is quick-and-dirty "wall chart" of all TECO commands
in file WCHART.TXT.

	This version of TECO-C runs under VMX/VMS, MS-DOS, AmigaDOS and Unix
(SunOS, Ultrix and System V on a Data General Aviion).  If you find bugs or
have any comments about TECO-C, please contact

	Pete Siemsen			Pete Siemsen
	645 Ohio Avenue #302		University of Southern California
	Long Beach, Ca. 90814		1020 West Jefferson Blvd.
					Los Angeles, CA 90089-0251

	(213) 433-3059  (home)		(213) 740-7391  (work)

			Internet: siemsen@usc.edu

	The following sections describe how to use or build TECO-C for
different environments.  The command line syntax is the same as the syntax
for TECO-11 command lines.


###########################################################################
			U N D E R    V A X / V M S

	TECO-C was mostly developed under VAX/VMS.  It compiles and links
using VAX C 3.1 under VMS version 5.3.

To set up:	inspect and modify ZVLOGIN.COM and then add it to your
		LOGIN.COM

To compile:	inspect/edit ZVBLD.COM,  then SUBMIT it

To link:	use the "LT" command (defined in ZVLOGIN.COM)

To run:		inspect/edit the symbol definitions in ZVLOGIN.COM
		Note: TECO-C uses the same command line syntax as TECO32


###########################################################################
			U N D E R   M S - D O S

	TECO-C compiles/links under Turbo C v2.00 under MS-DOS 4.01 and
Concurrent DOS 386 v3.0.

To build:	run MAKE on MAKEFILE.TC.  to build TECO from scratch say:
		"make -fmakefile.tc -DALL".  see MAKEFILE.TC for other make
		-D options.  also see MAKETECO.BAT.

To run:		use TECO.BAT to edit a file or MUNG.BAT to run a macro,
		for example:  TECO filename.


###########################################################################
			U N D E R    U N I X

To compile and link:  use the make utility on "makefile.sun" (for SunOS) or
	"makefile.ulx" (for Ultrix) or "makefile.dg" (for System V Unix
	on a Data General Aviion).

To run:	define aliases and environment variables in your .cshrc file.
        Here's an example,  assuming my initialization file is named
	.tecorc and it's in my home directory,  and my macros are in a
        directory named tecoc under my main directory.  You'll have to
	change the directory names, of course.

	        setenv TEC_INIT \$/home/sol/siemsen/.tecorc
	        setenv TEC_LIBRARY /home/sol/siemsen/tecoc/
	        alias te   '/home/sol/siemsen/tecoc/tecoc teco -nocreate'
	        alias mung '/home/sol/siemsen/tecoc/tecoc mung'
	        alias tma  '/home/sol/siemsen/tecoc/tecoc make'

After doing this, "te" runs tecoc.


Unix Command line 
---- ------- ----

To make Unix users more comfortable,  TECO-C uses Unix-like options syntax
instead of the familiar "/" style used under VAX/VMS.  The options can be
abbreviated.  They are (optional characters in square brackets):

-in[spect]
-noc[reate]
-noi[ni]
-nom[emory]
-sc[roll]
-scroll:nnn:SEEALL
	do nnn,7:W and 1,3:W


-nop[age]
	Don't separate input into pages.  Instead,  treat form feeds as
	normal characters.
+nnn 
	where nnn is any number of digits. Go to line number nnn and set
	nopage. Note this uses the UNIX end of line convention for use with
        cc output &c. The non-unix line termination will still be used for
	nL and other intrinsic TECO commands, this just controls the
	starting line AT INVOCATION.


EZ Mode Control Flag
-- ---- ------- ----

To add functionality,  TECO-C is sensitive to bits in an extra mode control
flag in addition to the standard ones.  Bits in the EZ mode control flag are
used in different ways by different operating systems.  Under Unix,  bits in
the flag have the following meanings:

 1	Mark Henderson,  who did much of the Unix port,  likes the way
	VAX/VMS	keeps versions of files.  VMS appends a semicolon followed
	by a version number to files,  and has the PURGE command to clean
	out old versions of files.  If this bit is off,  TECO-C will handle
	file version numbers,  appending the appropriate version number to
	file names.  Mark supplied a "purge" program (distributed with TECO-C)
	for users of this feature.  Setting this flag turns off the feature,
	to make TECO-C function as expected by the average Unix user.  This
	flag is set by default.

 8	I don't know what this flag does.  It only makes sense when TECO-C
	is compiled with CURSES support.

 16	If set, when it's time to beep,  use an audio beep,  and if it fails,
	then try a video flash.  If clear,  try a video flash,  and if it
	fails,  use an audio beep.  This bit works only if TECO-C was compiled
	with CURSES support.  This bit is off by default.

 32	If set,  and split screen scrolling mode is on (see the 7:W command),
	TECO-C puts a line between the upper and lower scrolling regions.
	If off,  don't display a line.  This bit works only if TECO-C was
	compiled with CURSES support.  This bit is off by default.

 128	If set,  don't stop on form feeds when reading.  If clear,  a form
	fed in the input stream stops the read (the standard TECO style).
	This bit is off by default.

 256	If set,  use Unix-style newline terminators.  This means when files
	are read in,  carriage-returns are not added to the end of lines,
	and when the user types the RETURN key,  only a newline is entered
	into the command line (usually a carriage-return/line-feed pair is
	entered).  Old macros (and old TECO users) may get confused if this
	bit is set,  but it's required if CURSES support is compiled into 
	TECO-C.

 2048	If set,  use a bottom-tee symbol instead of a diamond to indicate
	the end-of-buffer.  This bit works only if TECO-C was compiled with
	CURSES support.  This bit is off by default.
	
 8192	If set,  don't show CR in SCROLL mode (like standard TECO).  If clear,
	show CR (a graphics character indicating a carriage return) when in
	SCROLL mode.  This bit works only if TECO-C was compiled with CURSES
	support.  This bit is on by default.


Filename Memory
-------- ------

TECO tries to remember the last file edited by TECO,  so that after you've
edited a file once,  you can re-edit without having to type the file name
on the command line.  TECO-C implements this under Unix by writing and reading
files named /tmp/tecoxxx.tmp,  where "xxx" is the process id of the parent
process.


NOTE:	There is another TECO in C,  written by Matt Fichtenbaum,  which
runs under Unix.  It is part of the DECUS TECO Collection.  Contact Pete
Siemsen at the above address for a copy.

###########################################################################
			U N D E R   A M I G A D O S

To compile and link:	see makefile.ami,  (note: it needs tecoc.lnk)

Comments from the author of the Amiga code:

    I've completed my TECOC port to the Amiga. It has been tested on an 
    Amiga 1000 under AmigaDOS 1.3.2. My system configuration is:
    
            68000 microprocessor
            2.5 megs memory
            80 meg hard drive
            Old Amiga chip set
            SAS Institute C for the Amiga version 5.10
    
    TECO-C runs in the CLI window it was started from, and it should run
    over a terminal hooked to an Amiga serial port as well.
    
    Adding 'W' command stuff would probably best be done by one of the
    termcap ports to the Amiga, however, TECO would have to handle
    SIGWINCH-like events, unless it is modified to open it's own window of
    a particular size without a resizing gadget. It wouldn't be to hard to
    write a terminal driver for Amiga windows using hard-coded escape
    sequences either, but then that would lose over a serial port.

EzFlag
------

The EZ flag is system-dependent.  Under AmigaDOS,  bits in the flag have the
following meanings:

 128	If set,  don't stop on form feeds when reading.  If clear,  a form
	fed in the input stream stops the read (the standard TECO style).
	This bit os off by default.

 256	If set,  use Unix-style newline terminators.  This means when files
	are read in,  carriage-returns are not added to the end of lines,
	and when the user types the RETURN key,  only a newline is entered
	into the command line (usually a carriage-return/line-feed pair is
	entered).  Old macros (and old TECO users) may get confused if this
	bit is set,  but it's required if CURSES support is compiled into 
	TECO-C.




###########################################################################
				F I L E S


	aareadme.txt	this readme file
	baksrc.c	backwards search
	bldstr.c	build a string with string build constructs
	change.tec	change macro squished
	change.tes	change macro source.  this macro allows you to
			replace one string with another in a group of
			files using match constructs.
	changes.txt	contains teco-c release notes starting from
			version 1.00 (TVERSION = 100)
	chmacs.h	ctype.h for teco-c
	clenup.c	clean up before exiting teco-c
	clpars.h	command line parsing macro, created by genclp.c
	clpars.tec	command line parsing macro squished
	clpars.tes	command line parsing macro source
	cmatch.c	match character with match construct
	date.tec	date macro squished
	date.tes	date macro source.  this macro displays the
			date and time.
	dchars.h	define identifiers for control characters
	deferr.h	define identifiers for error messages
	defext.h	EXTERN decarations for global variables
	detab.tec	de-tabify macro squished
	detab.tes	de-tabify macro source.  this macro converts
			tabs to spaces
	dir.tec		"display directory" macro, in squished form
	dir.tes		"display directory" macro source.  This macro simply
			displays the directory.  a wildcarded filename
			in the edit buffer can control what is displayed.
	docjr.c		code common to ExeC, ExeJ, and ExeR
	doeves.c	work code for EV and ES
	doflag.c	changes a mode control flag (ED,EH,ES,ET,EU,EV,^X)
	dscren.h	terminal types for 0:W command
	echoit.c	display character in its printable form
	err.c		display teco-c's error messages
	exea.c		execute A
	exeats.c	execute at-sign @
	exeb.c		execute B
	exebar.c	execute |
	exebsl.c	execute backslash \
	exec.c		execute C
	execcc.c	execute control-caret ^^
	execln.c	execute colon modifiers : and ::
	execom.c	execute comma ,
	execrt.c	execute caret ^
	execst.c	execute a TECO command string
	execta.c	execute ^A
	exectc.c	execute ^C
	exectd.c	execute ^D
	execte.c	execute ^E
	execti.c	execute ^I
	exectl.c	execute ^L
	exectn.c	execute ^N
	execto.c	execute ^O
	exectp.c	execute ^P
	exectq.c	execute ^Q
	exectr.c	execute ^R
	exects.c	execute ^S
	exectt.c	execute ^T
	exectu.c	execute ^U
	exectv.c	execute ^V
	exectw.c	execute ^W
	exectx.c	execute ^X
	execty.c	execute ^Y
	exectz.c	execute ^Z
	exed.c		execute D
	exedgt.c	execute digit (0-9)
	exedot.c	execute dot .
	exedqu.c	execute double quote "
	exee.c		execute E
	exeequ.c	execute equals =, ==, and ===
	exeesc.c	execute escape
	exeexc.c	execute exclamation point !
	exeey.c		execute EY
	exef.c		execute F
	exefb.c		execute FB
	exeg.c		execute G
	exegtr.c	execute greater than sign >
	exeh.c		execute H
	exei.c		execute I
	exeill.c	"execute" illegal command
	exej.c		execute J
	exek.c		execute K
	exel.c		execute L
	exelbr.c	execute left-bracket [
	exelst.c	execute less than sign <
	exem.c		execute M
	exen.c		execute N
	exenul.c	execute null ^@
	exenyi.c	"execute" not-yet-implemented command
	exeo.c		execute O
	exeopr.c	execute operator (+,-,(,#,/,&,^_,*)
	exep.c		execute P (see singlp.c)
	exeprc.c	execute percent sign %
	exepw.c		execute PW
	exeq.c		execute Q
	exeqes.c	execute question mark ?
	exer.c		execute R
	exerbr.c	execute right bracket ]
	exertp.c	execute right paren )
	exes.c		execute S
	exescl.c	execute semicolon ;
	exet.c		execute T
	exeu.c		execute U
	exeund.c	execute underscore _
	exeusc.c	execute unit separator character ^_
	exev.c		execute V
	exew.c		execute W
	exex.c		execute X
	exey.c		execute Y
	exez.c		execute Z
	findes.c	find end of string
	findqr.c	find q-register name
	flowec.c	flow to end of conditional
	flowee.c	flow to else part of conditional
	flowel.c	flow to end of loop
	fmtmac.tec	PDP-11 .MAC formatter squished
	fmtmac.tes	PDP-11 .MAC formatter source.  this macro does
			case formatting for PDP-11 macro (.MAC) source files
	format.tec	FORTRAN formatter squished
	format.tes	FORTRAN formatter source.  formats the first part of
			a FORTRAN source line.
	genclp.c	generates the command line parsing macro in clpars.h
	getara.c	get area in edit buffer an "m,n" argument defines
	getnma.c	get numeric argument off expression stack
	inccbp.c	increments command buffer pointer
	init.c		teco-c initializatin routines
	insstr.c	insert string into edit buffer
	isradx.c	is digit within current radix?
	ln2chr.c	convert line-number to character-number (^Q)
	lowcase.tec	convert edit buffer to lowercase macro, see upcase.tec
	makdbf.c	make digit buffer, binary to ascii in current radix
	makefile.ami	AmigaDOS make file 
	makefile.cct	CodeCenter under SunOS
	makefile.dg	System V Unix make file (works on Data General Aviion)
	makefile.sun	SunOS make file
	makefile.tc	Turbo C make file (MS-DOS)
	makefile.ulx	Ultrix make file
	maketeco.bat	MS-DOS batch file to make teco.  this is probably how
			the temdos.exe executable file was made.
	makprnt.tec	make printable macro squished
	makprnt.tes	make printable macro squished.  this macro makes a
			string with embedded control characters printable
	makrom.c	make room in Q-register for new text
	mung.bat	MS-DOS batch to execute a teco macro from command line
	pg.mem		a programmer's guide.  it may be useful to anyone
			fixing bugs, adding features, or porting teco-c to
			a new environment.
	pkzip.rsp	a PKZIP response file used to build TECOC.ZIP archive
	popmac.c	pop macro environment off macro stack
	problems.txt	contains notes about bugs and ideas
	pshmac.c	push current macro environment on macro stack
	pushex.c	push expression onto stack, try to reduce it
	rdline.c	read a line
	rdpage.c	read a page
	readcs.c	read command string
	replac.c	code for search and replace commands
	search.c	code for most of the search comamnds
	search.tec	search macro squished
	search.tes	search macro source.  this macro finds a string in
			a group of files using teco match constructs
	singlp.c	does a single P command
	skpcmd.c	skip over current command in command bufer
	squ.bat		MS-DOS batch file to run squ.tec w/standard options
	squ.tec		squish macro squished
	squ.tes		squish macro source.  this macro takes a teco macro
			source file and makes it as small and as fast (and
			as unreadable) as possible
	srclop.c	search loop, handles loop arguments for search 
	sserch.c	do a simple search
	sub.tec		?
	tabort.c	terminate teco-c
	tctlib.rsp	a Turbo C TLIB response file used by makefile.tc
	teco.bat	MS-DOS batch file to run teco-c
	teco.doc	Standard TECO documentation from DECUS
	teco.tes	teco initialization file
	tecoc.c		the main source module, contains all the definitions
			of system-independent global variables, as well as
			comments explaining what they are.
	tecoc.exe	the teco-c executable for VMS
	tecoc.h		defines structures, general identifiers, and
			the in-line debugging scheme.
	tecoc.lnk	link file needed under AmigaDOS (see makefile.ami)
	tecoin.tes	teco initialization file
	temsdos.exe	the teco-c executable for MS-DOS
	tst.bat		MS-DOS batch file to run the test macros
	tst.com		VMS command procedure to run the test macros
	tstbsl.tec	--
	tstcss.tec	 |
	tstequ.tec	 | these macros contain code which tests some
	tsto.tec	 | aspects of teco-c
	tstpw.tec	 |
	tstqr.tec	 |
	tstsrc.tec	--
	typbuf.c	types a buffer on the screen, converting unprintable
			characters to printable as nescessary.
	type.tec	type a file macro squished
	type.tes	type a file macro source.  this macro apparently
			types a file on the console with all kinds of
			wonderful options.
	typest.c	types the erroneous command string for errors
	uminus.c	unary minus (converts -C to -1C)
	unsqu.tec	unsquish macro squished
	unsqu.tes	unsquish macro source.  this macro does some simple
			formatting of a squished teco macro to make it more
			readable (like indenting)
	upcase.tec	convert edit buffer to uppercase macro, see lowcase.tec
	vrbmsg.h	verbose forms of teco error messages
	vtedit.tec	vtedit macro squished
	vtedit.tes	vtedit macro source.  humongous screen editor macro.
			note: this macro does NOT run under teco-c.
	wchart.txt	contains a wall chart of TECO commands
	wrpage.c	write a page
	zamiga.c	AmigaDOS system dependent code.
	zfirst.c	dummy module used if CHECKSUM_CODE is TRUE in zport.h
	zfrsrc.c	do a forward search using VAX SCANC instruction
	zmsdos.c	MS-DOS system dependant code
	zport.h		portability header file.
	zunix.c		SunOS (Unix) system dependant code
	zunkn.c		UNKNOWN system dependant code (function stubs)
	zvms.c		VAX/VMS system dependant code


	The W command does not work,  but the EXEW.C file contains code
to tell you what the command would have done if it had been executed.  I used
this to play with executing VTEDIT.TEC under TECOC in order to find bugs.

	I maintain the DECUS TECO Collection,  a bunch of stuff relating to
TECO.  It includes sources for DEC's TECO-11 v39, three TECOs written in C,
including ones for VAX/VMS, MS-DOS, SunOS (Unix) and Ultrix,  the current
Standard TECO manual, two EMACs-like macro packages and LOTS of macros.

	I you have something that you feel should be included in this
collection,  or if you would like to improve the software,  please contact
me at the above address.


Pete Siemsen
