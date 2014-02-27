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

#ifdef SECCOMP
#include <seccomp.h>
#endif

#include <sys/ptrace.h>
#include <sys/syscall.h> 
#include <sys/reg.h> 
#include <sys/user.h>

#include "elf-gen.h"
#include "castle.h"

static const int area_size = 4096;

void loader (char *const code, char *const stack)
{
	alarm (7);
    char bin[17] = { 0 };

    elf_gen (code, bin);
    execl (bin, bin, NULL);
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
	while ((pid = waitpid (-1, &status, WNOHANG)) > 0) { }
}

#ifdef SECCOMP
void set_sandbox (void)
{
	/* init_sandbox */
	/* seccomp */
	scmp_filter_ctx sfcx = seccomp_init (SCMP_ACT_TRAP);
	if ( sfcx == NULL ) {
		perror ("Seccomp failed!");
		return;
	}

	int ret = -1;

	/* for signal handlers :( */
	ret = seccomp_rule_add (sfcx, SCMP_ACT_ALLOW, SCMP_SYS(mmap), 0);
	if ( ret < 0 ) {
		errno = -ret;
		perror ("Seccomp add rule failed");
		return;
	}
	ret = seccomp_rule_add (sfcx, SCMP_ACT_ALLOW, SCMP_SYS(fstat), 0);
	if ( ret < 0 ) {
		errno = -ret;
		perror ("Seccomp add rule failed");
		return;
	}
	ret = seccomp_rule_add (sfcx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
	if ( ret < 0 ) {
		errno = -ret;
		perror ("Seccomp add rule failed");
		return;
	}
	/* end of signal handler :( */

	ret = seccomp_rule_add (sfcx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
	if ( ret < 0 ) {
		errno = -ret;
		perror ("Seccomp add rule failed");
		return;
	}

	ret = seccomp_rule_add (sfcx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
	if ( ret < 0 ) {
		errno = -ret;
		perror ("Seccomp add rule failed");
		return;
	}

	ret = seccomp_rule_add (sfcx, SCMP_ACT_ALLOW, SCMP_SYS(munmap), 0);
	if ( ret < 0 ) {
		errno = -ret;
		perror ("Seccomp add rule failed");
		return;
	}

	ret = seccomp_load (sfcx);
	if ( ret < 0 ) {
		errno = -ret;
		perror ("Seccomp load filter failed");
		return;
	}
	seccomp_release (sfcx);
	/* end sandbox init */
}
#endif

void child_work (void)
{


#ifdef SECCOMP
	set_sandbox ();
#endif

	const pid_t pid = fork ();
	if ( pid == 0 ) {

        char code [area_size];
        memset (code, '\x90', area_size);
		const int n = read (STDIN_FILENO, code, area_size);
		if ( n < 0 ) {
			perror ("can't read");
			exit (EXIT_FAILURE);
		}

        char stack [area_size];
		prepare_castle (code, stack);

        loader (code, stack);
        // never reached
		exit (EXIT_SUCCESS);
    }

	int status = 0;
	while ( waitpid (pid, &status, 0) > 0 ) {
		if ( WIFEXITED (status) ) { break; }

		/* take care about children's SIGALRM */
		int signo = 0;
		if ( WIFSTOPPED (status) == 1 ) {
			switch (WSTOPSIG (status)) {
				case SIGALRM:
					signo = SIGALRM;
					break;
				default:
					signo = WSTOPSIG (status);
					printf ("Child died with signal %d\n", signo);
					break;
			}
			/* set signo and continu with ptrace (PTRACE_CONT, ... */
			goto ptrace_cont;
		}

		const unsigned long orig_rax = ptrace (PTRACE_PEEKUSER, pid, 8 * ORIG_RAX, NULL);
		if ( orig_rax == SYS_execve ) {
			printf ("syscall denied!\n");
			kill (pid, SIGKILL);
		}

		ptrace_cont:
			ptrace (PTRACE_CONT, pid, NULL, signo);
	}
}

void run_child (int sock) {

	pid_t pid = fork ();

	if ( pid == -1 ) {
		perror ("can't fork!");
		return;
	} else if ( pid == 0 ) {
		dup2 (sock, STDOUT_FILENO);
		dup2 (sock, STDIN_FILENO);
		dup2 (sock, STDERR_FILENO);
		close (sock);

		child_work ();

		exit (EXIT_SUCCESS);
	} else if ( pid > 0 ) {
		close (sock);
		return;
	}
}

int main (void)
{
#ifdef DEBUG
	/* real debugging */
	child_work ();
	return EXIT_SUCCESS;
#endif

	struct sockaddr_in server;

	memset (&server, 0, sizeof (server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons (12345);

	int sock = socket(AF_INET , SOCK_STREAM , 0);
	if ( sock == -1 ) {
		perror ("socket failed");
		return EXIT_FAILURE;
	}
	int on = 1;
	setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	
	if ( bind (sock, (struct sockaddr *)&server , sizeof (server)) < 0) {
		perror ("bind failed. Error");
		close (sock);
		return EXIT_FAILURE;
	}

	if ( listen (sock, 100) == -1 ) {
        perror ("listen failed.");
        close (sock);
        return EXIT_FAILURE;
    }

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
            return EXIT_FAILURE;
		}
		printf ("CONN: client: %s\n", inet_ntoa (client.sin_addr));
		run_child (client_sock);
	}

	return EXIT_SUCCESS;
}
