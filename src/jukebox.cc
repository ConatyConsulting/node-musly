#include <nan.h>
#include <musly/musly.h>
#include "jukebox.h"


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
      NanNew<FunctionTemplate>(About)->GetFunction());

  NanAssignPersistent(constructor, tpl->GetFunction());
}

NAN_METHOD(Jukebox::New) {
  NanScope();

  Jukebox* obj = new Jukebox(0,0);
  //obj->counter_ = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
  obj->Wrap(args.This());

  NanReturnValue(args.This());
}

NAN_METHOD(Jukebox::About) {
  NanScope();
  
  Jukebox* obj = ObjectWrap::Unwrap<Jukebox>(args.This());
  
  NanReturnValue(String::New(musly_jukebox_aboutmethod(obj->mb_)));
}

Local<Object> Jukebox::NewInstance(Local<Value> arg) {
  NanEscapableScope();

  const unsigned argc = 1;
  Local<Value> argv[argc] = { arg };
  Local<Function> cons = NanNew<Function>(constructor);
  Local<Object> instance = cons->NewInstance(argc, argv);

  return NanEscapeScope(instance);
}