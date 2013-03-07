#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <signal.h>

int allowed_code (char const* const code)
{

}

void loader (long *const code, char *const stack)
{
	long *point = (long *) &point + 3;
	(*point) = (long *) code;

	asm ("mov rsi, %0;"
	     "mov r15, %1;"
		: /* no output */
		: "r" (code), "r" (stack)
		: "%rsi", "%r15"
	);
}

void child_sig_handler (int signum, siginfo_t *siginfo, void *blank)
{
	switch ( signum ) {
		case SIGILL:
		case SIGSEGV:
		case SIGBUS:
			printf ("sorry. your code fail!\n");
			break;
		default:
			printf ("wtf? signum: %d", signum);
			break;
	}
	exit (0);
}

void child_work (void)
{
	struct sigaction sa;
	struct sigaction old_sa;

	memset (&sa, 0, sizeof (sa));
	sa.sa_sigaction = &child_sig_handler;
	sa.sa_flags = SA_SIGINFO;

	sigset_t oset;
	sigprocmask (0, NULL, &oset);
	sa.sa_mask = oset;

	int ret = 0;
	ret = sigaction (SIGILL, &sa, &old_sa);
	if ( ret == -1 ) { exit (EXIT_FAILURE); }
	ret = sigaction (SIGSEGV, &sa, &old_sa);
	if ( ret == -1 ) { exit (EXIT_FAILURE); }
	ret = sigaction (SIGBUS, &sa, &old_sa);
	if ( ret == -1 ) { exit (EXIT_FAILURE); }

	long *const code = mmap (NULL, 1024, PROT_EXEC | PROT_WRITE | PROT_READ, MAP_PRIVATE |  MAP_ANONYMOUS, 0, 0);
	if ( code == MAP_FAILED ) {
		perror ("mmap failed");
		exit (EXIT_FAILURE);
	}
	char *const stack = mmap (NULL, 1024, PROT_EXEC | PROT_WRITE | PROT_READ, MAP_PRIVATE |  MAP_ANONYMOUS, 0, 0);
	if ( code == MAP_FAILED ) {
		perror ("mmap failed");
		exit (EXIT_FAILURE);
	}

	/* just exit with exit code 2 */
	//const char *const runcode = "\x48\x31\xc0\xfe\xc0\x48\x31\xdb\xfe\xc3\xfe\xc3\xcd\x80";
	/* print Hell and exit */
	const char *const runcode = "\x66\x83\xc6\x28\x48\x31\xc0\xfe\xc0\x48\x31\xff\x66\xff\xc7\x48\x31\xd2\x66\x83\xc2\x05\x0f\x05\xb8\x3c\x00\x00\x00\x48\x31\xff\x66\xff\xc7\x66\xff\xc7\x0f\x05\x48\x65\x6c\x6c\x0a";

	const char *const text = "Force be with you!\n";
	
	memcpy (code, runcode, 45);
	memcpy (stack, text, strlen (text));

	loader (code, stack);

	// never happen?
	munmap (code, 1024);
	munmap (stack, 1024);
}

int main (void)
{

	pid_t pid = fork ();

	if ( pid == -1 ) {
		perror ("can't fork!");
		exit (EXIT_FAILURE);
	} else if ( pid == 0 ) {
		child_work ();
		exit (EXIT_SUCCESS);
	} else if ( pid > 0 ) {
		waitpid (pid, NULL, 0); 	
	}

	return EXIT_SUCCESS;
}
