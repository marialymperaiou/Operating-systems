#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

/*
 * POSIX thread functions do not return error numbers in errno, 
 * but in the actual return value of the function call instead.
 */
 #define perror_pthread(ret,msg)\
 	do { errno = ret; perror(msg);} while(0)
 #define N 10000000

 pthread_mutex_t mutex;								//declare mutex

 #if defined(SYNC_ATOMIC)^defined(SYNC_MUTEX)==0
 #error You must #define exactly one of SYNC_ATOMIC or SYNC_MUTEX.
 #endif

 #if defined(SYNC_ATOMIC)
 # define USE_ATOMIC_OPS 1
 #else
 # define USE_ATOMIC_OPS 0
 #endif

 void *increase_fn(void *arg){
 	int i;
 	volatile int *ip=arg;
 	fprintf(stderr, "About to increase variable %d times \n", N);
 	for (i=0;i<N;i++){
 		if(USE_ATOMIC_OPS){
 			__sync_add_and_fetch(&*ip,1);
 		}
 		else{
 			//one thread is used to lock a mutex. If it is already locked, the current thread is suspended
 			pthread_mutex_lock(&mutex);
 			++(*ip)
 			//it is unclocked from the thread that occupies the mutex
 			pthread_mutex_unlock(&mutex);
 		}
 	}
 	fprintf(stderr, "Done increasing variable.\n");
 	return NULL;
 }

 void *decrease_fn(void *arg){
 	int i;
 	volatile int *ip=arg;
 	fprintf(stderr, "About to decrease variable %d times\n", N);
 	for(i=0;i<N;i++){
 		if(USE_ATOMIC_OPS){
 			__sync_sub_and_fetch(&*ip,1);
 		}
 		else{
 			pthread_mutex_lock(&mutex);
 			--(*ip);
 			pthread_mutex_unlock(&mutex);
 			//with lock and unlock ++ and -- aren't executed in the same time
 		}
 	}
 	fprintf(stderr, "Done decreasing variable.\n");
 }

 int main(int argc, char *argv[]){
 	int val, ret, ok;
 	pthread_t t1,t2;
 	//initial value
 	val = 0;								//check variable

 	/* Creation of a new executable thread: id of the new thread, object of predefined capacity,
 	 * thread execution routine, address of thread parameters
 	 */

 	 ret = pthread_create(&t1, NULL, increase_fn, &val);
 	 if (ret){
 	 	perror_pthread(ret,"pthread_create");
 	 	exit(1);
 	 }
 	 //creation of a new executable thread
 	 ret=pthread_create(&t2, NULL, decrease_fn,&val);
 	 if (ret){
 	 	perror_pthread(ret,"pthread_create");
 	 	exit(1);
 	}
 	/*
 	 * Wait for threads to terminate. The coordinating thread is suspended until the other terminate.
 	 */
 	 ret = pthread_join(t1,NULL);
 	 if(ret) perror_pthread(ret,"pthread_join");
 	 ok=(val==0);

 	 //Finally val should be 0, because it increases as much it decreases.
 	 //then ok=1, and the threads are synchronised
 	 printf(" OK, val = %d.\n", ok?"": "NOT", val);
 	 return ok;
 }