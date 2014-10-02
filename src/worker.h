#ifndef WORKER_H
#define WORKER_H

#include <vector>
#include "jukebox.h"
#include <nan.h>
#include <musly/musly.h>

class AnalyzeAudioWorker : public NanAsyncWorker {
public:
  AnalyzeAudioWorker(NanCallback* callback,
      v8::Local<v8::Object> &jukeboxHandle,
      NanAsciiString* audiofile,
      float excerpt_length,
      float excerpt_start);
  ~AnalyzeAudioWorker();
  void Execute();
  void HandleOKCallback();
    
private:
  musly_jukebox* jukebox;
    float excerpt_length;
    float excerpt_start;
    musly_track* track;
    NanAsciiString* audiofile;
};

class AddTracksWorker : public NanAsyncWorker {
public:
  AddTracksWorker(NanCallback * callback,
      v8::Local<v8::Object> &jukeboxHandle,
      v8::Local<v8::Array> &tracksHandle);
  void Execute();
  void HandleOKCallback();

private:
  musly_jukebox* jukebox;
  std::vector<musly_track*> tracks;
  std::vector<musly_trackid> trackids;
  int num_tracks;
  int generate_ids;
};

#endif