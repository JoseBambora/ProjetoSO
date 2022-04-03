#include <unistd.h>
#include <fcntl.h>


int main(int argc, char** argv){
    int fileRead  = open(argv[1],O_RDONLY);
    char aux[1024];
    for(int byte_reads = read(fileRead,aux,sizeof(aux)); byte_reads > 0; byte_reads = read(fileRead,aux,sizeof(aux)))
          write(1,aux,byte_reads);
    close(fileRead);
    return 0;
}
