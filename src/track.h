#ifndef TRACK_H
#define TRACK_H

#include <nan.h>
#include <musly/musly.h>

using namespace v8;
using namespace node;

class Track : public ObjectWrap {
public:
  static void Init();
  static Local<Object> NewInstance();
  musly_track* musly() { return track_; }
  void musly(musly_track* track) { track_ = track; }
  musly_jukebox* jukebox() { return jukebox_; }
  void jukebox(musly_jukebox* jukebox) { jukebox_ = jukebox; }
    
private:
  Track();
  ~Track();
  musly_track* track_;
  musly_jukebox* jukebox_;
    
  static Persistent<Function> constructor;
  static NAN_METHOD(New);
  static NAN_METHOD(ToString);
  static NAN_METHOD(ToBuffer);
};

#endif