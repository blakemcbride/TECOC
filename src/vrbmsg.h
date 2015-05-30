/****************************************************************************
	This file contains the verbose forms of TECO error messages.  It is
an include file because the contents are shared by at least two of the "Z"
files,  and the contents are large.
	IMPORTANT:  If you change this file,  you will need to change
TECOC.RNH,  ERR.C and DEFERR.H !!!
*****************************************************************************/
#if defined(__POWERC)
static char *ParaTx[] = {
	"Verbose messages not loaded",
	NULL
};
static int StartP[] = {
	/* ERR_ARG 0 */  0,	/* ERR_BNI 1 */  0,	/* ERR_DTB 2 */  0,
	/* ERR_FNF 3 */  0,	/* ERR_ICE 4 */  0,	/* ERR_IEC 5 */  0,
	/* ERR_IFC 6 */  0,	/* ERR_IIA 7 */  0,	/* ERR_ILL 8 */  0,
	/* ERR_ILN 9 */  0,	/* ERR_IPA 10 */ 0,	/* ERR_IQC 11 */ 0,
	/* ERR_IQN 12 */ 0,	/* ERR_IRA 13 */ 0,	/* ERR_ISA 14 */ 0,
	/* ERR_ISS 15 */ 0,	/* ERR_IUC 16 */ 0,	/* ERR_MAP 17 */ 0,
	/* ERR_MEM 18 */ 0,	/* ERR_NAB 19 */ 0,	/* ERR_NAC 20 */ 0,
	/* ERR_NAE 21 */ 0,	/* ERR_NAP 22 */ 0,	/* ERR_NAQ 23 */ 0,
	/* ERR_NAS 24 */ 0,	/* ERR_NAU 25 */ 0,	/* ERR_NCA 26 */ 0,
	/* ERR_NFI 27 */ 0,	/* ERR_NFO 28 */ 0,	/* ERR_NYA 29 */ 0,
	/* ERR_NYI 30 */ 0,	/* ERR_OFO 31 */ 0,	/* ERR_PDO 32 */ 0,
	/* ERR_PES 33 */ 0,	/* ERR_POP 34 */ 0,	/* ERR_SNI 35 */ 0,
	/* ERR_SRH 36 */ 0,	/* ERR_STL 37 */ 0,	/* ERR_TAG 38 */ 0,
	/* ERR_UTC 39 */ 0,	/* ERR_UTM 40 */ 0,	/* ERR_XAB 41 */ 0,
	/* ERR_YCA 42 */ 0,	/* ERR_IFE 43 */ 0,	/* ERR_SYS 44 */ 0,
	/* ERR_UCD 45 */ 0,	/* ERR_UCI 46 */ 0,	/* ERR_UCO 47 */ 0,
	/* ERR_UFI 48 */ 0,	/* ERR_UFO 49 */ 0,	/* ERR_URC 50 */ 0,
	/* ERR_URE 51 */ 0,	/* ERR_URL 52 */ 0,	/* ERR_UWL 53 */ 0
};
#else
static char *ParaTx[] = {
	/* ERR_ARG 0   improper arguments */
	/*  0*/ "Three arguments are given (a,b,c or H,c).",
	/*  1*/ NULL,
	/* ERR_BNI 1   > not in iteration */
	/*  2*/ "There is a close angle bracket not matched by an open",
	/*  3*/ "angle bracket somewhere to its left. (Note: an",
	/*  4*/ "iteration in a macro stored in a Q-register must be",
	/*  5*/ "complete within the Q-register.)",
	/*  6*/ NULL,
	/* ERR_DTB 2   delete too big */
	/*  7*/ "An nD command has been attempted which is not contained",
	/*  8*/ "within the current page.",
	/*  9*/ NULL,
	/* ERR_FNF 3   file not found "filename" */
	/* 10*/ "The requested input file could not be located. If this",
	/* 11*/ "occurred within a macro, the colon modified ER or EB",
	/* 12*/ "command may be necessary.",
	/* 13*/ NULL,
	/* ERR_ICE 4   illegal ^E command in search argument */
	/* 14*/ "A search argument contains a ^E command that is either",
	/* 15*/ "not defined or incomplete. The only valid ^E commands",
	/* 16*/ "in search arguments are: ^EA, ^ED, ^EV, ^EW, ^EL, ^ES,",
	/* 17*/ "^E<nnn> and ^E[a,b,c...]",
	/* 18*/ NULL,
	/* ERR_IEC 5   illegal character "x" after E */
	/* 19*/ "An invalid E command has been executed. The E character",
	/* 20*/ "must be followed by an alphabetic to form a legal E",
	/* 21*/ "command (i.e. ER or EX).",
	/* 22*/ NULL,
	/* ERR_IFC 6   illegal character "x" after F */
	/* 23*/ "An illegal F command has been executed.",
	/* 24*/ NULL,
	/* ERR_IIA 7   illegal insert argument */
	/* 25*/ "A command of the form \"nItext$\" was attempted. This",
	/* 26*/ "combination of character and text insertion is illegal.",
	/* 27*/ NULL,
	/* ERR_ILL 8   illegal command "x" */
	/* 28*/ "An attempt has been made to execute an invalid TECO",
	/* 29*/ "command.",
	/* 30*/ NULL,
	/* ERR_ILN 9   illegal number */
	/* 31*/ "An 8 or 9 has been entered when the radix of TECO is set",
	/* 32*/ "to octal.",
	/* 33*/ NULL,
	/* ERR_IPA 10   negative or 0 argument to P */
	/* 34*/ "The argument preceding a P or PW command is negative or",
	/* 35*/ "0.",
	/* 36*/ NULL,
	/* ERR_IQC 11   illegal character "x" following " */
	/* 37*/ "One of the valid \" commands did not follow the \". Refer",
	/* 38*/ "to Section 5.14 (conditional execution commands) for",
	/* 39*/ "the legal set of commands.",
	/* 40*/ NULL,
	/* ERR_IQN 12   illegal q-register name "x" */
	/* 41*/ "An illegal Q-register name was specified in one of the",
	/* 42*/ "Q-register commands.",
	/* 43*/ NULL,
	/* ERR_IRA 13   illegal radix argument to ^R */
	/* 44*/ "The argument to a ^R radix command must be 8, 10 or 16.",
	/* 45*/ NULL,
	/* ERR_ISA 14   illegal search argument */
	/* 46*/ "The argument preceding a search command is 0. This",
	/* 47*/ "argument must not be 0.",
	/* 48*/ NULL,
	/* ERR_ISS 15   illegal search string */
	/* 49*/ "One of the search string special characters (^Q, ^V, ^W,",
	/* 50*/ "etc.) would have modified the search string delimiter",
	/* 51*/ "(usually ESCAPE).",
	/* 52*/ NULL,
	/* ERR_IUC 16   illegal character "x" following ^ */
	/* 53*/ "The character following a ^ must have an ASCII value",
	/* 54*/ "between 100 and 137 inclusive or between 141 and 172",
	/* 55*/ "inclusive.",
	/* 56*/ NULL,
	/* ERR_MAP 17   missing ' */
	/* 57*/ "Every conditional (opened with the \" command) must be",
	/* 58*/ "closed with the ' command.",
	/* 59*/ NULL,
	/* ERR_MEM 18   memory overflow */
	/* 60*/ "Insufficient memory available to complete the current",
	/* 61*/ "command. Make sure the Q-register area does not contain",
	/* 62*/ "much unnecessary text. Breaking up the text area into",
	/* 63*/ "multiple pages might be useful.",
	/* 64*/ NULL,
	/* ERR_NAB 19   no argument before ^_ */
	/* 65*/ "The ^_ command must be preceded by either a specific",
	/* 66*/ "numeric argument or a command that returns a numeric",
	/* 67*/ "value.",
	/* 68*/ NULL,
	/* ERR_NAC 20   no argument before , */
	/* 69*/ "A command has been executed in which a , is not preceded",
	/* 70*/ "by a numeric argument.",
	/* 71*/ NULL,
	/* ERR_NAE 21   no argument before = */
	/* 72*/ "The =, ==, or === command must be preceded by either a",
	/* 73*/ "specific numeric argument or a command that returns a",
	/* 74*/ "numeric value.",
	/* 75*/ NULL,
	/* ERR_NAP 22   no argument before ) */
	/* 76*/ "A ) parenthesis has been encountered and is not properly",
	/* 77*/ "preceded by a specific numeric argument or a command",
	/* 78*/ "that returns a numeric value.",
	/* 79*/ NULL,
	/* ERR_NAQ 23   no argument before " */
	/* 80*/ "The \" commands must be preceded by a single numeric",
	/* 81*/ "argument on which the decision to execute the following",
	/* 82*/ "commands or skip to the matching ' is based.",
	/* 83*/ NULL,
	/* ERR_NAS 24   no argument before ; */
	/* 84*/ "The ; command must be preceded by a single numeric",
	/* 85*/ "argument on which the decision to execute the following",
	/* 86*/ "commands or skip to the matching > is based.",
	/* 87*/ NULL,
	/* ERR_NAU 25   no argument before U */
	/* 88*/ "The U comand must be preceded by either a specific",
	/* 89*/ "numeric argument or a command that returns a numeric",
	/* 90*/ "value.",
	/* 91*/ NULL,
	/* ERR_NCA 26   negative argument to , */
	/* 92*/ "A comma was preceded by a negative number.",
	/* 93*/ NULL,
	/* ERR_NFI 27   no file for input */
	/* 94*/ "Before issuing an input command, such as Y, it is",
	/* 95*/ "necessary to open an input file by use of a command such",
	/* 96*/ "as ER or EB.",
	/* 97*/ NULL,
	/* ERR_NFO 28   no file for output */
	/* 98*/ "Before issuing an output command, such as N or P,",
	/* 99*/ "it is necessary to open an output file by use of a",
	/*100*/ "command such as EW or EB.",
	/*101*/ NULL,
	/* ERR_NYA 29   numeric argument with Y */
	/*102*/ "The Y command must not be preceded by either a numeric",
	/*103*/ "argument or a command that returns a numeric value.",
	/*104*/ NULL,
	/* ERR_NYI 30   not yet implemented */
	/*105*/ "A command was issued that is not yet implemented in this",
	/*106*/ "version of TECO.",
	/*107*/ NULL,
	/* ERR_OFO 31   output file already open */
	/*108*/ "A command has been executed which tried to create an",
	/*109*/ "output file, but an output file currently is open. It",
	/*110*/ "is typically appropriate to use the EC or EK command as",
	/*111*/ "the situation calls for to close the output file.",
	/*112*/ NULL,
	/* ERR_PDO 32   push-down list overflow */
	/*113*/ "The command string has become too complex. Simplify it.",
	/*114*/ NULL,
	/* ERR_PES 33   attempt to pop empty stack */
	/*115*/ "A ] command (pop off q-register stack into a q-register)",
	/*116*/ "was encountered when there was nothing on the q-register",
	/*117*/ "stack.",
	/*118*/ NULL,
	/* ERR_POP 34   attempt to move pointer off page with "x" */
	/*119*/ "A J, C or R command has been executed which attempted to",
	/*120*/ "move the pointer off the page. The result of executing",
	/*121*/ "one of these commands must leave the pointer between 0",
	/*122*/ "and Z, inclusive. The characters referenced by a D or",
	/*123*/ "nA command must also be within the buffer boundary.",
	/*124*/ NULL,
	/* ERR_SNI 35   ; not in iteration */
	/*125*/ "A ; command has been executed outside of an open",
	/*126*/ "iteration bracket. This command may only be executed",
	/*127*/ "within iteration brackets.",
	/*128*/ NULL,
	/* ERR_SRH 36   search failure "text" */
	/*129*/ "A search command not preceded by a colon modifier and",
	/*130*/ "not within an iteration has failed to find the specified",
	/*131*/ "\"text\". After an S search fails the pointer is left at",
	/*132*/ "the beginning of the buffer. After an N or _ search",
	/*133*/ "fails the last page of the input file has been input",
	/*134*/ "and, in the case of N, output, and the buffer is",
	/*135*/ "cleared. In the case of an N search it is usually",
	/*136*/ "necessary to close the output file and reopen it for",
	/*137*/ "continued editing.",
	/*138*/ NULL,
	/* ERR_STL 37   string too long */
	/*139*/ "A search or file name string is too long. This is most",
	/*140*/ "likely the result of a missing ESCAPE after the string.",
	/*141*/ NULL,
	/* ERR_TAG 38   missing tag !tag! */
	/*142*/ "The tag !tag! specified by an O command cannot be",
	/*143*/ "found. This tag must be in the same macro level as the",
	/*144*/ "O command referencing it.",
	/*145*/ NULL,
	/* ERR_UTC 39   unterminated command "x" */
	/*146*/ "This is a general error which is usually caused by an",
	/*147*/ "unterminated insert, search, or filename argument, an",
	/*148*/ "unterminated ^A message, an unterminated tag or comment",
	/*149*/ "(i.e., unterminated ! construct), or a missing '",
	/*150*/ "character which closes a conditional execution command.",
	/*151*/ NULL,
	/* ERR_UTM 40   unterminated macro */
	/*152*/ "This error is that same as the ?UTC error except that the",
	/*153*/ "unterminated command was executing from a Q-register",
	/*154*/ "(i.e., it was a macro). (Note: An entire command",
	/*155*/ "sequence stored in a q-register must be complete within",
	/*156*/ "the Q-register.)",
	/*157*/ NULL,
	/* ERR_XAB 41   execution aborted */
	/*158*/ "Execution of TECO was aborted. This is usually due to",
	/*159*/ "the typing of <CTRL/C>.",
	/*160*/ NULL,
	/* ERR_YCA 42   Y command aborted */
	/*161*/ "An attempt has been made to execute a Y or _ search",
	/*162*/ "command with an output file open, that would cause text",
	/*163*/ "in the text buffer to be erased without outputting it to",
	/*164*/ "the output file. The ED command controls this check.",
	/*165*/ NULL,
	/* ERR_IFE 43   ill-formed numeric expression */
	/*166*/ "The numeric expression preceding a command doesn't make",
	/*167*/ "sense.  For example, 5+ isn't a complete expression.",
	/* ERR_SYS 44   %s */
	/*168*/ NULL,
	/* ERR_UCD 45   unable to close and delete output file */
	/*169*/ NULL,
	/* ERR_UCI 46   unable to close input file */
	/*170*/ NULL,
	/* ERR_UCO 47   unable to close output file */
	/*171*/ NULL,
	/* ERR_UFI 48   unable to open file "x" for input */
	/*172*/ NULL,
	/* ERR_UFO 49   unable to open file "x" for output */
	/*173*/ NULL,
	/* ERR_URC 50   unable to read character from terminal */
	/*174*/ NULL,
	/* ERR_URE 51   unable to read TECO command file */
	/*175*/ NULL,
	/* ERR_URL 52   unable to read line from input file */
	/*176*/ NULL,
	/* ERR_UWL 53   unable to write line to output file */
	/*177*/ NULL
};
static int StartP[] = {
	/* ERR_ARG 0  */   0,	/* ERR_BNI 1  */   2,	/* ERR_DTB 2  */   7,
	/* ERR_FNF 3  */  10,	/* ERR_ICE 4  */  14,	/* ERR_IEC 5  */  19,
	/* ERR_IFC 6  */  23,	/* ERR_IIA 7  */  25,	/* ERR_ILL 8  */  28,
	/* ERR_ILN 9  */  31,	/* ERR_IPA 10 */  34,	/* ERR_IQC 11 */  37,
	/* ERR_IQN 12 */  41,	/* ERR_IRA 13 */  44,	/* ERR_ISA 14 */  46,
	/* ERR_ISS 15 */  49,	/* ERR_IUC 16 */  53,	/* ERR_MAP 17 */  57,
	/* ERR_MEM 18 */  60,	/* ERR_NAB 19 */  65,	/* ERR_NAC 20 */  69,
	/* ERR_NAE 21 */  72,	/* ERR_NAP 22 */  76,	/* ERR_NAQ 23 */  80,
	/* ERR_NAS 24 */  84,	/* ERR_NAU 25 */  88,	/* ERR_NCA 26 */  92,
	/* ERR_NFI 27 */  94,	/* ERR_NFO 28 */  98,	/* ERR_NYA 29 */ 102,
	/* ERR_NYI 30 */ 105,	/* ERR_OFO 31 */ 108,	/* ERR_PDO 32 */ 113,
	/* ERR_PES 33 */ 115,	/* ERR_POP 34 */ 119,	/* ERR_SNI 35 */ 125,
	/* ERR_SRH 36 */ 129,	/* ERR_STL 37 */ 139,	/* ERR_TAG 38 */ 142,
	/* ERR_UTC 39 */ 146,	/* ERR_UTM 40 */ 152,	/* ERR_XAB 41 */ 158,
	/* ERR_YCA 42 */ 161,	/* ERR_IFE 43 */ 166,	/* ERR_SYS 44 */ 168,
	/* ERR_UCD 45 */ 169,	/* ERR_UCI 46 */ 170,	/* ERR_UCO 47 */ 171,
	/* ERR_UFI 48 */ 172,	/* ERR_UFO 49 */ 173,	/* ERR_URC 50 */ 174,
	/* ERR_URE 51 */ 175,   /* ERR_URL 52 */ 176,	/* ERR_UWL 53 */ 177
};
#endif
