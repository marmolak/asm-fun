#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include <signal.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include <errno.h>
#include <time.h>
#include <stdarg.h>

#include "castle.h"

static const int loader_len = 8;
static const int area_size = 4096;

void do_log (const char *const fmt, ...)
{
	struct tm w;
	const time_t now = time (NULL);

	va_list ap;
	va_start (ap, fmt);
	const int bsize = vsnprintf (NULL, 0, fmt, ap);
	va_end (ap);

	char msg [bsize];
	va_start (ap, fmt);
	vsnprintf (msg, bsize, fmt, ap);
	va_end (ap);

	if ( localtime_r (&now, &w) == NULL ) {
		memset (&w, 0, sizeof (w));
	}

	w.tm_year += 1900;
	++w.tm_mon;	

	printf ("%d-%.2d-%.2d %.2d:%.2d:%.2d - [%ld]\t%s\n", w.tm_year, w.tm_mon, w.tm_mday, w.tm_hour, w.tm_min, w.tm_sec, now, msg);
}

void loader (long *const code, char *const stack)
{
	alarm (5);
	char *const stack_top = stack + (area_size - 1);
	asm ( "mov rsp, %1;"
              "mov rbp, %1;"
	      "push %0;"
	      "xor rax, rax;"
	      "xor rbx, rbx;"
	      "xor rcx, rcx;"
	      "xor rdx, rdx;"
	      "xor rsi, rsi;"
	      "xor rdi, rdi;"
	      "xor r8, r8;"
	      "xor r9, r9;"
	      "xor r10, r10;"
	      "xor r11, r11;"
	      "xor r12, r12;"
	      "xor r13, r13;"
	      "xor r14, r14;"
	      "xor r15, r15;"
	      "xorps xmm0, xmm0;"
	      "xorps xmm1, xmm1;"
	      "xorps xmm2, xmm2;"
	      "xorps xmm3, xmm3;"
	      "xorps xmm4, xmm4;"
	      "xorps xmm5, xmm5;"
	      "xorps xmm6, xmm6;"
	      "xorps xmm7, xmm7;"
	      "xorps xmm8, xmm8;"
	      "xorps xmm9, xmm9;"
	      "xorps xmm10, xmm10;"
	      "xorps xmm11, xmm11;"
	      "xorps xmm12, xmm12;"
	      "xorps xmm13, xmm13;"
	      "xorps xmm14, xmm14;"
	      "xorps xmm15, xmm15;"
	      "ret;"
		: /* no input */
		: "r" (code), "r" (stack_top)
		: /* nothing */
	);
}

void child_sig_handler (int signum, siginfo_t *siginfo, void *blank)
{
	/* shut up gcc */
	(void) siginfo;
	(void) blank;

	switch ( signum ) {
		case SIGILL:
		case SIGSEGV:
		case SIGBUS:
			printf ("sorry. your code fail!\n");
			break;
		case SIGALRM:
			printf ("sorry, your code run too long\n");
			break;
		default:
			printf ("wtf? signum: %d", signum);
			break;
	}
	exit (0);
}

void parent_sigchld_handler (int signum, siginfo_t *siginfo, void *blank)
{
	/* shut up gcc */
	(void) siginfo;
	(void) blank;

	if ( signum != SIGCHLD ) {
		return;
	}

	int status = 0;
	pid_t pid = 0;
	while ((pid = waitpid (-1, &status, WNOHANG)) > 0) { 
		do_log ("END: [%d] - child exited", pid);
	}
}

void child_work (void)
{
	/* set signal handlers - gdb souces style ;) */
	{
		struct sigaction sa;

		memset (&sa, 0, sizeof (sa));
		sa.sa_sigaction = &child_sig_handler;
		sa.sa_flags = SA_SIGINFO;

		int ret = -1;
		ret = sigaction (SIGILL, &sa, NULL);
		if ( ret == -1 ) { exit (EXIT_FAILURE); }
		ret = sigaction (SIGSEGV, &sa, NULL);
		if ( ret == -1 ) { exit (EXIT_FAILURE); }
		ret = sigaction (SIGBUS, &sa, NULL);
		if ( ret == -1 ) { exit (EXIT_FAILURE); }
		ret = sigaction (SIGFPE, &sa, NULL);
		if ( ret == -1 ) { exit (EXIT_FAILURE); }
		ret = sigaction (SIGALRM, &sa, NULL);
		if ( ret == -1 ) { exit (EXIT_FAILURE); }
	}

	/* allocate memory for code and stack */
	long *const code = mmap (NULL, area_size, PROT_EXEC | PROT_WRITE | PROT_READ, MAP_PRIVATE |  MAP_ANONYMOUS, 0, 0);
	if ( code == MAP_FAILED ) {
		perror ("mmap failed");
		exit (EXIT_FAILURE);
	}
	char *const stack = mmap (NULL, area_size, PROT_EXEC | PROT_WRITE | PROT_READ, MAP_PRIVATE |  MAP_ANONYMOUS, 0, 0);
	if ( stack == MAP_FAILED ) {
		perror ("mmap failed");
		exit (EXIT_FAILURE);
	}

	const int n = read (STDIN_FILENO, code, area_size);
	if ( n < 0 ) {
		perror ("can't read");
		exit (EXIT_FAILURE);
	}

	prepare_castle (code, stack);
	loader (code, stack);

	/* never happen? */
	munmap (code, area_size);
	munmap (stack, area_size);
}

pid_t run_child (int sock) {

	pid_t pid = fork ();

	if ( pid == -1 ) {
		perror ("can't fork!");
		return pid;
	} else if ( pid == 0 ) {
		dup2 (sock, STDOUT_FILENO);
		dup2 (sock, STDIN_FILENO);
		dup2 (sock, STDERR_FILENO);
		close (sock);

		child_work ();

		exit (EXIT_SUCCESS);
	} else if ( pid > 0 ) {
		close (sock);
		return pid;
	}

	return -1;
}



int main (void)
{
#ifdef DEBUG
	/* real debugging */
	child_work ();
	exit (EXIT_SUCCESS);
#endif

	struct sockaddr_in server;

	memset (&server, 0, sizeof (server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons (12345);

	int sock = socket(AF_INET , SOCK_STREAM , 0);
	if ( sock == -1 ) {
		perror ("socket failed");
		exit (EXIT_FAILURE);
	}
	int on = 1;
	setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	
	if ( bind (sock, (struct sockaddr *)&server , sizeof (server)) < 0) {
		perror ("bind failed. Error");
		close (sock);
		exit (EXIT_FAILURE);
	}

	listen (sock, 100);

	/* set sigchld handler - gdb sources way ;) */
	{
		struct sigaction sa;
		memset (&sa, 0, sizeof (sa));
		sa.sa_sigaction = &parent_sigchld_handler;
		sa.sa_flags = SA_SIGINFO;

		int ret = sigaction (SIGCHLD, &sa, NULL);
		if ( ret == -1 ) { exit (EXIT_FAILURE); }
	}

	/* take care about connection */
	socklen_t c = sizeof (struct sockaddr_in);
	for (;;) {
		struct sockaddr_in client;
		memset (&client, 0, sizeof (client));
		int client_sock = accept (sock, (struct sockaddr *) &client, &c);
		if ( client_sock == -1 ) {
			if ( errno == EINTR ) {
				continue;
			}
			perror ("accept error!");
			continue;
		}
		const pid_t pid = run_child (client_sock);
		
		do_log ("CONN: client: [%s] - pid: [%d]\n", inet_ntoa (client.sin_addr), pid);
	}

	return EXIT_SUCCESS;
}
