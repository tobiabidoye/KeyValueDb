#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <netinet/in.h>
#include <stdio.h> 
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>


static void die(const char * msg){
    //function for printing out error messages
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();

}

int main(){

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        die("socket()");
    }
    
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET; 
    addr.sin_port = ntohs(1234); 
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));

    if(rv){
        die("connect");
    }
    
    char msg[] = "hello"; 
    write(fd, msg, strlen(msg));
    
    char rbuf[64] = {};
    ssize_t n = read(fd, rbuf, sizeof(rbuf) -1);

    if(n < 0){
        die("read");
    }
    
    printf("server says: %s \n", rbuf);
    close(fd);




}
