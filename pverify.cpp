// pverify. version 1.0
// Copyright (c) 2012 Zhu Hui
// E-mail: lookatmeyou@126.com
// QQ: 1241745499


#include <stdio.h>
#include <algorithm>
#include <pthread.h>
#include <vector>
#include <string.h>
#include <string>
#include <memory.h>
#include "http.h"
using namespace std;

struct proxy {
    char ip[16];
    char port[6];
    double time;
    friend bool operator < ( const proxy &a, const proxy &b ) {
        if ( a.time < 0 ) {
            if ( b.time < 0 ) return a.time > b.time;
            else return false;
        }
        else if ( b.time < 0 ) return true;
        else return a.time < b.time;
    }
};

vector<proxy> okay;
vector<string> filelist;
char ip[512][16], port[512][6];
pthread_mutex_t mutex, mutex_p;
int cnt, fcnt;
FILE *f;

void result_output() {
    sort( okay.begin(), okay.end() );
    puts( "Statistics:\n"
          "+-----------------+-----------+-----------+-----------+\n"
          "|    IP Adress    |    Port   |  Delay(s) |   Status  |\n"
          "+-----------------+-----------+-----------+-----------+" );
    int i, a = 0, u = 0;
    for ( i = 0; i < okay.size(); ++i ) {
        printf( "| %15s |   %5s   |", okay[i].ip, okay[i].port );
        if ( okay[i].time >= 0 ) {
            printf( "%8.3f   | available |\n", okay[i].time / 1000.0 );
            ++a;
        }
        else {
            printf( "%8.3f   |  unknown  |\n", -okay[i].time / 1000.0 );
            ++u;
        }
    }
    puts( "+-----------------+-----------+-----------+-----------+" );
    printf( "Total: %d available, %d unknown.\n", a, u );
}

int read_data( FILE *f ) {
    for ( cnt = 0; cnt < 512; ++cnt ) {
        if ( fscanf( f, "%15s%5s", ip[cnt], port[cnt] ) != 2 ) {
            break;
        }
    }
    return cnt;
}

unsigned char tag[18] = { 0xB0, 0xD9, 0xB6, 0xC8, 0xD2, 0xBB, 0xCF, 0xC2,
	0xA3, 0xAC, 0xC4, 0xE3, 0xBE, 0xCD, 0xD6, 0xAA, 0xB5, 0xC0 };
bool check( unsigned char data[], int cnt ) {
	int i, j;
	for ( i = 0; i <= cnt - 18; ++i ) {
		for ( j = 0; j < 18; ++j ) {
			if ( data[i + j] != tag[j] ) break;
		}
		if ( j >= 18 ) return true;
	}
	return false;
}

int str_to_int( char *str ) {
    int i, x = 0;
    for ( i = 0; str[i]; ++i ) {
         x = x * 10 + str[i] - '0';
    }
    return x;
}

void *verify( void *p ) {
    int x;
    proxy tmp;
    unsigned char buf[10240];
    Http url;
    url.init( 5, 3, buf );
    while ( true ) {
        pthread_mutex_lock( &mutex );
        if ( cnt == 0 ) {
            if ( read_data( f ) == 0 ) {
                fclose( f );
                for ( ++fcnt; fcnt < filelist.size(); ++fcnt ) {
                    f = fopen( filelist[fcnt].c_str(), "r" );
                    if ( f != NULL ) break;
                }
                if ( fcnt >= filelist.size() ) cnt = -1;
            }
            pthread_mutex_unlock( &mutex );
            continue;
        }
        else if ( cnt < 0 ) {
            pthread_mutex_unlock( &mutex );
            break;
        }
        x = --cnt;
        strcpy( tmp.ip, ip[x] );
        strcpy( tmp.port, port[x] );
        pthread_mutex_unlock( &mutex );
        url.set_proxy( tmp.ip, str_to_int( tmp.port ) );
        if ( url.perform() != 0 ) continue;
        if ( check( buf, url.get_size() ) ) tmp.time = url.total_time();
        else tmp.time = -url.total_time();
        pthread_mutex_lock( &mutex_p );
        okay.push_back( tmp );
        pthread_mutex_unlock( &mutex_p );
    }
    pthread_exit( NULL );
}

void usage( char appname[] ) {
    puts( "pverify 1.0\n"
          "Copyright (c) 2012 Zhu Hui\n"
          "E-mail: hi-there@139.com\n"
          "QQ: 1241745499" );
    printf( "\nUsage: %s filename1 filename2 ... [-t  N]\n", appname );
    puts( "\nfilenames are files that contain proxy server addresses and ports\n"
            "    to verify. The files' format should be like this:\n"
            "    IP1 port1\n"
            "    IP2 port2\n"
            "    ...\n"
            "    each line contains an ip address and its port separated by\n"
            "    spaces. For example:\n"
            "    192.168.13.23 3128\n"
            "    10.16.17.45   8080\n"
            "    ....\n"
            "-t  is optional. And it specifies the number of threads to work at\n"
            "    the same time by N. The default number of threads is 160.\n" );
}


inline void unknown_cmd( char appname[] ) {
    printf( "Unknown command. Try \"%s\" for help.\n", appname );
    exit( 0 );
}
int main( int argc, char *argv[] )
{
    if ( argc <= 1 ) {
        usage( argv[0] );
        return 0;
    }
    int i, thread_cnt = 160;
    for ( i = 1; i < argc; ++i ) {
        if ( argv[i][0] == '-' ) {
            if ( argv[i][1] == 't' && argv[i][2] == 0 ) {
                if ( i + 1 >= argc ) unknown_cmd( argv[0] );
                thread_cnt = str_to_int( argv[++i] );
            }
            else unknown_cmd( argv[0] );
        }
        else {
            filelist.push_back( argv[i] );
        }
    }
    if ( filelist.size() == 0 ) unknown_cmd( argv[0] );
    cnt = 0, f = NULL;
    for ( fcnt = 0; fcnt < filelist.size(); ++fcnt ) {
        f = fopen( filelist[fcnt].c_str(), "r" );
        if ( f != 0 ) break;
    }
    if ( fcnt >= filelist.size() ) {
        exit( 0 );
    }
    puts( "Please wait. Verifying ..." );
    if ( thread_cnt <= 0 ) thread_cnt = 160;
    pthread_t *threads = new pthread_t[ thread_cnt ];
    pthread_attr_t attr;
    pthread_mutex_init( &mutex, NULL );
    pthread_mutex_init( &mutex_p, NULL );
    pthread_attr_init( &attr );
    pthread_attr_setstacksize( &attr, 1024000 );
    pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
    for ( i = 0; i < thread_cnt; ++i ) {
        if ( pthread_create( threads + i, &attr, verify, NULL ) != 0 ) {
            thread_cnt = i;
            break;
        }
    }
    for ( i = 0; i < thread_cnt; ++i ) {
        pthread_join( threads[i], NULL );
    }
    delete[] threads;
    result_output();
    pthread_attr_destroy( &attr );
    pthread_mutex_destroy( &mutex );
    pthread_mutex_destroy( &mutex_p );
    pthread_exit( NULL );
}

