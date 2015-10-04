// pverify, version 1.0
// Copyright (c) 2012 Zhu Hui
// E-mail: lookatmeyou@126.com
// QQ: 1241745499


#include "http.h"
#include <fcntl.h>
#include <sys/timeb.h>

const char http_header[256] =
        "GET http://www.baidu.com/ HTTP/1.1\r\n"
        "User-Agent: pverify 1.0\r\n"
        "Host: www.baidu.com\r\n"
        "Proxy-Connection: close\r\n"
        "Cache-Control: max-age=0\r\n"
        "Accept-Charset: utf-8\r\n"
        "\r\n";

Http::Http(): hl( strlen( http_header ) ) {}

Http::~Http() {}

int Http::create_socket() {
    if ( ( s = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) return -1;
    bzero( &addr, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_port = htons( port );
    addr.sin_addr.s_addr = inet_addr( ip );
    fcntl( s, F_SETFL, O_NONBLOCK );
    if ( connect( s, (sockaddr*)&addr, sizeof( addr ) ) == -1 ) {
        timeval tv;
        tv.tv_sec = contimeout;
        tv.tv_usec = 0;
        fd_set writeset;
        FD_ZERO( &writeset );
        FD_SET( s, &writeset );
        if ( select( s + 1, NULL, &writeset, NULL, &tv ) <= 0 ) return -1;
        if ( !FD_ISSET( s, &writeset ) ) return -1;
    }
    fcntl( s, F_SETFL, 2 );
    timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    if ( setsockopt( s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof( timeval ) ) < 0 ) {
        close( s );
        return -1;
    }
    if ( setsockopt( s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof( timeval ) ) < 0 ) {
        close( s );
        return -1;
    }
    if ( send( s, http_header, hl, 0 ) < 0 ) {
        close( s );
        return -1;
    }
    return s;
}

int Http::perform() {
    timeb t1;
    ftime( &t1 );
    size = 0;
    int s = create_socket();
    if ( s < 0 ) return -1;
    int ts;
    while ( ts = recv( s, buf + size, 10240 - size, 0 ), ts > 0 ) {
        size += ts;
        if ( size > 4096 ) break;
    }
    close( s );
    timeb t2;
    ftime( &t2 );
    time = ( t2.time - t1.time ) * 1000 + t2.millitm - t1.millitm;
    return 0;
}

