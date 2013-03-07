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

typedef struct task_child {
	pid_t pid;
} task_child_t;

static task_child_t child;

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

	return; /* never return */
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

void parent_sigchld_handler (int signum, siginfo_t *siginfo, void *blank)
{
	if ( signum != SIGCHLD ) {
		return;
	}

	/* handler called in child */
	if ( child.pid == 1 ) {
		return;
	}

	int status = 0;
	pid_t pid = 0;
	while ((pid = waitpid (-1, &status, WNOHANG)) > 0) {
	}
}

void child_work (void)
{
	/* set signal handlers */
	{
		struct sigaction sa;

		memset (&sa, 0, sizeof (sa));
		sa.sa_sigaction = &child_sig_handler;
		sa.sa_flags = SA_SIGINFO;

		sigset_t oset;
		sigprocmask (0, NULL, &oset);
		sa.sa_mask = oset;

		int ret = 0;
		ret = sigaction (SIGILL, &sa, NULL);
		if ( ret == -1 ) { exit (EXIT_FAILURE); }
		ret = sigaction (SIGSEGV, &sa, NULL);
		if ( ret == -1 ) { exit (EXIT_FAILURE); }
		ret = sigaction (SIGBUS, &sa, NULL);
		if ( ret == -1 ) { exit (EXIT_FAILURE); }

	}

	/* allocate memory for code and stack */
	long *const code = mmap (NULL, 1024, PROT_EXEC | PROT_WRITE | PROT_READ, MAP_PRIVATE |  MAP_ANONYMOUS, 0, 0);
	if ( code == MAP_FAILED ) {
		perror ("mmap failed");
		exit (EXIT_FAILURE);
	}
	char *const stack = mmap (NULL, 1024, PROT_EXEC | PROT_WRITE | PROT_READ, MAP_PRIVATE |  MAP_ANONYMOUS, 0, 0);
	if ( stack == MAP_FAILED ) {
		perror ("mmap failed");
		exit (EXIT_FAILURE);
	}

	/* examples */
	/* just exit with exit code 2
	const char *const runcode = "\x48\x31\xc0\xfe\xc0\x48\x31\xdb\xfe\xc3\xfe\xc3\xcd\x80"; */
	/* print Hell and exit
	const char *const runcode = "\x66\x83\xc6\x28\x48\x31\xc0\xfe\xc0\x48\x31\xff\x66\xff\xc7\x48\x31\xd2\x66\x83\xc2\x05\x0f\x05\xb8\x3c\x00\x00\x00\x48\x31\xff\x66\xff\xc7\x66\xff\xc7\x0f\x05\x48\x65\x6c\x6c\x0a";
	*/

	/* read shell code */
	int rt = read (STDIN_FILENO, code, 1024);
	if ( rt < 0 ) {
		printf ("I cant read!\n");
		exit (EXIT_FAILURE);
	}

	const char *const text = "Force be with you!\n";
	
	//memcpy (code, runcode, 45);
	memcpy (stack, text, strlen (text));

	loader (code, stack);

	// never happen?
	munmap (code, 1024);
	munmap (stack, 1024);
}

void run_child (int sock) {

	pid_t pid = fork ();
	child.pid = 0;

	if ( pid == -1 ) {
		perror ("can't fork!");
		return;
	} else if ( pid == 0 ) {
		dup2 (sock, STDOUT_FILENO);
		dup2 (sock, STDIN_FILENO);
		close (sock);
		
		child.pid = 1;
		child_work ();
		exit (EXIT_SUCCESS);
	} else if ( pid > 0 ) {
		close (sock);
		child.pid = 0;
		return;
	}
}

int main (void)
{
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
	setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
	
	if ( bind (sock, (struct sockaddr *)&server , sizeof (server)) < 0) {
		perror ("bind failed. Error");
		close (sock);
		exit (EXIT_FAILURE);
	}

	listen (sock, 100);

	/* set sigchld handler */
	{
		struct sigaction sa;
		memset (&sa, 0, sizeof (sa));
		sa.sa_sigaction = &parent_sigchld_handler;
		sa.sa_flags = SA_SIGINFO;

		sigset_t oset;
		sigprocmask (0, NULL, &oset);
		sa.sa_mask = oset;

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
		printf ("CONN: client: %s\n", inet_ntoa (client.sin_addr));
		run_child (client_sock);
	}

	return EXIT_SUCCESS;
}
