// HMP (Headless Moyai Protocol viewer client)
#include "client.h"


Network *g_nw;
Conn *g_conn;

int main( int argc, char **argv ) {

    const char *host = "localhost";
    if( argc > 1 && argv[1] ) host = argv[1];
    int port = 22222;
    if( argc > 2 && argv[2] ) port = atoi(argv[2]);
    print("viewer config: host:'%s' port:%d", host, port );
    
    Moyai::globalInitNetwork();
    g_nw = new Network();    
    g_conn = g_nw->connectToServer(host,port);
    bool done = false;
    while(!done) {
        g_nw->heartbeat();
    }
    delete g_nw;
    return 0;
}
