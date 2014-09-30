#ifndef JUKEBOX_H
#define JUKEBOX_H

#include <nan.h>

class Jukebox : public node::ObjectWrap {

  public:
    static void Init();
    static v8::Local<v8::Object> NewInstance(v8::Local<v8::Value> arg);
  
  private:
    Jukebox(const char* method, const char* decoder);
    ~Jukebox();
    
    musly_jukebox* mb_;
    
    static v8::Persistent<v8::Function> constructor;
    static NAN_METHOD(New);
    static NAN_METHOD(About);
};

#endif