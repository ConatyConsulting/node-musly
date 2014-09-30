#include <musly/musly.h>
#include <nan.h>
#include "jukebox.h"

using namespace v8;

NAN_METHOD(ListMethods) {
  NanScope();
  NanReturnValue(String::New(musly_jukebox_listmethods()));
}

NAN_METHOD(CreateJukebox) {
  NanScope();
  NanReturnValue(Jukebox::NewInstance(args[0]));
}

void InitAll(Handle<Object> exports) {
  
  Jukebox::Init();
  
  //"Global" Methods
  exports->Set(NanNew<String>("listMethods"), FunctionTemplate::New(ListMethods)->GetFunction());
  
  //Jukebox
  exports->Set(NanNew<String>("jukebox"), FunctionTemplate::New(CreateJukebox)->GetFunction());
}

NODE_MODULE(binding, InitAll)