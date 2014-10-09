#ifndef WORKER_H
#define WORKER_H

#include <vector>
#include <nan.h>
#include <musly/musly.h>

#include "jukebox.h"

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
  Jukebox* jukebox;
  float excerpt_length;
  float excerpt_start;
  NanAsciiString* audiofile;
  unsigned char* trackBuffer;
};

class ReadyWorker : public NanAsyncWorker {
public:
  ReadyWorker(NanCallback* callback,
      v8::Local<v8::Object> &jukeboxHandle,
      int samplesize);
  void Execute();
  void HandleOKCallback();

private:
  Jukebox* jukebox;
  int sampleSize;
};

class RecommendWorker : public NanAsyncWorker {
public:
  RecommendWorker(NanCallback* callback,
      v8::Local<v8::Object> &jukeboxHandle,
      int seed,int topN,int guessLength);
  void Execute();
  void HandleOKCallback();

private:
  Jukebox* jukebox;
  int seed;
  int topN;
  int guessLength;
  std::vector<similarity_knn> results;
};

#endif