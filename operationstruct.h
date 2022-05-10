typedef struct maxOperation
{
    char operation[20];
    int number;
    int max;
}MAXOPERATION;

typedef struct execstatus
{
    char pedido[1024];
    int nrpedido;
    struct execstatus *next;
} *EXECSTATUS;

typedef struct operations
{
    EXECSTATUS execstatus;
    int numtasks;
    MAXOPERATION ope[7];
}OPERATION;

typedef struct task
{
    char pedido[1024];
    char cliente[1024];
}TASK;

typedef struct waitqueue
{
    char *pedido[1024];
    char pedidob[1024];
    int espacos;
    int file;
    int array[7];
    long time;
    int prioridade;
    struct waitqueue *next;
} *WAITQUEUE;