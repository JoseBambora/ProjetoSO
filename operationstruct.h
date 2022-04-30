typedef struct maxOperation
{
    char operation[20];
    int number;
    int max;
}MAXOPERATION;

typedef struct operations
{
    char tasks[2048][40];
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
    char cliente[1024];
    int espacos;
    int array[7];
    struct waitqueue *next;
} *WAITQUEUE;