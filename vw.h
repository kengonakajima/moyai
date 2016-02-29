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

class File {
public:
    char path[256];
    char *data;
    size_t data_len;

    File( const char *path, const char *indata, size_t indata_len );
    bool comparePath( const char *comparepath ) {
        return strcmp( path, comparepath) == 0 ;
    }
};
class FileDepo {
public:
    static const int MAX_FILES = 128;
    File *files[MAX_FILES];
    FileDepo() {
        for(int i=0;i<MAX_FILES;i++) files[i] = NULL;
    };
    File* get( char *path );
    File *ensure( char *path, char *data, size_t datalen );
    File *getByIndex(int ind);    
};



#endif

