#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "operationstruct.h"



int main(int argc, char** argv)
{
    // enviar pedido por pipes com nome
    // servidor escreve o vai escrevendo as estapas para cada pedido
    char pedido[1024];
    if(argc > 1)
        strcpy(pedido,argv[1]);
    for(int i = 2; i < argc; i++)
    {
        strcat(pedido," ");
        strcat(pedido,argv[i]);
    }
    char k[1031];
    if(argc < 2 || *argv[1] != 's')
    {
        strcpy(k,"acabei ");
        strcat(k,pedido);
        strcat(pedido,"\0");
    }
    int server = open("tmp/cliente_server",O_WRONLY);
    write(server,pedido,sizeof(pedido));
    close(server);
    int bytes = 1;
    while(bytes > 0)
    {
        server = open("tmp/server_cliente", O_RDONLY);
        bytes = read(server,pedido,sizeof(pedido));
        write(1,pedido,bytes);
        close(server);
    }
    if(argc < 2 || *argv[1] != 's')
    {
        server = open("tmp/cliente_server", O_WRONLY);
        write(server,k,sizeof(k));
        close(server);
    }
    return 0;
}