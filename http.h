// pverify, version 1.0
// Copyright (c) 2012 Zhu Hui
// E-mail: lookatmeyou@126.com
// QQ: 1241745499


#include <string>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
using namespace std;

class Http {
private:
    char ip[16];
    int timeout, contimeout, port;
    unsigned char *buf;
    int size, time;
    sockaddr_in addr;
    int s;
    const int hl;
    int create_socket();
public:
    Http();
    ~Http();
    void init( int timeouta, int contimeouta, unsigned char *bufa ) {
        timeout = timeouta;
        contimeout = contimeouta;
        buf = bufa;
    }
    void set_timeout( int sec ) {
        timeout = sec;
    }
    void set_connecttimeout( int sec ) {
        contimeout = sec;
    }
    void set_buffer( unsigned char *bu ) {
        buf = bu;
    }
    void set_proxy( const char *ipa, int porta ) {
        strcpy( ip, ipa );
        port = porta;
    }
    int total_time() {
        return time;
    }
    int get_size() {
        return size;
    }
    int perform();
};
