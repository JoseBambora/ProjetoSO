#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char** argv){
	char *exec_args[]={"ccrypt","-e","-K","123456",argv[1],NULL};
	execvp("ccrypt",exec_args);
	perror("error executing command");	
	return 0;
}
