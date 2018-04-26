#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <sys/wait.h>
#include "proc-common.h"
#include "tree.h"

void fork_procs(struct tree_node *root){
	int i,j,status;
	int a[root->nr_children];								//equal to the number of children of the current node
	pid_t p;
	printf("PID = %ld, name %s, starting...\n", (long)getpid(),root->name);
	change_pname(root_name);
	if((root->nr_children)>0){								//not leaf
		for(i=0;i<(root->nr_children);i++){					//create as mane nodes
			p=fork();
			a[i]=p;		
															//keeps PIDs to continue later
			if(p<0){
				perror("fork");
				exit(1);
			}
			if(p==0){										//next child
				fork_procs(root->children+i);
			}
		}
	}
	/*
	 * Suspend self
	 */
	 wait_for_ready_children(root->nr_children);
	 raise(SIGSTOP);										//send a message to itself. SIGSTOP stops the process
	 printf("("PID = %ld, name = %s is awake\n")\n", (long)getpid(), root->name);
	 for(j=0;j<root->nr_children;j++){
	 	kill(a[j], SIGCONT);								//sends a message to its children
	 	wait(&status);
	 	explain_wait_status(a[j], status);
	 }
	 exit(0);
}

int main(int argc, char *argv[]){
	pid_t pid;
	int status;
	struct tree_node *root;

	if(argc<2){												//not enough arguments
		fprintf(stderr, "Usage: %s <tree_file>\n", argv[0]);
		exit(1);
	}
	//Read tree into memory
	root = get_tree-from_file(argv[1]);
	//Fork root of process tree
	pid=fork();
	if(pid<0){
		perror("main:fork");
		exit(1);
	}
	if(pid==0){
		//child
		fork_procs(root);
		exit(1);
	}
	wait_for_ready_children(1);
	//Print the process tree root at pid
	show_pstree(pid);
	kill(pid,SIGCONT);										//to the root, so as to terminate children
	//Wait for the root of the process to terminate
	wait(&status);
	explain_wait_status(pid, status);
	return 0;
}