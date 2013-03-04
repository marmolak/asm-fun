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

	asm ("mov rsi, %0"
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
	const char *const runcode = "\x66\x83\xc6\x28\x48\x31\xc0\xfe\xc0\x48\x31\xff\x66\xff\xc7\x48\x31\xd2\x66\x83\xc2\x05\x0f\x05\xb8\x3c\x00\x00\x00\x48\x31\xff\x66\xff\xc7\x66\xff\xc7\x0f\x05\x48\x65\x6c\x6c\x0a";


	memcpy (code, runcode, 45);

	loader (code);

	// never happen?
	munmap (code, 1024);

	exit (EXIT_SUCCESS);

	return EXIT_SUCCESS;


}
