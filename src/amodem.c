/*****************************************************************************
	This program provides basic communication with a remote computer.
The remote machine is connected to /dev/tty01.  It is assumed to be a VAX
running VMS.  This program sends whatever is typed at the keyboard to the
remote machine and puts whatever the remote machine sends on the terminal
screen.
	When this program is invoked,  connections to the serial port and
the keyboard are made and the user is left in the communication state.  To
terminate the program,  use control-N,  then period.
	A special feature is the "trapping" of what the modem sends to one
or more files.  Basically,  the program was written to provide a way to
get a large number of files from the VAX to this machine (XENIX).  The idea
is to log in to the VAX and execute a command like "TYPE *.C".  When VAX/VMS
types out files which match the wildcard filename "*.C", it
precedes the text of each file with a header line containing the file name.
This program is sensitive to these headers,  and uses them to name output
files.  Thus,  running this program and typing "TYPE *.C" to VMS on the
remote machine should produce one file in the current directory for each
file typed on the remote machine.
	This program seems to be similar to the CU program provided by UNIX.
When I wrote the first version of this program,  I didn't know about CU.
When I learned of CU,  I had trouble getting it working,  so I just decided
to use this program.
	A feature of setting port characteristics with stty is that the port's
characteristics remain set after the program exits.  This is inconvenient in
the case of the console,  so this program tries to re-set the characteristics
before exiting.
*****************************************************************************/
#include <ctype.h>
#include <sgtty.h>
#include <signal.h>
#include <stdio.h>
#define BOOLEAN char
#define CMPECHO	0777767			/* complement of ECHO in sgtty.h */
#define KEY	"SD0:[PETE.TECO.TECOC]"
#define KEY_LEN	21
#define FALSE	0
#define TRUE	1
char		buffer[100];		/* character buffer */
char		chr;			/* one character */
struct sgttyb	cons_cb;		/* console charactertistics block */
int		cons_fd;		/* console file descriptor */
int		cons_flags_saved;	/* saved value of cons_cb.sg_flags */
BOOLEAN		cons_opened;		/* console need to be closed? */
BOOLEAN		cons_set;		/* console characteristics changed? */
struct sgttyb	modm_cb;		/* modem characteristics block */
int		modm_fd;		/* modem file descriptor */
int		modm_flags_saved;	/* saved value of modm_cb.sg_flags */
char		modm_ispeed_saved;	/* saved value of modm_cb.sg_ispeed */
char		modm_ospeed_saved;	/* saved value of modm_cb.sg_ospeed */
BOOLEAN		modm_opened;		/* does the modem need to be closed? */
BOOLEAN		modm_set;		/* modem characteristics changed? */
int		proc_id;		/* process identification */
BOOLEAN		trapping;		/* trapping modem input to a file? */
FILE		*splice_fp;		/* splice file descriptor */
FILE		*trap_fp;		/* capture file descriptor */
reset_modm_port()
{
	modm_cb.sg_ispeed = modm_ispeed_saved;
	modm_cb.sg_ospeed = modm_ospeed_saved;
	modm_cb.sg_flags = modm_flags_saved;
	printf("Amodem: re-sttying the modem port\n");
	if (stty(modm_fd, &modm_cb) != 0)
		{
		printf("Amodem: couldn't re-stty the modem port\n");
		exit(1);
		}
}
close_modm_port()
{
	printf("Amodem: closing the modem port\n");
	if (close(modm_fd) == -1)
		{
		printf("Amodem: couldn't close the modem port\n");
		exit(1);
		}
}
reset_cons_port()
{
	cons_cb.sg_flags = cons_flags_saved;
	printf("Amodem: re-sttying the console port\r\n");
	if (stty(cons_fd, &cons_cb) != 0)
		{
		printf("Amodem: couldn't re-stty the console port\n");
		exit(1);
		}
}
close_cons_port()
{
	printf("Amodem: closing the console port\r\n");
	if (close(cons_fd) == -1)
		{
		printf("Amodem: couldn't close the console port\n");
		exit(1);
		}
}
cleanup_and_exit()
{
	if (proc_id == 0)
		{
		if (trapping)
			{
			printf("Amodem: closing trap file\r\n");
			if (fclose(trap_fp) == EOF)
				printf("cleanup: unable to close trap file\n");
			}
		}
	else
		{
		if (cons_set)
			reset_cons_port();
		if (cons_opened)
			close_cons_port();
		if (modm_set)
			reset_modm_port();
		if (modm_opened)
			close_modm_port();
		}
	exit(1);
}
open_console()				/* open console port */
{
	cons_fd = open("/dev/tty",2);
	if (cons_fd == -1)
		{
		printf ("Amodem: couldn't open the console port\n");
		cleanup_and_exit();
		}
	cons_opened = TRUE;
	if (gtty(cons_fd, &cons_cb) != 0)
		{
		printf("Amodem: couldn't gtty the console port\n");
		cleanup_and_exit();
		}
	cons_flags_saved = cons_cb.sg_flags;
	cons_cb.sg_flags = RAW;
	if (stty(cons_fd, &cons_cb) != 0)
		{
		printf("Amodem: couldn't stty the console port\n");
		cleanup_and_exit();
		}
	cons_set = TRUE;
}
open_modem()		/* open modem port (serial port A, or /dev/tty01)*/
{
	modm_fd = open("/dev/tty01",2);
	if (modm_fd == -1)
		{
		printf ("Amodem: couldn't open the modem port\n");
		cleanup_and_exit();
		}
	modm_opened = TRUE;
	if (gtty(modm_fd, &modm_cb) != 0)
		{
		printf("Amodem: couldn't gtty the modem port\n");
		cleanup_and_exit();
		}
	modm_ispeed_saved = modm_cb.sg_ispeed;
	modm_ospeed_saved = modm_cb.sg_ospeed;
	modm_cb.sg_ispeed = modm_cb.sg_ospeed = B1200;
	modm_flags_saved = modm_cb.sg_flags;
	modm_cb.sg_flags = RAW;
	if (stty(modm_fd, &modm_cb) != 0)
		{
		printf("Amodem: couldn't stty the modem port\n");
		cleanup_and_exit();
		}
	modm_set = TRUE;
}
cmd_char()
{
#define FBFSIZ 20
#define MAXLINE 132
	int idx;
	char FNamBuf[FBFSIZ];
	char fline[MAXLINE];
	char *line;
	read(cons_fd, &chr, 1);
	switch (chr){
		case '.':
			if (kill(proc_id, SIGTERM) != 0)
				printf("cmd_char: kill failed\n");
			cleanup_and_exit();
			break;
		case '\016':
			break;
		case 's':
		case 'S':
			printf("\r\nname of file to splice into output: ");
			idx = 0;
			while (chr != '\15')
				{
				read(cons_fd, &chr, 1);
				write(cons_fd, &chr, 1);
				if (chr == '\15')
					{
					FNamBuf[idx] = '\0';
					write(cons_fd, "\n", 1);
					break;
					}
				else
					{
					FNamBuf[idx] = chr;
					if (idx++ > FBFSIZ)
						{
					printf("file name too long\r\n");
					chr = '\15';
					return;
						}
					}
				}
			if (access(FNamBuf, 4) != 0)
				{
				printf("that file does not exist\r\n");
				return;
				}
			if ((splice_fp = fopen(FNamBuf, "r")) == NULL)
				{
				printf("\tUnable to open %s\r\n", FNamBuf);
				return;
				}
			line = &fline[0];
			for (;;)
				{
				line = fgets(line, MAXLINE, splice_fp);
				if (line == NULL)
					break;
				write(modm_fd, line, strlen(line) - 1);
				write(modm_fd, "\r", 1);
				}
			if (fclose(splice_fp) == EOF)
				{
				printf("Unable to close %s\r\n", FNamBuf);
				cleanup_and_exit();
				}
			break;
		default:
			printf("\r\n");
			printf("\r\n");
			printf("Control-N options are:");
			printf("\r\n");
			printf("\r\n");
			printf("1.\t. to terminate this program");
			printf("\r\n");
			printf("2.\t^n to send a control-N");
			printf("\r\n");
			printf("3.\ts to splice a file into the output");
			printf("\r\n");
			printf("\r\n");
			break;
		}
}
tst_line()		/* if line contains the key, trap to file */
{
	char *index();
	char *semi;
	char *tmpptr;
	if (strncmp(buffer, KEY, KEY_LEN) == 0)
		{
		if (trapping)
			if (fclose(trap_fp) == EOF)
				{
				printf("Amodem: unable to close trap file\n");
				cleanup_and_exit();
				}
		semi = index(buffer, ';');
		*semi = '\0';
		tmpptr = buffer;
		while (*tmpptr != '\0')
			{
			if (isupper(*tmpptr))
				*tmpptr = tolower(*tmpptr);
			tmpptr++;
			}
		if ((trap_fp = fopen(&buffer[KEY_LEN], "w")) == NULL)
			{
			printf("Unable to create file AMODEM.OUT\r\n");
			cleanup_and_exit();
			}
		trapping = TRUE;
		}
}
parent_send()			/* reads keyboard,  sends to tty01 */
{
	for (;;)
		{
		read(cons_fd, &chr, 1);
		if (chr == '\016')
			cmd_char();
		write(modm_fd, &chr, 1);
		}
}
child_receive()			/* reads tty01,  sends to terminal screen */
{
	char *ptr;
	ptr = &buffer[0];
	for (;;)
		{
		read(modm_fd, &chr, 1);
		write(cons_fd, &chr, 1);
		if ((trapping) && (chr != '\r'))
			putc(chr, trap_fp);
		*ptr++ = chr;
		if (chr == '\n')
			{
			tst_line();
			ptr = &buffer[0];
			}
		}
}
main()
{
	cons_opened = cons_set = modm_opened = modm_set = trapping = FALSE;
	signal(SIGINT, cleanup_and_exit);
	signal(SIGTERM, cleanup_and_exit);
	open_console(),
	open_modem();
	printf("\r\n");
	printf("\r\n");
	printf("Communication established.");
	printf("\r\n");
	printf("\r\n");
	proc_id = fork();
	if (proc_id == -1)
		{
		printf("Amodem: fork failed\n");
		cleanup_and_exit();
		}
	if (proc_id == 0)
		child_receive();
	else
		parent_send();
}
