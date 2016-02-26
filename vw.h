#ifndef _VW_H_
#define _VW_H_


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



#endif

