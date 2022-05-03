#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include "operationstruct.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

char path[1024];
int indexler = 1;
int indexescrever = 2;
char pending[] = "Pending\n";
char processing[] = "Processing\n";
char invalido[] = "Invalido\n";
char sobrecarga[] = "Servidor sobrecarregado, pedido descartado\n";

int fleitura, fescrita;

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

int countBytes(int file)
{
    lseek(file,0,SEEK_SET);
    char aux[1024];
    int result = 0;
    int bytes_reads = 0;
    while((bytes_reads = read(file,aux,sizeof(aux))) > 0)
        result += bytes_reads;
    return result;
}

void printStatus(OPERATION operations, int fescrita)
{
    char escrever[2048];
    EXECSTATUS execstatus = operations.execstatus;
    while(execstatus)
    {
        int j = sprintf(escrever,"Task %d: %s\n",execstatus->nrpedido, execstatus->pedido);
        write(fescrita,escrever,j);
        execstatus = execstatus->next;
    }
    for(int i  = 0; i < 7; i++)
    {
        int n1 = operations.ope[i].max;
        int n2 = operations.ope[i].number;
        int j = sprintf(escrever,"Transf: %s (%d/%d) (running/max)\n",operations.ope[i].operation,n2,n1);
        write(fescrita,escrever,j);
    }
    close(fescrita);
}

void execname(char *src, char *result)
{
    strcpy(result,path);
    strcat(result,src);
}

void applyexec(char *exec_src)
{
    int i;
    for(i = 0; exec_src[i] != '\0'; i++);
    char exec[i+strlen(path)];
    execname(exec_src,exec);
    execlp(exec,exec,NULL);
    perror("\nExec erro \n");
    _exit(1);
}

void closepipes(int **pipes, int argc)
{
    for(int i = 4; i < argc+1;i++)
    {
        close(pipes[i][1]);
        close(pipes[i][0]);
    }
}

void finalprocess(int f1, int fread, int finalfile)
{
    int bytesinput = countBytes(fread);
    int bytesoutput = countBytes(finalfile);
    close(fread);
    close(finalfile);
    char concluded[1024];
    sprintf(concluded,"Concluded (bytes-input: %d, bytes-output: %d)\n",bytesinput,bytesoutput);
    write(f1,concluded,strlen(concluded));
    close(f1);
}

void finalprocess2(int f1, int fread, int finalfile)
{
    int bytesinput = 0;
    int bytesoutput = 0;
    int pipesbytes[2][2];
    pipe(pipesbytes[0]);
    pipe(pipesbytes[1]);
    if(fork() == 0)
    {
        close(pipesbytes[0][0]);
        close(pipesbytes[1][0]);
        close(pipesbytes[1][1]);
        bytesinput = countBytes(fread);
        write(pipesbytes[0][1],&bytesinput,sizeof(int));
        close(pipesbytes[0][1]);
        close(fread);
        _exit(0);
    }
    if(fork() == 0)
    {
        close(pipesbytes[0][0]);
        close(pipesbytes[1][0]);
        close(pipesbytes[0][1]);
        bytesoutput = countBytes(finalfile);
        write(pipesbytes[1][1],&bytesoutput,sizeof(int));
        close(pipesbytes[1][1]);
        close(finalfile);
        _exit(0);
    }
    close(fread);
    close(finalfile);
    wait(NULL);
    wait(NULL);
    close(pipesbytes[0][1]);
    close(pipesbytes[1][1]);
    read(pipesbytes[0][0],&bytesinput,sizeof(int));
    read(pipesbytes[1][0],&bytesoutput,sizeof(int));
    close(pipesbytes[0][0]);
    close(pipesbytes[1][0]);
    char concluded[1024];
    sprintf(concluded,"Concluded (bytes-input: %d, bytes-output: %d)\n",bytesinput,bytesoutput);
    write(f1,concluded,strlen(concluded));
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

void execpedido(int argc, char **argv, int f1)
{
    write(f1,processing,sizeof(processing));
    int fread = open(argv[indexler],O_RDONLY);
    int finalfile = open(argv[indexescrever],O_CREAT | O_TRUNC | O_RDWR, 0660);
    int lim = argc-1;
    int **pipes = (int **) malloc(sizeof (int *) * argc);
    for(int i = 4; i < argc;i++)
        pipes[i] = (int *) malloc(sizeof (int) * 2);
    for(int i = 4; i < argc;i++)
        pipe(pipes[i]);
    // Aplica a primeira operação ao ficheiro de leitura
    if(argc > 3)
    {
        if(fork() == 0)
        {
            dup2(fread,0);
            if(argc == 4)
                dup2(finalfile,1);
            else
                dup2(pipes[4][1],1);
            closepipes(pipes,lim);
            applyexec(argv[3]);
        }
    }
    if(argc > 4)
    {
        // Vai aplicando as restantes operações
        for(int i = 4; i < lim; i++)
        {
            if(fork() == 0)
            {
                dup2(pipes[i][0],0);
                dup2(pipes[i+1][1],1);
                closepipes(pipes,lim);
                applyexec(argv[i]);
            }
        }
        // Ultima operação para o ficheiro resultante
        if(fork() == 0)
        {
            dup2(pipes[lim][0],0);
            dup2(finalfile,1);
            closepipes(pipes,lim);
            applyexec(argv[lim]);
        }
    }
    closepipes(pipes,lim);
    for(int i = 4; i < argc;i++)
        free(pipes[i]);
    free(pipes);
    for(int i = 3; i < argc;i++)
        wait(NULL);
    finalprocess(f1,fread,finalfile);
}

void addQueue(WAITQUEUE *queue, char *pedido[], char cliente[], int array[], int espacos, char pedidob[])
{
    WAITQUEUE aux = *queue;
    WAITQUEUE add = malloc(sizeof(struct waitqueue));
    add->espacos = espacos;
    strcpy(add->cliente,cliente);
    strcpy(add->pedidob,pedidob);
    for(int i = 0; i < espacos; i++)
    {
        add->pedido[i] = (char *) malloc(sizeof(char)* 1024);
        strcpy(add->pedido[i],pedido[i]);
    }
    for(int i = 0; i < 7; i++)
        add->array[i] = array[i];
    add->next = NULL;
    if(aux != NULL)
    {
        while(aux->next != NULL)
            aux = aux->next;
        aux->next = add;
    }
    else
        *queue = add;
}

void addPedidoOperation(EXECSTATUS *execstatus, int num, char pedido[])
{
    EXECSTATUS add = malloc(sizeof(struct execstatus));
    add->nrpedido = num;
    strcpy(add->pedido,pedido);
    add->next = NULL;
    EXECSTATUS aux = *execstatus;
    EXECSTATUS ant = NULL;
    while(aux)
    {
        ant = aux;
        aux = aux->next;
    }
    if(ant)
        ant->next = add;
    else
        *execstatus = add;
}

void removePedidoOperation(EXECSTATUS *execstatus, char pedido[])
{
    pedido += 7;
    EXECSTATUS aux = *execstatus;
    EXECSTATUS ant = NULL;
    while(aux)
    {
        if(strcmp(aux->pedido,pedido) == 0)
        {
            if(ant)
                ant->next = aux->next;
            else
                *execstatus = aux->next;
            free(aux);
            break;
        }
        ant = aux;
        aux = aux->next;
    }
}

int canExecute(const int array[], OPERATION operation)
{
    for(int i = 0; i < 7; i++)
        if(array[i] + operation.ope[i].number > operation.ope[i].max)
            return 0;
    return 1;
}

int main(int argc, char **argv)
{
    char*caminho = argv[2];
    strcpy (path, caminho);
    OPERATION operation;
    int f1 = open(argv[1],O_RDONLY);
    char buf[1024];
    int i = 0;
    operation.numtasks = 0;
    WAITQUEUE queue = NULL;
    operation.execstatus = NULL;
    while(readln(f1,buf,sizeof(buf)) > 0)
    {
        char *aux;
        int n = numopera(strtok_r(buf," ",&aux)) + 1;
        if(n > 0)
        {
            n--;
            strcpy(operation.ope[n].operation,buf);
            operation.ope[n].max = atoi(strtok(aux,"\n"));
            operation.ope[n].number = 0;
            i++;
        }
    }
    unlink("tmp/cliente_server");
    if(mkfifo("tmp/cliente_server",0666)<0)
        write(2,"erro\n",5);
    // Operations ir trabalhando
    int numpedidos = 0;
    int finfo2 = open("tmp/cliente_server", O_RDONLY); 
    int faux = open("tmp/cliente_server",O_WRONLY);
    while(1)
    {  
        // lê pedido  
        TASK task;
        char pedido[1024];
        char pedidob[1024];
        read (finfo2,&task,sizeof(task));
        strcpy(pedido,task.pedido);
        strcpy(pedidob,pedido);
        char *pedidoaux;
        pedidoaux = pedido;
        char *componentes[1024];
        int espacos = 1;
        for(int i = 0; pedidoaux[i] != '\0'; i++)
            if(pedidoaux[i] == ' ')
                espacos++;
        for(int i = 0; i < espacos  ;i++)
        {
            componentes[i] = (char *) malloc(sizeof(char)* 1024);
            strcpy(componentes[i],strsep(&pedidoaux," "));
        }
        if(*pedido == 'a')
        {
            removePedidoOperation(&operation.execstatus,pedidob);
            for(int i = 4; i < espacos; i++)
                operation.ope[numopera(componentes[i])].number--;
            WAITQUEUE aux = queue;
            WAITQUEUE ant = NULL;
            while(aux)
            {
                int exec = 0;
                if(canExecute(aux->array,operation))
                {
                    exec = 1;
                    for(int i = 0; i < 7; i++)
                    {
                        operation.ope[i].number+=aux->array[i];
                    }
                    numpedidos++;
                    addPedidoOperation(&operation.execstatus,numpedidos,aux->pedidob);
                    int finfo1 = open(aux->cliente, O_WRONLY); // adaptar
                    if (fork() == 0) 
                    {
                        if (*pedido == 's' || aux->espacos == 1)
                            printStatus(operation, finfo1);
                        else
                            execpedido(aux->espacos, aux->pedido, finfo1);
                        _exit(0);
                    }
                    close(finfo1);
                }
                if(exec)
                {
                    WAITQUEUE aux2 = aux;
                    aux = aux->next;
                    if(ant)
                        ant->next = aux;
                    else
                        queue = aux;
                    free(aux2);
                }
                else
                {
                    ant = aux;
                    aux = aux->next;
                }
            }
        }
        else
        {
            write(1,"Novo pedido\n",12);
            int finfo1 = open(task.cliente, O_WRONLY); // adaptar
            write(finfo1,pending,sizeof(pending));
            int arrayaux[7] = {0,0,0,0,0,0,0};
            for(int i = 0; i < espacos; i++)
            {
                int index = numopera(componentes[i]);
                if(index >= 0)
                {
                    arrayaux[index]++;
                    if(arrayaux[index] > operation.ope[index].max)
                    {
                        write(finfo1,invalido,sizeof(invalido));
                        espacos = 0;
                    }
                }
            }
            if(espacos > 0)
            {
                int exec = 1;
                for(int i = 0; i < 7; i++)
                {
                    if(canExecute(arrayaux,operation) == 0)
                    {
                        exec = 0;
                        break;
                    }
                }
                if(exec) 
                {
                    if (*pedido != 's' && espacos > 1)
                    {
                        numpedidos++;
                        addPedidoOperation(&operation.execstatus,numpedidos,pedidob);
                    }
                    for(int i = 0; i < 7; i++)
                        operation.ope[i].number+=arrayaux[i];
                    if (fork() == 0) 
                    {
                        if (*pedido == 's' || espacos == 1)
                            printStatus(operation, finfo1);
                        else
                            execpedido(espacos, componentes, finfo1);
                        _exit(0);
                    }
                }
                else
                {
                    addQueue(&queue,componentes,task.cliente,arrayaux,espacos,pedidob);
                    write(finfo1,"w",1);
                }
            }
            close(finfo1);
            for(int i = 0; i < espacos  ;i++)
                free(componentes[i]);
        }
    }    
    unlink("server_cliente");
    unlink("cliente_server");
    close(finfo2);
    close(faux);
    return 0;
}