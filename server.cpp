#include <assert.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include<arpa/inet.h>
#include <stdbool.h>
#include <string.h>

const size_t k_max_msg = 4096;

static int32_t read_full(int fd, char * buf, size_t n){ //function cannot be called from outside file
    while(n > 0){
        // n is count of items
        ssize_t rv = read(fd, buf, n); 
        if(rv <= 0){
            return -1; //unexpected end of file
        }

        assert((size_t)rv <= n);
        n -= (size_t) rv;
        buf += rv;

    }

    return 0;


}

static int32_t write_all(int fd, const char * buf, size_t n){
    while(n > 0){
        ssize_t rv = write(fd, buf, n);

        if(rv <= 0){
            return -1;
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv; 
        buf += rv;

    }
    return 0;
}

static void die(const char * msg){
    int err = errno;
    fprintf(stderr,"[%d] %s\n", err, msg);
    abort();

}

static void msg(const char * msg){
    //print error message
    fprintf(stderr, "%s\n", msg);
}



static int32_t one_request(int connfd){
    
    char rbuf[4+k_max_msg]; 
    errno = 0; 
    int32_t err = read_full(connfd, rbuf, 4); 
    
    if(err){
        msg(errno == 0 ?"EOF": "read()error");
        return err;
    }
    
    uint32_t len = 0;
    //compute to see if length of recerived message is too long
    memcpy(&len, rbuf, 4);
    if(len > k_max_msg){
        msg("too long"); 
        return -1;
    }
    //request body
    err = read_full(connfd, &rbuf[4], len);
    if(err){
        msg("read() error");
        return err;
    }

    printf("client says: %.*s\n", len, &rbuf[4]);
    const char reply[] = "world"; 
    char wbuf[4 + sizeof(reply)];
    len = (uint32_t) strlen(reply); 
    memcpy(wbuf, &len, 4); 
    memcpy(&wbuf[4], reply, len);
    return write_all(connfd, wbuf, 4+len);
}

int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0); 

    if(fd <  0){
        //if socket handle is retrieved incorrectly
        die("socket()");
    }
    

    struct sockaddr_in addr = {}; // in arpa inet .h 
                                  // holds ipv4 port pair stored as big endian numbers
                                
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234); //port 1234 in big endian
    addr.sin_addr.s_addr = htonl(0); //wildcard ip address
                                     //htonl swaps endianness from cpu endian to big endian
    int rv = bind(fd, (const struct sockaddr*)&addr, sizeof(addr));
    

    if(rv){
        //if rv does not bind to a port correctly
        die("bind()");
    }
    
    
    //listen
    rv = listen(fd, SOMAXCONN); //listen on port which fd has been bound to with size of queue SOMAXCONN which is 4096
    if (rv) {
        die("listen()"); 
    }

    while(true){
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen); //accept connection
        
        if(connfd < 0){
            continue;
        }

        
        while(true){
            int32_t err = one_request(connfd);
            if(err){
                break;
            }
        }
        close(connfd); 
        
    }

    return 0;

}
