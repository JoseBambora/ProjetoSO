#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "operationstruct.h"

int main(int argc, char** argv)
{
    // enviar pedido por pipes com nome
    // servidor escreve o vai escrevendo as estapas para cada pedido
    TASK task;
    pid_t cliente = getpid();
    sprintf(task.cliente,"tmp/%d",cliente);
    mkfifo(task.cliente,0666);
    if(argc > 1)
        strcpy(task.pedido,argv[1]);
    for(int i = 2; i < argc; i++)
    {
        strcat(task.pedido," ");
        strcat(task.pedido,argv[i]);
    }
    TASK task2;
    if(argc < 2 || *argv[1] != 's')
    {
        strcpy(task2.pedido,"acabei ");
        strcat(task2.pedido,task.pedido);
    }
    int server = open("tmp/cliente_server",O_WRONLY);
    write(server,&task,sizeof(task));
    close(server);
    int bytes;
    server = open(task.cliente, O_RDONLY);
    char pedido[1024];
    int i = 0;
    char c = '0';
    while((bytes = read(server,pedido,sizeof(pedido))) > 0 && c != 'w')
    {
        c = *pedido;
        if(c != 'w')write(1,pedido,bytes);
        i++;
    }
    close(server);
    if(c == 'w')
    {
        server = open(task.cliente, O_RDONLY);
        while((bytes = read(server,pedido,sizeof(pedido))) > 0)
        {
            write(1,pedido,bytes);
            i++;
        }
        close(server); 
    }
    if(c != 'I' && c!= 'S' && i > 2 && argc > 3)
    {
        server = open("tmp/cliente_server", O_WRONLY);
        write(server,&task2,sizeof(task2));
        close(server);
    }
    // unlink pipecomnome
    unlink(task.cliente);
    return 0;
}