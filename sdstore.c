#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "operationstruct.h"

int indexler = 2;
int indexescrever = 3;

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
    return r-1;
}

void execname(char *src, char *result)
{
    int i,j;
    result[0] = '.';
    result[1] = '/';
    for(i = 0, j = 2; src[i] != '\0'; i++, j++)
        result[j] = src[i];
    result[j] = '\0';
}


void printStatus()
{
    OPERATION operations;
    int finfo = open("server_status",O_RDWR);
    if(finfo < 0)
    {
        perror("Ficheiro status inexistente\n");
        exit(1);
    }
    read(finfo,&operations,sizeof(operations));
    char escrever[1024];
    for(int i = 0; i < operations.numtasks; i++)
    {
        sprintf(escrever,"Task %d: %s\n",i,operations.tasks[i]);
        int i;
        for(i = 0; escrever[i] != '\0'; i++);
        write(1,escrever,i);
    }
    for(int i  = 0; i < 7; i++)
    {
        int n1 = operations.ope[i].max;
        int n2 = operations.ope[i].number;
        sprintf(escrever,"Transf: %s (%d/%d) (running/max)\n",operations.ope[i].operation,n2,n1);
        int i;
        for(i = 0; escrever[i] != '\0'; i++);
        write(1,escrever,i);
    }
}

void saveOperation(OPERATION operations)
{
    int f3 = open("server_status",O_CREAT | O_TRUNC | O_WRONLY, 0660);
    write(f3,&operations,sizeof(operations));
    close(f3);
}

OPERATION copyOperation()
{
    // Em vez de server_max_info talvez server_status de forma a dar update à informação
    OPERATION operations;
    int fo = open("server_max_info",O_RDWR);
    if(fo < 0)
    {
        perror("Servidor não está aberto\n");
        exit(1);
    }
    read(fo,operations.ope,sizeof(operations.ope));
    close(fo);
    return operations;
}

void removeauxfiles()
{
    if(fork() == 0)
        execlp("rm","rm","aux1",NULL);
    if(fork() == 0)
        execlp("rm","rm","aux2",NULL);
    wait(NULL);
    wait(NULL);
}

void applyexec(char *exec_src, int f1, int f2)
{
    lseek(f1,SEEK_SET,0);
    lseek(f2,SEEK_SET,0);
    dup2(f2,1);
    dup2(f1,0);
    int i;
    for(i = 0; exec_src[i] != '\0'; i++);
    char exec[i+3];
    execname(exec_src,exec);
    execlp(exec,exec,NULL);
    perror("ERRO\n");
}

int main(int argc, char** argv)
{
    if(argc == 2)
    {
        printStatus();
        return 0;
    }
    if(strcmp(argv[1],"proc-file") == 0)
    {
        // Se não for possivel atender o pedido, interromper o processo aqui (max_operações == operações atuais)
        OPERATION operations = copyOperation();
        int opindex = 0; // depois quando tivermos varios pedidos controlar melhor esta variavel
        // Talvez meter esta parte concorrente e usar semáforos de forma a não devolver valores errados.
        for(int i = 4; i < argc; i++)
        {
            strcat(operations.tasks[opindex],argv[i]);
            strcat(operations.tasks[opindex]," ");
            int n = numopera(argv[i]);
            operations.ope[n].number++;
        }
        operations.numtasks = opindex + 1;
        saveOperation(operations); 
        int f1 = dup(1);
        write(f1,"Processing\n",11);
        int fread = open(argv[indexler],O_RDONLY);
        int faux1 = open("aux1",O_CREAT | O_TRUNC | O_RDWR, 0660);
        int faux2 = open("aux2",O_CREAT | O_TRUNC | O_RDWR, 0660);
        int finalfile = open(argv[3],O_CREAT | O_TRUNC | O_WRONLY, 0660);
        // Programa está de forma sequencial devido a problemas de concorrência
        if(argc > 4)
        {
            if(fork() == 0)
                applyexec(argv[4],fread,faux1);
            else
                wait(NULL);
        }
        for(int i = 5; i < argc; i++)
        {
            if(fork() == 0)
            {
                if(i%2 == 1)
                    applyexec(argv[i],faux1,faux2);
                else
                    applyexec(argv[i],faux2,faux1);
            }
            else
                wait(NULL);
        }
        if(fork() == 0)
        {
            if(argc%2 == 0)
                applyexec("nop",faux2,finalfile);
            else
                applyexec("nop",faux1,finalfile);
        }
        else
            wait(NULL);
        removeauxfiles();
        close(finalfile);
        close(fread);
        close(faux1);
        close(faux2);
        write(f1,"Concluded\n",10);
    }
    return 0;
}