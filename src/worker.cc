#include "worker.h"
#include "track.h"
#include "jukebox.h"

#include <nan.h>

#include <iostream>

using namespace v8;

AnalyzeAudioWorker::AnalyzeAudioWorker(NanCallback* callback,
    v8::Local<v8::Object> &jukeboxHandle,
    NanAsciiString* audiofile,
    float excerpt_length,
    float excerpt_start)
  : NanAsyncWorker(callback),  excerpt_length(excerpt_length), excerpt_start(excerpt_start), audiofile(audiofile) {

  SaveToPersistent("jukebox", jukeboxHandle);

  jukebox = node::ObjectWrap::Unwrap<Jukebox>(jukeboxHandle)->musly();
  track = musly_track_alloc(jukebox);
}

AnalyzeAudioWorker::~AnalyzeAudioWorker() {
  delete audiofile;
}

void AnalyzeAudioWorker::Execute() {
  int ret = musly_track_analyze_audiofile(jukebox,**audiofile,excerpt_length,excerpt_start,track);
  if (ret == -1) {
    SetErrorMessage("Error in musly_track_analyze_audiofile");
  }
}
    
void AnalyzeAudioWorker::HandleOKCallback() {
  NanScope();

  Local<Object> jsTrack = Track::NewInstance();
  Track* obj = ObjectWrap::Unwrap<Track>(jsTrack);
  obj->musly(track);
  obj->jukebox(jukebox);

  Local<Value> argv[] = {
    NanNull(),
    jsTrack
  };
  callback->Call(2, argv);
}


AddTracksWorker::AddTracksWorker(NanCallback *callback, v8::Local<v8::Object> &jukeboxHandle, v8::Local<v8::Array> &tracksHandle)
  : NanAsyncWorker(callback) {

  std::cout << "Add Tracks Worker: Constructor\n";
  SaveToPersistent("jukebox", jukeboxHandle);
  SaveToPersistent("tracks", tracksHandle);

  jukebox = node::ObjectWrap::Unwrap<Jukebox>(jukeboxHandle)->musly();

  std::cout << "Add Tracks Worker: Adding " << tracksHandle->Length() << " Tracks to vector\n";
  for (int index = 0, size = tracksHandle->Length(); index < size; index++) {
    Track* track = ObjectWrap::Unwrap<Track>(tracksHandle->Get(index).As<Object>());
    tracks.push_back(track->musly());
  }

  trackids.assign(tracks.size(), -1);
}

void AddTracksWorker::Execute() {
  std::cout << "Add Tracks Worker: Execute on " << tracks.size() << " tracks\n";
  std::cout << tracks.data() << "\n";
  std::cout << trackids.data() << "\n";
  int ret = musly_jukebox_addtracks(jukebox, tracks.data(), trackids.data(), tracks.size(), 1);
  if (ret == -1) {
    SetErrorMessage("Failed musly_jukebox_addtracks");
  }
}

void AddTracksWorker::HandleOKCallback() {
  NanScope();

  std::cout << "Add Tracks Worker: HandleOKCallback\n";
  Local<Array> ids = NanNew<Array>(trackids.size());
  for (int i = 0; i < (int)trackids.size(); i++) {
    ids->Set(i,NanNew<Integer>(trackids[i]));
  }

  Local<Value> argv[] = {
      NanNull(),
      ids
  };

  callback->Call(2,argv);
}