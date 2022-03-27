#include <unistd.h>
#include <fcntl.h>


int main(int argc, char** argv){
    int fileRead  = open(argv[1],O_RDONLY);
    int fileWrite = open(argv[2],O_WRONLY);
    char aux[1024];
    for(int byte_reads = read(fileRead,aux,sizeof(aux)); byte_reads > 0; byte_reads = read(fileRead,aux,sizeof(aux)))
          write(fileWrite,aux,byte_reads);
    close(fileRead);
    close(fileWrite);
    return 0;
}
