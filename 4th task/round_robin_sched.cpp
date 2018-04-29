#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "proc-common.h"
#include "request.h"

/* Compile-time parameters. */
#define SCHED_TQ_SEC 2 /* time quantum */
#define TASK_NAME_SZ 60 /* maximum size for a task's name */

struct node{
	 int id;
	 pid_t pid;
	 struct node *next;
	 struct node *prev;
	 char *name[];
};
struct node *k;

/*
* SIGALRM handler
*/
static void
sigalrm_handler(int signum) {
	 kill(k->pid,SIGSTOP);
}

/*
* SIGCHLD handler
*/
static void
sigchld_handler(int signum) {
	 pid_t p;
	 int status;
	 for (;;){
		 p=waitpid(-1,&status,WUNTRACED|WNOHANG);
			 if(p<0){
			 perror("waitpid");
			 exit(1);
			}
		if (p==0) break;
 		explain_wait_status(p,status);
		if (WIFEXITED(status)||WIFSIGNALED(status)){
 			struct node *y=k->next;
			k->prev->next=k->next;
			k->next->prev=k->prev;
 			k=y;
 			alarm(SCHED_TQ_SEC);
 			kill(k->pid,SIGCONT);
		}
 		if (WIFSTOPPED(status)){
 			k=k->next;
 			alarm(SCHED_TQ_SEC);
 			kill(k->pid,SIGCONT);
		}
	}
}

/* Install two signal handlers.
* One for SIGCHLD, one for SIGALRM.
* Make sure both signals are masked when one of them is running.
*/

static void
install_signal_handlers(void) {
	 sigset_t sigset;
	 struct sigaction sa;
	 sa.sa_handler = sigchld_handler;
	 sa.sa_flags = SA_RESTART;
	 sigemptyset(&sigset);
	 sigaddset(&sigset, SIGCHLD);
	 sigaddset(&sigset, SIGALRM);
	 sa.sa_mask = sigset;
 	 if (sigaction(SIGCHLD, &sa, NULL) < 0) {
 	 	perror("sigaction: sigchld");
 		exit(1);
 	}
 	 sa.sa_handler = sigalrm_handler;
	 if (sigaction(SIGALRM, &sa, NULL) < 0) {
		 perror("sigaction: sigalrm");
		 exit(1);
 	}

 	/*
	 * Ignore SIGPIPE, so that write()s to pipes
	 * with no reader do not result in us being killed,
	 * and write() returns EPIPE instead.
	 */
	 if (signal(SIGPIPE, SIG_IGN) < 0) {
	 	perror("signal: sigpipe");
	 	exit(1);
	 }
}

int main(int argc, char *argv[])
{
	 pid_t p;
	 int nproc,i;
	 struct node *x;
	 k = (struct node *) malloc(sizeof(struct node));
	 /*
	 * For each of argv[1] to argv[argc - 1],
	 * create a new child process, add it to the process list.
	 */
	 x=k;
	 for(i=1;i<argc;i++){
		 p=fork();
		 //perror
		 if(p==0){
			 char executable[] = "prog";
			 char *newargv[] = { executable, NULL, NULL, NULL };
			 char *newenviron[] = { NULL };
			 raise(SIGSTOP);
			 printf("I am %s, PID = %ld\n",argv[0], (long)getpid());
			 printf("About to replace myself with the executable %s...\n",executable);
			 execve(executable, newargv, newenviron);
			 /* execve() only returns on error */
			 perror("execve");
			 exit(1);
		 }
		 x->id=i;
		 x->pid=p;
		 if(i!=(argc-1)){
		 	x->next= (struct node *) malloc(sizeof(struct node));
		 	x->next->prev=x;
		 	x=x->next;
		 }
	 }
	 x->next=k;
	 x->next->prev=x;
	 nproc = argc-1; /* number of proccesses goes here */
	 /* Wait for all children to raise SIGSTOP before exec()ing. */
	 wait_for_ready_children(nproc);
	
	 /* Install SIGALRM and SIGCHLD handlers. */
	 install_signal_handlers();
	 if (nproc == 0) {
		 fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
		 exit(1);
	 }
	 printf("%d\n",k->pid);
	 kill(k->pid,SIGCONT);
	 alarm(SCHED_TQ_SEC);
	 /* loop forever until we exit from inside a signal handler. */
	 while (pause())
	 		;
	 /* Unreachable */
	 fprintf(stderr, "Internal error: Reached unreachable point\n");
	 return 1;
}


