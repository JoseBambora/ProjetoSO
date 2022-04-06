#include <unistd.h>
#include <fcntl.h>


int main(int argc, char** argv){
    char aux[1024];
    for(int byte_reads = read(0,aux,sizeof(aux)); byte_reads > 0; byte_reads = read(0,aux,sizeof(aux)))
          write(1,aux,byte_reads);
    return 0;
}
