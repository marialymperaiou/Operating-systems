#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define OUTPUT "fconc.out"
#define USAGE "Usage: ./fconc infile1 infile2 [outfile (default:fconc.out)]\n"

void doWrite(int fd, const char *buff, int len);
//function call with arguments: file descriptor, char array in which the data are stored, and the number of transmitted data
void write_file(int fd, const char *infile);

int main(int argc, char *argv[]){			//number of arguments and their array
	char *output = OUTPUT;
	if (argc<3){
		printf(USAGE);
		return 0;
	}
	else if (argc>3){						//acceptable number of arguments
		output = argv[3];
	}
	int fdwr, oflags, mode;
	oflags = O_CREAT | O_WRONLY | O_TRUNC;	//create file, write and truncate
	mode = S_IRUSR | S_IWUSR;				//permit read-write
	fdwr = open(output, oflags, mode);
	if(fdwr<0){								//error!
		perror("open");
		exit(1);
	}
	//write at fdwr
	write_file(fdwr, argv[1]);				//1st argument
	write_file(fdwr, argv[2]);
	close(fdwr);
	return 0;
}

void doWrite(int fd, const char *buff, int len){
	size_t idx;
	ssize_t wcnt;
	if (len == 0)							//if there are no more bytes to be read
		return;
	idx = 0;								//counter for comparison of current bytes
	do {
		wcnt = write(fd, buff+idx, len - idx);
		//number of recorder bytes are returned
		if (wcnt<0){
			perror("write");
			exit(1);
		}
		idx+=wcnt;
		//add how many bytes have been returned
	} while(idx<len);						//until the overall number of bytes
}

void write_file(int fd, const char *infile){
	char buff[10];							//buffer size
	ssize_t rcnt;							//counts how many chars have been read by read
	int fdmp;								//int that is returned by file opening
	fdtmp=open(infile, O_RDONLY);			//file descriptor
	if(fdtmp<0){
		perror(infile);
		exit(1);
	}
	while((rcnt = read(fdtmp, buff, sizeof(buff)-1))>0){
		//while there is input to be read, rcnt counts the number of bytes that have been read
		if (rcnt == 0){						//end of file
			return;
		}
		if (rcnt<0){
			perror("read");
			exit(1);
		}
		buff[rcnt] = '\0';					//end of string character
											//initialise for doWrite
		doWrite(fb, buff, rcnt);			//call doWrite
	}
	close(fdtmp);
}