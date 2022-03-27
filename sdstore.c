#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>


int main(int argc, char** argv)
{
    write(STDIN_FILENO,"Processing\n",11);
    int fileWrite = open(argv[2],O_CREAT | O_TRUNC | O_WRONLY, 0660);
    close(fileWrite);
    for(int i = 3; i < argc; i++)
    {
        if(fork() == 0)
            execlp(argv[i],argv[i],argv[1],argv[2],NULL);
    }
    for(int i = 3; i < argc; i++)
        wait(NULL);
    write(STDIN_FILENO,"Conclued\n",9);
}