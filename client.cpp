#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <stdio.h> 
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include  <assert.h>
#include <string.h>

const size_t k_max_msg = 4096;

static void msg(const char * msg){
    fprintf(stderr, "%s\n", msg);
}

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
    //function for printing out error messages
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();

}

static int32_t query(int fd, const char * text){
    
    uint32_t len = (uint32_t) strlen(text);
    if(len > k_max_msg){
        return -1; 
    }
    
    //send message
    char wbuf[4 + k_max_msg];
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], text, len);
    if(int32_t err = write_all(fd, wbuf, 4 + len)){
        return err;
    }

    //4 bytes header
    char rbuf[4 + k_max_msg + 1]; 
    errno = 0;
    int32_t err = read_full(fd, rbuf, 4);

    if(err){
        msg(errno == 0 ?"EOF": "read() error");
    }
    
    memcpy(&len, rbuf, 4); 
    if(len > k_max_msg){

        msg("too long");
        return -1;
    }
    // receive response
    
    err = read_full(fd, &rbuf[4], len);
    if(err){
        msg("read() error");
        return err;
    }

    printf("server says: %.*s\n", len, &rbuf[4]);
    return 0;
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
    
    int32_t err = query(fd, "hello1");
    if(err){
        goto L_DONE ;
    }
    
    err = query(fd, "hello2");
    if(err){
        goto L_DONE ;
    }
L_DONE:
    close(fd);
    return 0;

}
