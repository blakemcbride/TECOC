
#include <sys/types.h>
#include <stdio.h>
#ifdef BSD43
#include <sys/dir.h>
#else
#include <dirent.h>
#endif
/* POSIX specifies sys/dirent.h - could add another define ---*/
#include <string.h>
/* usage: purge [-n] files... */
int plevel;
main(argc,argv) 
int argc;
char *argv[];
{
	char *q;	
	int fileindex;
	fileindex=1;
	if (argc==1)	{
		fprintf(stderr,"usage: purge [-n] files...\n");
		exit(1);
	}
	plevel = 3;
	if (*argv[1]=='-') {
		q=argv[1]+1;
		plevel = atoi(q);
		fileindex++;
	}
	if (plevel < 1) {
		fprintf(stderr,"usage: purge [-n] files...\n");
		exit(1);
	}
	while(fileindex < argc) {
		if (purge(argv[fileindex],plevel)<0) {
			fprintf(stderr,"error purging %s\n",argv[fileindex]);
		}
		fileindex++;
	}
}
int purge(target,plevel)
char *target;
int plevel;
{
/* 
maximum version number of target (similar to VMS mechanism)
return
-3	error - problems other than file not found and can't open directory 
-2	error - did not find file
-1	error - cannot open directory containing target 
 0	file found - no version numbers found 
 n > 0	version number of highest name;n
*/
	DIR            *dirp;
	int found = 0;	/* file found flag */
	char *ftarget;
	int maxver = 0;
	char *dirname;
#ifdef BSD43
	struct direct  *dp;
#else
	struct dirent  *dp;
#endif
	int n;
	char *s;
	n = strlen(target);
	dirname=(char *)malloc(strlen(target)+4);
	ftarget=(char *)malloc(strlen(target)+4);
	strcpy(dirname, target);
	s=dirname + n;
	while (*s != '/' && s > dirname)
		s--; 		
	if (*s=='/' && s==dirname) { /* must be root directory */
		strcpy(ftarget,s+1);
		*(s+1)='\0';  
	} else if (*s=='/')  { /* we have string/name */
		strcpy(ftarget,s+1);
		*s='\0';
	} else { /* must have s==dirname and *s!='/', so current directory */
		strcpy(ftarget,target);
		*dirname='.'; *(dirname+1)='\0';	
	}
	dirp = opendir(dirname);
	if (dirp == NULL) {
		fprintf(stderr,"\nerror openning directory %s\n",dirname);
		free(dirname);
		free(ftarget);
		return(-1);
	}
	n=strlen(ftarget);
	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		if (strncmp(dp->d_name,ftarget,n) == 0) {
			char *suffix = dp->d_name + n;
			if (*suffix == '\0') {
				found = 1;
				if (maxver <= 0)
					maxver = 0;
			}
			if (*suffix == ';') { /* assume no other ; in files */
				int k;
				found = 1;
				k=atoi(suffix+1);
				if (k<=0) {
					free(dirname);
					free(ftarget);
					closedir(dirp);
					return(-3);
				}
				if (k>=maxver)
					maxver = k;
			}
		}
	}
	closedir(dirp);
	if (!found) 
		return(-2);
	if (maxver==0)
		return(1);
	dirp = opendir(dirname);
	if (dirp == NULL) {
		fprintf(stderr,"\nerror openning directory %s\n",dirname);
		free(dirname);
		free(ftarget);
		return(-1);
	}
	n=strlen(ftarget);
	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		if (strncmp(dp->d_name,ftarget,n) == 0) {
			char *suffix = dp->d_name + n;
			if (*suffix == ';') {
				int k;
				k=atoi(suffix+1);
				if (k<=0) {
					free(dirname);
					free(ftarget);
					closedir(dirp);
					return(-3);
				}
				if (k <= maxver - plevel + 1) {
					char *st;
					st=dp->d_name;
					fprintf(stderr,"unlink %s -- ",st);
					if(unlink(st)==(-1))
						perror("failed : ");
					else
						fprintf(stderr,"succeeded");
					fprintf(stderr,"\n");
				}
			}
		}
	}
	closedir(dirp);
	return(1);
}
