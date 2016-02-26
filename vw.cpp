// HMP (Headless Moyai Protocol viewer client)
#include "client.h"

class HMPClientConn : public Conn {
public:
    HMPClientConn(Network *nw, int fd) : Conn(nw,fd) {
        connecting = true;
    }
    ~HMPClientConn() {}

    virtual void onError( NET_ERROR e, int eno );
    virtual void onClose();
    virtual void onConnect();
    virtual void onPacket( uint16_t funcid, char *argdata, size_t argdatalen );
    
};

void HMPClientConn::onError( NET_ERROR e, int eno ) {
    print("HMPClientConn::onError. e:%d eno:%d",e,eno);
}
void HMPClientConn::onClose() {
    print("HMPClientConn::onClose.");    
}
void HMPClientConn::onConnect() {
    print("HMPClientConn::onConnect");        
}
void HMPClientConn::onPacket( uint16_t funcid, char *argdata, size_t argdatalen ) {
    print("HMPClientConn::onPacket");
    switch(funcid) {
    case PACKETTYPE_S2C_PROP2D_SNAPSHOT:
        print("PACKETTYPE_S2C_PROP2D_SNAPSHOT[%d] len:%d", funcid, argdatalen );
        break;
    default:
        print("unhandled packet type:%d", funcid );
        break;
    }
    
}

Network *g_nw;
HMPClientConn *g_conn;

int main( int argc, char **argv ) {

    const char *host = "localhost";
    if( argc > 1 && argv[1] ) host = argv[1];
    int port = 22222;
    if( argc > 2 && argv[2] ) port = atoi(argv[2]);
    print("viewer config: host:'%s' port:%d", host, port );
    
    Moyai::globalInitNetwork();
    g_nw = new Network();    
    int fd = g_nw->connectToServer(host,port);
    if(fd<0) {
        print("can't connect to server");
        return 1;
    }
    g_conn = new HMPClientConn(g_nw,fd);
    bool done = false;
    while(!done) {
        g_nw->heartbeat();
    }
    delete g_nw;
    return 0;
}
