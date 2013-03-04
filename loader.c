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

	asm ("mov rcx, %0"
		: /* no output */
		: "r" (code)
		: 
	);
}

int main (void)
{

	long *const code = mmap (NULL, 1024, PROT_EXEC | PROT_WRITE | PROT_READ, MAP_PRIVATE |  MAP_ANONYMOUS, 0, 0);
	if ( code == MAP_FAILED ) {
		perror ("mmap failed");
		exit (EXIT_FAILURE);
	}

	/* just exit with exit code 2 */
	//const char *const runcode = "\x48\x31\xc0\xfe\xc0\x48\x31\xdb\xfe\xc3\xfe\xc3\xcd\x80";
	/* print Hell and exit */
	const char *const runcode = "\x66\x81\xc1\x25\x00\x48\x31\xc0\x04\x04\x48\x31\xdb\xfe\xc3\x48\x31\xd2\x80\xc2\x05\xcd\x80\x48\x31\xc0\xfe\xc0\x48\x31\xdb\xfe\xc3\xfe\xc3\xcd\x80Hell\x0a";
	memcpy (code, runcode, 43);

	loader (code);

	// never happen?
	munmap (code, 1024);

	exit (EXIT_SUCCESS);

	return EXIT_SUCCESS;


}
