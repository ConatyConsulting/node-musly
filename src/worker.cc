#include "worker.h"

#include <iostream>

AnalyzeAudioWorker::AnalyzeAudioWorker(NanCallback* callback,
    v8::Local<v8::Object> &jukeboxHandle,
    NanAsciiString* audiofile,
    float excerpt_length,
    float excerpt_start)
  : NanAsyncWorker(callback),  excerpt_length(excerpt_length), excerpt_start(excerpt_start), audiofile(audiofile) {

  SaveToPersistent("jukebox", jukeboxHandle);
  jukebox = node::ObjectWrap::Unwrap<Jukebox>(jukeboxHandle);
}

AnalyzeAudioWorker::~AnalyzeAudioWorker() {
  delete audiofile;
}

void AnalyzeAudioWorker::Execute() {
  std::cout << "AnalyzeAudioWorker::Execute";
  int ret = jukebox->analyzeAudiofile(**audiofile,excerpt_length,excerpt_start,&trackBuffer);
  if (ret == -1) {
    SetErrorMessage("Error in musly_track_analyze_audiofile");
  }
}
    
void AnalyzeAudioWorker::HandleOKCallback() {
  NanScope();

  int length = jukebox->trackBinSize();
  node::Buffer *slowBuffer = node::Buffer::New(length);
  memcpy(node::Buffer::Data(slowBuffer), trackBuffer, length);

  v8::Local<v8::Object> globalObj = NanGetCurrentContext()->Global();
  v8::Local<v8::Function> bufferConstructor = globalObj->Get(NanNew("Buffer")).As<v8::Function>();
  v8::Handle<v8::Value> constructorArgs[3] = { slowBuffer->handle_, NanNew(length), NanNew(0) };
  v8::Local<v8::Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);

  v8::Local<v8::Value> argv[] = {
    NanNull(),
    actualBuffer
  };
  callback->Call(2, argv);
}

ReadyWorker::ReadyWorker(NanCallback* callback,
    v8::Local<v8::Object> &jukeboxHandle,
    int sampleSize)
    : NanAsyncWorker(callback),  sampleSize(sampleSize) {

  SaveToPersistent("jukebox", jukeboxHandle);
  jukebox = node::ObjectWrap::Unwrap<Jukebox>(jukeboxHandle);
}

void ReadyWorker::Execute() {
  if (!jukebox->ready(sampleSize)) {
    SetErrorMessage("Error in Jukebox::ready");
  }
}

void ReadyWorker::HandleOKCallback() {
  v8::Local<v8::Value> argv[] = {
      NanNull()
  };

  callback->Call(1,argv);
}

RecommendWorker::RecommendWorker(NanCallback* callback,
    v8::Local<v8::Object> &jukeboxHandle,
    int seed,int topN,int guessLength)
: NanAsyncWorker(callback), seed(seed), topN(topN), guessLength(guessLength) {

  SaveToPersistent("jukebox", jukeboxHandle);
  jukebox = node::ObjectWrap::Unwrap<Jukebox>(jukeboxHandle);

  std::cout << "RecommendWorker::RecommendWorker seed = " << seed << ", topN = " << topN << ", guessLength = " << guessLength << "\n";
}

void RecommendWorker::Execute() {
  results = jukebox->recommend(seed, topN, guessLength);
  if (results.size() == 0) {
    SetErrorMessage("Error in Jukebox::recommend. Empty results");
  }
}

void RecommendWorker::HandleOKCallback() {

  v8::Local<v8::Array> arr = NanNew<v8::Array>(results.size());
  for (int i = 0; i < (int) results.size(); i++) {
    v8::Local<v8::Array> pair = NanNew<v8::Array>();
    pair->Set(0,NanNew<v8::Integer>(results[i].first));
    pair->Set(1,NanNew<v8::Number>(results[i].second));
    arr->Set(i,pair);
    std::cout << "RecommendWorker::HandleOKCallback - Set Pair (" << results[i].first << "," << results[i].second << ")\n";
  }
  v8::Local<v8::Value> argv[] = {
      NanNull(),
      arr
  };

  callback->Call(2,argv);
}