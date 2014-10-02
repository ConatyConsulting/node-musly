#include "track.h"

using namespace v8;

Track::Track() {};

Track::~Track() {
  musly_track_free(track_);
};

Persistent<Function> Track::constructor;

void Track::Init() {
  NanScope();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
  tpl->SetClassName(NanNew("Track"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototype
  tpl->PrototypeTemplate()->Set(NanNew("toString"),
      NanNew<FunctionTemplate>(ToString)->GetFunction());
  tpl->PrototypeTemplate()->Set(NanNew("toBuffer"),
      NanNew<FunctionTemplate>(ToBuffer)->GetFunction());

  NanAssignPersistent(constructor, tpl->GetFunction());
}

Local<Object> Track::NewInstance() {
  NanEscapableScope();

  const unsigned argc = 0;
  Local<Value> argv[argc] = {  };
  Local<Function> cons = NanNew<Function>(constructor);
  Local<Object> instance = cons->NewInstance(argc, argv);

  return NanEscapeScope(instance);
}

NAN_METHOD(Track::New) {
  NanScope();

  Track* obj = new Track();
  //obj->counter_ = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
  obj->Wrap(args.This());

  NanReturnValue(args.This());
}

NAN_METHOD(Track::ToString) {
  NanScope();

  Track* obj = ObjectWrap::Unwrap<Track>(args.This());
  const char * str = musly_track_tostr(obj->jukebox(),obj->musly());

  NanReturnValue(NanNew<String>(str));
}

NAN_METHOD(Track::ToBuffer) {
  NanScope();

  Track* obj = ObjectWrap::Unwrap<Track>(args.This());

  int datasize = musly_track_binsize(obj->jukebox());
  unsigned char* data = new unsigned char[datasize];

  int serialized = musly_track_tobin(obj->jukebox(),obj->musly(),data);

  if (serialized != datasize) {
    NanThrowError("Failed musly_track_tobin");
  }

  node::Buffer *slowBuffer = node::Buffer::New(datasize);
  memcpy(node::Buffer::Data(slowBuffer), data, datasize);
  Local<Object> globalObj = NanGetCurrentContext()->Global();
  Local<Function> bufferConstructor =
      Local<Function>::Cast(globalObj->Get(String::New("Buffer")));
  Handle<Value> constructorArgs[3] = { slowBuffer->handle_,
      Integer::New(datasize),
      Integer::New(0) };
  Local<Object> actualBuffer =
      bufferConstructor->NewInstance(3, constructorArgs);

  NanReturnValue(actualBuffer);
}