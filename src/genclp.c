/***************************************************************************
 * This program reads a TECO macro contained in file CLPARS.TEC.  It writes
 * CLPARS.H,  a C include file that is included by the ZPrsCL function.
***************************************************************************/
#ifdef sun
void	exit();			/* exit the program */
int	puts();			/* write string to stdout, with newline */
void	perror();		/* write a message on stdout */
int	fputs();		/* put string to a given stream */
int	fclose();		/* close a stream */
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#else
#include <stdlib.h>		/* define prototype for exit() */
#ifdef vax11c
#define EXIT_SUCCESS 1
#define EXIT_FAILURE 2
#endif
#endif
#include <stdio.h>		/* define prototypes for fopen(), etc. */
#include <string.h>		/* define prototype for strcpy */
#ifndef FALSE
#define FALSE (1==0)
#endif
#ifndef TRUE
#define TRUE (1==1)
#endif
typedef char BOOLEAN;
#define ILINE_SIZE 132	/* maximum input file (CLPARS.TEC) line length */
#define OLINE_SIZE 75	/* maximum output file (CLPARS.H) line length */
static void
#if USE_PROTOTYPES
open_clpars_tec(FILE **ifile)
#else
open_clpars_tec(ifile)
FILE **ifile;
#endif
{
#if defined(MSDOS)  ||  defined(WIN32)
	*ifile = fopen("win\\clpars.tec","rt");
#else
	*ifile = fopen("clpars.tec","r");
#endif
	if (*ifile == NULL)
		{
		puts("genclp: Unable to open file CLPARS.TEC for reading");
		perror("genclp");
		exit(EXIT_FAILURE);
		}
}
static void
#if USE_PROTOTYPES
open_clpars_h(FILE **ofile)
#else
open_clpars_h(ofile)
FILE **ofile;
#endif
{
#if defined(MSDOS)  ||  defined(WIN32)
	*ofile = fopen("clpars.h","wt");
#else
	*ofile = fopen("clpars.h","w");
#endif
	if (*ofile == NULL)
		{
		puts("genclp: Unable to open file CLPARS.C for writing");
		perror("genclp");
		exit(EXIT_FAILURE);
		}
}
static void
#if USE_PROTOTYPES
write_line(char *obuf, FILE *ofile)
#else
write_line(obuf, ofile)
char *obuf;
FILE *ofile;
#endif
{
	if (fputs(obuf, ofile) == EOF)
		{
		puts("genclp: Unable to write to file CLPARS.C");
		perror("genclp");
		exit(EXIT_FAILURE);
		}
}
static void
#if USE_PROTOTYPES
write_header(FILE *ofile)
#else
write_header(ofile)
FILE *ofile;
#endif
{
	write_line("/*\n", ofile);
	write_line(" * This file was created by the GENCLP program.\n",ofile);
	write_line(" * It should not be edited.  To make changes,\n", ofile);
	write_line(" * change the CLPARS.TES file,  then squeeze\n", ofile);
	write_line(" * CLPARS.TES to produce the CLPARS.TEC file,\n", ofile);
	write_line(" * then run GENCLP to produce this file.\n", ofile);
	write_line(" */\n", ofile);
}
static void
#if USE_PROTOTYPES
close_clpars_tec(FILE *ifile)
#else
close_clpars_tec(ifile)
FILE *ifile;
#endif
{
	if (fclose(ifile) != 0)
		{
		puts("genclp: Unable to close input file CLPARS.C");
		perror("genclp");
		exit(EXIT_FAILURE);
		}
}
static void
#if USE_PROTOTYPES
close_clpars_h(FILE *ofile)
#else
close_clpars_h(ofile)
FILE *ofile;
#endif
{
	if (fclose(ofile) != 0)
		{
		puts("genclp: Unable to close output file CLPARS.C");
		perror("genclp");
		exit(EXIT_FAILURE);
		}
}
static void
#if USE_PROTOTYPES
cnvrt(	FILE *ifile,
	FILE *ofile,
	BOOLEAN ANSI_style)
#else
cnvrt(ifile, ofile, ANSI_style)
FILE *ifile;
FILE *ofile;
BOOLEAN ANSI_style;
#endif
{
	char *iptr;
	char *optr;
	int total_chars = 0;
	int total_lines = 0;
	char iline[ILINE_SIZE];
	char oline[OLINE_SIZE];
	oline[0] = '\"';
	optr = &oline[1];
	while (fgets(iline, ILINE_SIZE, ifile) != NULL)
		{
		iptr = iline;
		while (*iptr != '\n')
			{
			switch (*iptr) {
			case '"':  *optr++ = '\\'; *optr++ = '"'; break;
			case '\\': *optr++ = '\\'; *optr++ = '\\'; break;
			case '\n': *optr++ = '\\'; *optr++ = 'n'; break;
			case '\r': *optr++ = '\\'; *optr++ = 'r'; break;
			default:
			    if (*iptr >= ' ')
				*optr++ = *iptr;
			    else {
				sprintf(optr,
					(ANSI_style) ? "\\%o\"\"" : "\\%03o",
					*iptr);
				optr += strlen(optr);
			      }
			}
			iptr++;
			total_chars += 1;
			if (optr > &oline[OLINE_SIZE-10])
				{
				*optr++ = '"';
				if (!ANSI_style)
					*optr++ = ',';
				*optr++ = '\n';
				*optr++ = '\0';
				write_line(oline, ofile);
				total_lines += 1;
				optr = &oline[1];
				}
			}
		}
	*optr++ = '"';
	*optr++ = '\n';
	*optr   = '\0';
	write_line(oline, ofile);
	total_lines += 1;
	write_line("};\n", ofile);
	sprintf(oline, "#define CLPARS_LEN %d\n", total_chars);
	write_line(oline, ofile);
	if (!ANSI_style)
		{
		sprintf(oline, "#define CLPARS_LINES %-3d\n", total_lines);
		write_line(oline, ofile);
		}
}
int
#if USE_PROTOTYPES
main(void)
#else
main()
#endif
{
	FILE *ifile, *ofile;
	open_clpars_h(&ofile);
/*
 * write "#if USE_ANSI_CLPARS" part
 */
	open_clpars_tec(&ifile);
	write_header(ofile);
	write_line("#if USE_ANSI_CLPARS\n", ofile);
	write_line("unsigned char clpars[] = {\n", ofile);
	cnvrt(ifile, ofile, TRUE);
	close_clpars_tec(ifile);
/*
 * write "#else" part
 */
	open_clpars_tec(&ifile);
	write_line("#else\n", ofile);
	write_line("char *clpars[] = {\n", ofile);
	cnvrt(ifile, ofile, FALSE);
	close_clpars_tec(ifile);
	write_line("#endif\n", ofile);
	close_clpars_h(ofile);
	return EXIT_SUCCESS;
}
