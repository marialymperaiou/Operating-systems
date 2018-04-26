#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "zing.h"

void zing(){
	const char *name;
	name = getlogin();
	printf("Hello %s", name);
	printf("!\n");
}