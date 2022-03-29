#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char** argv){
	int fileWrite = open (argv[2], O_WRONLY);
	dup2(fileWrite, 1);
	execlp("gzip", "gzip", "-c", "-k",argv[1],NULL);
	perror("error executing command");	
	return 0;
}
