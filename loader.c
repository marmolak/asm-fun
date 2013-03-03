#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

void loader (long *const code)
{
	long *point = (long *) &point + 2;
	(*point) = (long *) code;
	return;
}

int main (void)
{
	long *const code = mmap (NULL, 1024, PROT_EXEC | PROT_WRITE | PROT_READ, MAP_PRIVATE |  MAP_ANONYMOUS, 0, 0);
	if ( code == MAP_FAILED ) {
		perror ("mmap failed");
		exit (EXIT_FAILURE);
	}

	const pid_t pid = fork ();
	
	if ( pid == -1 ) {
		perror ("fork failed");
		exit (EXIT_FAILURE);
	} else if ( pid == 0 ) {

		const char *const runcode = "\x48\x31\xc0\xfe\xc0\x48\x31\xdb\xfe\xc3\xfe\xc3\xcd\x80";
		memcpy (code, runcode, strlen (runcode));

		loader (code);

		munmap (code, 1024);

		exit (EXIT_SUCCESS);
	}

	int status = 0;
	waitpid (pid, &status, 0);
	printf ("child status: %d\n", WEXITSTATUS (status));

	return EXIT_SUCCESS;
}
