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
    operations.numtasks = 0;
    close(fo);
    return operations;
}

OPERATION readStatus()
{
    OPERATION operations;
    int fo = open("server_status",O_RDONLY);
    if(fo < 0)
        operations = copyOperation();
    else
        read(fo,&operations,sizeof(operations));
    return operations;
}

int isOperationValid (int number, int max){
    if (number>max) return 0;
    else return 1;
}

void applyexec(char *exec_src)
{
    int i;
    for(i = 0; exec_src[i] != '\0'; i++);
    char exec[i+3];
    execname(exec_src,exec);
    execlp(exec,exec,NULL);
    perror("ERRO\n");
    _exit(1);
}

void closepipes(int **pipes, int argc)
{
    for(int i = 5; i < argc+1;i++)
    {
        close(pipes[i][1]);
        close(pipes[i][0]);
    }
}

void pedidoValido(OPERATION operations, OPERATION operations2, int argc, char **argv)
{
    for(int i = 4; i < argc; i++)
    {
        int index = numopera(argv[i]);
        operations2.ope[index].number++;
        int max = operations2.ope[index].max;
        int numberOps = operations2.ope[index].number;
        if (isOperationValid(numberOps,max) == 0)
        {
            perror("Pedido inválido");
            exit(1);
        }
    }
}

void preexecute(OPERATION operations, OPERATION operations2)
{
    for(int i = 0; i < 7; i++)
    {
        int pednum = operations2.ope[i].number;
        int pronum = operations.ope[i].number;
        int promax = operations.ope[i].max;
        if(pednum + pronum > promax)
        {
            // while até poder
        }
    }
}

void escreveOperations(OPERATION operations, int argc,char **argv)
{
    int opindex = operations.numtasks; // depois quando tivermos varios pedidos controlar melhor esta variavel
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
        OPERATION operations, operations2;
        operations = readStatus();
        operations2 = copyOperation(); // Só devolve o max
        pedidoValido(operations,operations2,argc,argv);
        preexecute(operations,operations2);
        escreveOperations(operations,argc,argv);
        // Começa as operações do ficheiro
        int f1 = dup(1);
        write(f1,"Processing\n",11);
        int fread = open(argv[indexler],O_RDONLY);
        int finalfile = open(argv[3],O_CREAT | O_TRUNC | O_WRONLY, 0660);
        int **pipes = (int **) malloc(sizeof (int *) * argc+1);
        for(int i = 5; i < argc+1;i++)
            pipes[i] = (int *) malloc(sizeof (int) * 2);
        for(int i = 5; i < argc+1;i++)
            pipe(pipes[i]);
        // Aplica a primeira operação ao ficheiro de leitura
        if(argc > 4)
        {
            if(fork() == 0)
            {
                dup2(fread,0);
                dup2(pipes[5][1],1);
                closepipes(pipes,argc);
                applyexec(argv[4]);
            }
        }
        // Vai aplicando as restantes operações
        for(int i = 5; i < argc; i++)
        {
            if(fork() == 0)
            {
                dup2(pipes[i][0],0);
                dup2(pipes[i+1][1],1);
                closepipes(pipes,argc);
                applyexec(argv[i]);
            }
        }
        // Copia o resultado da ultima operação para o ficheiro resultante
        if(fork() == 0)
        {
            dup2(pipes[argc][0],0);
            dup2(finalfile,1);
            closepipes(pipes,argc);
            applyexec("nop");
        }
        closepipes(pipes,argc);
        for(int i = 4; i < argc+1;i++)
            wait(NULL);
        close(finalfile);
        close(fread);
        write(f1,"Concluded\n",10);
    }
    return 0;
}