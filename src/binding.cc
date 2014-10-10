#include <musly/musly.h>
#include <nan.h>
#include <v8.h>
#include "jukebox.h"
//#include "track.h"

using namespace v8;

NAN_METHOD(Version) {
  NanScope();
  NanReturnValue(NanNew<String>(musly_version()));
}

NAN_METHOD(ListMethods) {
  NanScope();
  NanReturnValue(NanNew<String>(musly_jukebox_listmethods()));
}

NAN_METHOD(ListDecoders) {
  NanScope();
  NanReturnValue(NanNew<String>(musly_jukebox_listdecoders()));
}

/*NAN_METHOD(CreateJukebox) {
  NanScope();
  NanReturnValue(Jukebox::NewInstance(args[0]));
}*/

/*NAN_METHOD(CreateTrack) {
  NanScope();
  NanReturnValue(Track::NewInstance());
}*/

NAN_METHOD(Debug) {
  NanScope();
  int level = args[0]->Int32Value();
  musly_debug(level);
  NanReturnUndefined();
}

void InitAll(Handle<Object> exports) {
  
  //Track::Init();
  Jukebox::Init(exports);
  
  //"Global" Methods
  exports->Set(NanNew<String>("version"), NanNew<FunctionTemplate>(Version)->GetFunction());
  exports->Set(NanNew<String>("debug"), NanNew<FunctionTemplate>(Debug)->GetFunction());
  exports->Set(NanNew<String>("listMethods"), NanNew<FunctionTemplate>(ListMethods)->GetFunction());
  exports->Set(NanNew<String>("listDecoders"), NanNew<FunctionTemplate>(ListDecoders)->GetFunction());
  
  //Jukebox
  //exports->Set(NanNew<String>("jukebox"), FunctionTemplate::New(CreateJukebox)->GetFunction());
  //exports->Set(NanNew<String>("track"), FunctionTemplate::New(CreateTrack)->GetFunction());
}

NODE_MODULE(binding, InitAll)