#ifndef _VW_H_
#define _VW_H_

class File {
public:
    char path[256];
    char *data;
    size_t data_len;

    File( const char *path, const char *indata, size_t indata_len );
    bool comparePath( const char *comparepath ) {
        return strcmp( path, comparepath) == 0 ;
    }
    bool saveInTmpDir( const char *dir_prefix, char *outpath, size_t outpathsize );
};
class FileDepo {
public:
    static const int MAX_FILES = 128;
    File *files[MAX_FILES];
    FileDepo() {
        for(int i=0;i<MAX_FILES;i++) files[i] = NULL;
    };
    File* get( const char *path );
    File *ensure( const char *path, const char *data, size_t datalen );
    File *getByIndex(int ind);    
};



#endif

