#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <sys/wait.h>
#include "proc-common.h"
#include "tree.h"

int creator(struct tree_node *q, int fd){
	pid-t p;
	int pfd1[2], pfd[2], value1[2];
	int status;
	char string1[64], string2[64];						//64 characters sufficient for this excercise
	change_pname(q->name);
	if((q->nr_children)>0){
		pipe(pfd1);										
		//if it is not a leaf, create 2 pipes
		if(pipe(pfd1)<0){
			perror("pipe");
			exit(1);
		}
		pipe(pfd2);
		if(pipe(pfd1)<0){
			perror("pipe");
			exit(1);
		}
	}
	if((q->nr_children)>0){								//if not a leaf, reproduce
		p=fork();
		if(p<0){
			perror("fork");
			exit(1);
		}
		if(p==0){										//it's a child
			close(pfd1[0]);								//then close reading to the successor
			creator(q->children,pfd1[1]);				//child has to write, so take write edge
		}												//recursion for the child, it is depth-first
		p=fork();
		if(p<0){
			perror("fork");
			exit(1);
		}
		if(p==0){										//now it is the 2nd child
			close(pfd2[0]);
			creator(q->children+1,pfd2[1]);				//recursion for the other child
		}
		close(pfd1[1]);									//close witing process to the parent
		close(pfd2[1]);
		read(pfd1[0], &string1[0], 10*sizeof(string1[0]));			//reading child's name
		if(read(pfd1[0], &string1[0], 10*sizeof(string1[0]))!=10*sizeof(string1[0])){
			perror("read from pipe");
			exit(1);
		}
		p=wait(&status);								//waiting until death signal
		explain_wait_status(p,status);
		//every node contains a string, so if number(leaf) we must convert to int
		value1[0]=atoi(string1);						//string->int
		value1[1]=atoi(string2);
		//only cases of * or +
		if(q->name[0]=='+'){							//decision and then overwritting
			sprintf(q->name,"%d",value1[0]+value1[1]);
		}
		else{
			sprintf(q->name,"%d",value1[0]*value1[1]);
		}
		write(fd,&q->name,sizeof(int));
		if(write(fd,&q->name,sizeof(int))!sizeof(int)){
			perror("read from pipe");
			exit(1);
		}
		exit(1);
	}
	else{												//leaf
		sleep(2);
		write(fd,&q->name,sizeof(int));					//if leaf write its name(number) to the pipe
		if(write(fd,&q->name,sizeof(int))!=sizeof(int)){
			perror("read from pipe");
			exit(1);
		}
		exit(1);										//leaf dies
	}
	return 0;
}

int main(int argc, char *argv[]){
	struct tree_node *root;
	int status;
	pid_t p,j;
	if(argc!=2){
		fprintf(stderr, "Usage: %s <input_tree_file>\n\n", argv[0]);
		exit(1);
	}
	root=get-tree_from_file(argv[1]);
	p=fork();
	if(p<0){
		perror("fork");
		exit(1);
	}
	if(p==0){
		creator(root,1);
		//reference to standard output
	}
	else{
		sleep(1);									//waiting the end of calculations
		show_pstree(p);
		j=wait(&status);
	}
	printf("\n");
	return 0;
}