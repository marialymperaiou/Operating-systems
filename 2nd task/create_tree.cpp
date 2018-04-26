/*
 * fork-example.c
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <sys/wait.h>
#include "proc-common.h"

#define SLEEP_SEC 10

int main(void){
	//parent process
	pid_t p;
	int status;
	fprintf(stderr, "parent, PID = %ld: Creating child...\n", (long)getpid());
	p = fork();										//reproduction

	if(p<0){
		//fork failed
		perror("fork");
		exit(1);
	}
	//create children to make the tree
	if(p==0){										//is child
		//in A
		p = fork();
		if(p<0){
			//fork failed
			perror("fork");
			exit(1);
		}
		if(p==0){
			//in C
			change_pname("C");
			sleep(SLEEP_SEC);
			exit(17);
		}
		change_pname("A");
		p=fork();
		if(p<0){
			perror("fork");
			exit(1);
		}
		//A needs to create B and C(leaf)
		if(p==0){
			//in B
			change_pname("B");
			p=fork();
			//B nees to create D(leaf)
			if(p==0){
				//in D
				change_pname("D");
				sleep(SLEEP_SEC);
				exit(13);
			}
			p=wait(&status);						//wait for children to die
			explain_wait_status(p,status);			//message why children died
			exit(19);
		}
		p=wait(&status);
		explain_wait_status(p,status);
		p=wait(&status);
		explain_wait_status(p,status);
		exit(16);
	}
	change_pname(".c");
	/* In parent process. Wait for the child to terminate 
	 * and report its termination status.
	 */
	printf("Parent, PID = %ld: Created child with PID = %ld, waiting for it to terminate...\n",(long)getpid(),(long)p);
	sleep(1);										//sleep so that the tree can be printed. This value was chosen after trials.
	show_pstree(p);
	p=wait(&status);
	explain_wait_status(p,status);
	//show_pstree(p);
	printf("Parent: All done, exiting...\n");
	return 0;
}

























