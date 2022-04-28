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