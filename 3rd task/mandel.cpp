/* A programm to draw the Mandelbrot Set on a 256-color xterm. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <semaphore.h>

#include "mandel-lib.h"
#define MANDEL_MAX_ITERATION 100000

#define perror_pthread(ret,msg)\
	do{ errno = ret; perror(msg);} while(0)

/*
 * Output at the terminal is x_chars wide by y_chars long
 */

int y_chars = 50;
int x_chars = 90;

/*
 * The part of the complex plane to be drawn:
 * upper left corner is (xmin,ymax), lower right corner is (xmax, ymin)
 */

double xmin=-1.8, xmax=1.0
double ymin=-1.0, ymax=1.0;

sem_t *mutex;

typedef struct name{
	int x;
	int y;
} data;

/*
 * Every character in the final output is
 * xstep x ystep units wide on the complex plane.
 */

double xstep;
double ystep;

/*
 * This function computes a line of output
 * as an array of x_char color values.
 */

void compute_mandel_line(int line, int color_val[]){

	/*
	 * x and y traverse the complex plane
	 */

	double x,y;
	int n;
	int val;

	//Find out the y value corresponding to this line
	y = ymax-ystep*line;
	//and iterate for all points on this line

	for(x=xmin,n=0;n<x_chars;x+=xstep,n++){
		//Compute the point's color value
		val=mandel_iterations_at_point(x,y,MANDEL_MAX_ITERATION);
		if(val>255)
			val=255;

		//And store it in the color_val[] array
		val=xterm_color(val);
		color_val[n]=val;
	}
}

/*
 * This function outputs an array of x_char color values
 * to a 256-color xterm.
 */

void output_mandel_line(int fd, int color_val[]){
	int i;
	char point='@';
	char newline='\n';
	for(i=0;i<x_chars;i++){
		//Set the current color, then output the point
		set_xterm_color(fd,color_val[i]);
		if(write(fd,&point,1)!=1){
			perror("compute_and_output_mandel_line:write point");
			exit(1);
		}
	}
	//Now that the line is done, output a newline character
	if(write(fd,&newline,1)!=1){
		perror("compute_and_output_mandel_line:write newline");
		exit(1);
	}
}

void *compute_and_output_mandel_line(void *arg){
	//A temporary array, used to hold color values for the line being drawn
	int color_val[x_chars],p;
	data *line=arg;								//new data pointer struct
	int myline=line->x;							//pointer to the number of the current thread
	int step=line->y; 							//pointer to the total number of threads
	p=line->x;
	while(myline<y_chars){ 						//while we have lines
		compute_mandel_line(myline,color_val);	//compute current line
		sem_wait(&mutex[p]); 					//print lines one by one, so only one thread should pass
		output_mandel_line(1, color_val); 		//print one line
		if(p!=step-1) sem_post(&mutex[p+1]);	//if current thread is not the last, activate its next
		else sem_post(&mutex[0]); 				//else, activate the first
		myline+=step; 							//finally, proceed for the computations (N, as the threads)
	}
	return NULL;
}

int main(int argc,char *argv[]){
	int i,ret;
	int N=atoi(argv[1]);
	data matrix[N];
	mutex=(sem_t*)malloc(N*sizeof(sem_t));
	//Dynamic creation of semaphore array of size N

	for(i=1;i<N;i++){
		//initialization
		sem_init(&mutex[i],0,0);
		matrix[i].x=i;							//current thread
		matrix[i].y=N;							//number of threads
	}
	matrix[0].x=0;
	matrix[0].y=N;
	sem_init(&mutex[i],0,1);
	if(sem_init(&mutex[0],0,1)==-1) perror("fail to initialize");
	pthread_t t[N];
	xstep=(xmax-xmin)/x_chars;
	ystep=(ymax-ymin)/y_chars;

	/*
 * Draw the Mandelbrot Set, one line at the time.
 * Output is sent to file descriptor '1', i.e., standard output
 */

	for(i=0;i<N;i++){
		//Creation of N threads
		//parameters: current thread and N (included in the matrix structure)
		ret=(pthread_create(&t[i],NULL,compute_and_output_mandel_line,&matrix[i]));
		if(ret){
			perror_pthread(ret,"pthread_create");
			exit(1);
		}
	}
	for(i=0;i<N;i++){
		ret=pthread_join(t[i], NULL);
		//free used semaphores
		if(ret) perror_pthread(ret,"pthread_join");
		ret=sem_destroy(&mutex[i]);
		if(ret==-1)perror_pthread(ret,"pthread_destroy");
	}
	reset_xterm_color(1);
	return 0;
}














