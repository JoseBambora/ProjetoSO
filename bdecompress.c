#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char** argv){
	execlp("bzip2","bzip2","-k","-d",argv[1],NULL);
	perror("error executing command");
	return 0;
}
