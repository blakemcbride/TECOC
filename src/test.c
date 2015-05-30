#include <stdio.h>
void main(void) {
	char *names="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

	while (*names) {
		printf("\001Q Register %c \001q%c:=\001 \001:q%c=",
			   *names, *names, *names);
		printf("\001Q Register .%c \001q.%c:=\001 \001:q.%c=",
			   *names, *names, *names);
		names++;
		printf("^T\033\n");
	}
}
