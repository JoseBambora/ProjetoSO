#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

void execname(char *src, char *result)
{
    int i,j;
    result[0] = '.';
    result[1] = '/';
    for(i = 0, j = 2; src[i] != '\0'; i++, j++)
        result[j] = src[i];
    result[j] = '\0';
}

void applyexec(char *exec_src, char*f1, char*f2)
{
    int i;
    for(i = 0; exec_src[i] != '\0'; i++);
    char exec[i+3];
    execname(exec_src,exec);
    execlp(exec,exec,f1,f2,NULL);
}

int main(int argc, char** argv)
{
    write(STDIN_FILENO,"Processing\n",11);
    int fileWrite = open(argv[2],O_CREAT | O_TRUNC | O_WRONLY, 0660);
    close(fileWrite);
    if(argc > 3)
    {
        if(fork() == 0)
            applyexec(argv[3],argv[1],argv[2]);
    }
    for(int i = 4; i < argc; i++)
    {
        if(fork() == 0)
            applyexec(argv[i],argv[2],argv[2]);
    }
    for(int i = 3; i < argc; i++)
        wait(NULL);
    write(STDIN_FILENO,"concluded\n",10);
}