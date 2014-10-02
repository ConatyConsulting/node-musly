#ifndef JUKEBOX_H
#define JUKEBOX_H

#include <nan.h>
#include <musly/musly.h>

class Jukebox : public node::ObjectWrap {
public:
  static void Init();
  static v8::Local<v8::Object> NewInstance(v8::Local<v8::Value> arg);
  musly_jukebox* musly() { return mb_;};
  musly_jukebox* mb_;
  
private:
  Jukebox(const char* method, const char* decoder);
  ~Jukebox();
    
  static v8::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);
  static NAN_METHOD(AboutMethod);
  static NAN_METHOD(AnalyzeAudiofile);
  static NAN_METHOD(CreateTrack);
  static NAN_METHOD(AddTracks);
};

#endif