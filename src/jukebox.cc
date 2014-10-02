#include <nan.h>
#include <musly/musly.h>
#include "jukebox.h"
#include "worker.h"
#include "track.h"

using namespace v8;

Jukebox::Jukebox(const char* method, const char* decoder) {
  mb_ = musly_jukebox_poweron(method,decoder);
};

Jukebox::~Jukebox() {
  musly_jukebox_poweroff(mb_);
};

Persistent<Function> Jukebox::constructor;

void Jukebox::Init() {
  NanScope();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
  tpl->SetClassName(NanNew("Jukebox"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototype
  tpl->PrototypeTemplate()->Set(NanNew("about"),
      NanNew<FunctionTemplate>(AboutMethod)->GetFunction());
  tpl->PrototypeTemplate()->Set(NanNew("analyzeAudiofile"),
      NanNew<FunctionTemplate>(AnalyzeAudiofile)->GetFunction());
  tpl->PrototypeTemplate()->Set(NanNew("createTrack"),
      NanNew<FunctionTemplate>(CreateTrack)->GetFunction());
  tpl->PrototypeTemplate()->Set(NanNew("addTracks"),
      NanNew<FunctionTemplate>(AddTracks)->GetFunction());

  NanAssignPersistent(constructor, tpl->GetFunction());
}

Local<Object> Jukebox::NewInstance(Local<Value> arg) {
  NanEscapableScope();

  const unsigned argc = 1;
  Local<Value> argv[argc] = { arg };
  Local<Function> cons = NanNew<Function>(constructor);
  Local<Object> instance = cons->NewInstance(argc, argv);

  return NanEscapeScope(instance);
}

NAN_METHOD(Jukebox::New) {
  NanScope();

  Jukebox* obj = new Jukebox(0,0);
  //obj->counter_ = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
  obj->Wrap(args.This());

  NanReturnValue(args.This());
}

NAN_METHOD(Jukebox::AboutMethod) {
  NanScope();
  
  Jukebox* obj = ObjectWrap::Unwrap<Jukebox>(args.This());
  
  NanReturnValue(String::New(musly_jukebox_aboutmethod(obj->mb_)));
}

NAN_METHOD(Jukebox::AnalyzeAudiofile) {
  NanScope();
  
  v8::Local<v8::Object> jukeboxHandle = args.This();
  NanAsciiString* audiofile = new NanAsciiString(args[0]);
  int excerpt_length = args[1]->Int32Value();
  int excerpt_start = args[2]->Int32Value();
  NanCallback* callback = new NanCallback(args[3].As<Function>());
  
  NanAsyncQueueWorker(new AnalyzeAudioWorker(callback, jukeboxHandle, audiofile, excerpt_length, excerpt_start));
  NanReturnUndefined();
}

NAN_METHOD(Jukebox::AddTracks) {
  NanScope();

  if (args.Length() != 2) {
    NanThrowError("Invalid number of arguments");
  }

  if (!args[0]->IsArray()) {
    NanThrowTypeError("First argument should be an Array");
  }

  Local<Array> tracksArray = args[0].As<Array>();
  for (int index = 0, size = tracksArray->Length(); index < size; index++) {
    Local<Value> element = tracksArray->Get(index);
    if (!element->IsObject()) {
      NanThrowTypeError("First argument should be an Array of Tracks");
      break;
    }
  }

  v8::Local<v8::Object> jukeboxHandle = args.This();
  NanCallback* callback = new NanCallback(args[1].As<Function>());

  NanAsyncQueueWorker(new AddTracksWorker(callback,jukeboxHandle,tracksArray));
  NanReturnUndefined();
}

NAN_METHOD(Jukebox::CreateTrack) {
  NanScope();

  if (args.Length() != 1) {
    NanThrowError("Invalid number of arguments");
  }

  if (!args[0]->IsObject()) {
    NanThrowTypeError("First argument should be a Buffer");
  }

  Local<Object> jsBuffer = args[0].As<Object>();
  Jukebox* obj = ObjectWrap::Unwrap<Jukebox>(args.This());

  musly_track* track = musly_track_alloc(obj->musly());

  size_t bytesRead = musly_track_frombin(obj->musly(),(unsigned char *)Buffer::Data(jsBuffer),track);

  if (bytesRead != Buffer::Length(jsBuffer)) {
    NanThrowError("Failed musly_track_frombin");
  }

  Local<Object> jsTrack = Track::NewInstance();
  Track* trackObj = ObjectWrap::Unwrap<Track>(jsTrack);
  trackObj->musly(track);
  trackObj->jukebox(obj->musly());

  NanReturnValue(jsTrack);
}

