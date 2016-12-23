#ifndef _POOL_H_
#define _POOL_H_

#include <unordered_map>

// Generic ID > pointer pool 
template <class Obj> class ObjectPool {
public:
    std::unordered_map<unsigned int,Obj*> idmap;
    ObjectPool() {};
    void set(unsigned int id, Obj *ptr ) {
        idmap[id] = ptr;
    }
    Obj *get(unsigned int id) {
        if( idmap.find(id) == idmap.end() ) return NULL; else return idmap[id];
    };
    Obj *alloc(unsigned int id) {
        Obj *ptr = new Obj();
        ptr->id = id;
        set( id, ptr );
        return ptr;
    }
    int del(unsigned int id) {
        int n = idmap.erase(id);
        return n;
    }    
    Obj *ensure(unsigned int id) {
        Obj *ptr = get(id);
        if(!ptr) {
            ptr = alloc(id);
        }
        return ptr;
    }
    Obj *ensure(unsigned int id, int arg0, int arg1 ) {
        Obj *ptr = get(id);
        if(!ptr) {
            ptr = new Obj(arg0, arg1);
            ptr->id = id;
            set(id,ptr);
        }
        return ptr;
    }
    int size() {
        return idmap.size();
    }
    Obj *getFirst() {
        if( idmap.size()==0 ) return NULL; else return idmap.begin()->second;
    }
};

#define POOL_SCAN(NAME,T) for( std::unordered_map<unsigned int,T*>::iterator it = NAME.idmap.begin(); it != NAME.idmap.end(); ++it )

        
#endif
