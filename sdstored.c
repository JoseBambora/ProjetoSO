#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include "operationstruct.h"

int numopera(char a[])
{
    int r = 0;
    if(strcmp(a,"nop") == 0)
        r = 1;
    else if(strcmp(a,"bcompress") == 0)
        r = 2;
    else if(strcmp(a,"bdecompress") == 0)
        r = 3;
    else if(strcmp(a,"gcompress") == 0)
        r = 4;
    else if(strcmp(a,"gdecompress") == 0)
        r = 5;
    else if(strcmp(a,"encrypt") == 0)
        r = 6;
    else if(strcmp(a,"decrypt") == 0)
        r = 7;
    return r;
}

ssize_t readln(int fd, char *line, size_t size)
{
    int bytes_reads = read(fd,line,size);
    if(bytes_reads == 0) return 0;
    int i;
    for(i = 0 ; i < bytes_reads; i++)
        if(line[i] == '\n')
            break;
    i++;
    if(i < bytes_reads) lseek(fd,i-bytes_reads,SEEK_CUR);
    ssize_t r;
    if(i > bytes_reads) r = bytes_reads;
    else r = i;
    return r;
}
// Ainda falta guardar a pasta aonde estarão guardados os executáveis
int main(int argc, char** argv)
{
    if(fork() == 0)
    {
        char*caminho = argv[2];
        char pathString[1024];
        strcpy (pathString, caminho);
        //int size = strlen(caminho);
        int fpath = open("path",O_CREAT | O_TRUNC | O_WRONLY, 0660);
        write(fpath,pathString, sizeof(pathString));
        close (fpath);
        _exit(0);
    }
    if(fork() == 0)
    {
        MAXOPERATION o[7];
        int f1 = open(argv[1],O_RDONLY);
        char buf[1024];
        int i = 0;
        while(readln(f1,buf,sizeof(buf)) > 0)
        {
            char *aux;
            int n = numopera(strtok_r(buf," ",&aux));
            if(n > 0)
            {
                n--;
                strcpy(o[n].operation,buf);
                o[n].max = atoi(strtok(aux,"\n"));
                o[n].number = 0;
                i++;
            }
        }
        if(i > 0)
        {
            int f2 = open("server_max_info",O_CREAT | O_TRUNC | O_WRONLY, 0660);
            write(f2,o,sizeof(o));
            close(f2);
            /*
            f2 = open("server_max_info",O_RDONLY);
            MAXOPERATION r[7];
            read(f2,r,sizeof(r));
            for(int i = 0; i < 7; i++)
            {
                printf("%s %d %d\n",r[i].operation, r[i].max, r[i].number);
            }
            */   
        }
        _exit(0);
    }
    wait(NULL);
    wait(NULL);
    return 0;
}