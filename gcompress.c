#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char** argv){
	execlp("gzip", "gzip", "-c", "-k",argv[1],NULL);
	perror("error executing command");	
	return 0;
}
