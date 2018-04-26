#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <sys/wait.h>
#include "proc-common.h"
#include "tree.h"

int creator(struct tree_node *q){			//create the tree recursively
	pid_t p;
	int i,c;
	int status;
	c=q->nr_children;						//keep number of children
	if((q->nr_children)>0){
		for(i=0,i<(q->nr_children);i++){	//do for every child
			p=fork();
			if(p==0){
				creator(q->children+i);		//i helps tracking in which child we are
			}
			else{
				change_pname(q->name);
			}
		}
		while(c>0){							//for every child wait until it's dead
			p=wait(&status);
			explain_wait_status(p,status);
			c--;
		}
		exit(1);							//every child is dead, so the parent dies too
	}
	else{
		change_pname(q->name);
		printf("leaf %s is sleeping \n", q->name);
		sleep(10);							//wait 10 sec, until then everything will be finished
		exit(1);
	}
	return 0;
}

int main(int argc, char *argv[]){
	struct tree_node *root;
	int status;
	pid_t p,j;

	if (argc!=2){							//not the right number of arguments!
	fprintf(stderr, "Uasge:%s <input_tree_file>\n\n", argv[0]);	
	exit(1);
	}
	root=ger_tree_from_file(argv[1]);		//read tree from memory
	//return an index in the beginning of the tree
	p=fork();
	if(p<0){
		//fork failed
		perror("fork");
		exit(1);
	}
	if(p==0){								//first node
		creator(root);
	}
	else{
		sleep(1);							//wait 1 sec, the tree should be ready until then
		show_pstree(p);						//and then print the tree
		j=wait(&status);
		explain_wait_status(j,status);
	}
	return 0;
}


/*
 * First higher-level nodes and then lower-level. Order of signals: CDEF
 * Leaves die first and then the parent dies
 */