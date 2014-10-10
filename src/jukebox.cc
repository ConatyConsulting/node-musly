#include "jukebox.h"

#include "worker.h"

#include <iostream>
#include <algorithm>


Jukebox::Jukebox(const char* method, const char* decoder) {
  mj = musly_jukebox_poweron(method,decoder);
};

Jukebox::~Jukebox() {
  musly_jukebox_poweroff(mj);
};

v8::Persistent<v8::Function> Jukebox::constructor;


int Jukebox::analyzeAudiofile(const char* audiofile,float excerpt_length,float excerpt_start,unsigned char** buffer) {
  std::cout << "Jukebox::analyzeAudiofile - " << audiofile << "\n";
  musly_track* mt = musly_track_alloc(mj);
  int ret = musly_track_analyze_audiofile(mj, audiofile, excerpt_length, excerpt_start, mt);
  std::cout << "Jukebox::analyzeAudiofile - " << ret << "\n";
  if (ret != -1) {
    int buffersize = musly_track_binsize(mj);
    *buffer = new unsigned char[buffersize];
    musly_track_tobin(mj, mt, *buffer);
  }
  musly_track_free(mt);
  return ret;
}

bool Jukebox::addTrack(int id, unsigned char* buffer) {
  musly_track* mt = musly_track_alloc(mj);
  size_t bytesRead = musly_track_frombin(mj, buffer, mt);
  if (bytesRead != musly_track_binsize(mj)) {
    musly_track_free(mt);
    return false;
  }
  tracks.push_back(mt);
  trackids.push_back(id);
  return true;
}

bool Jukebox::ready(int sampleSize) {
  int ret;
  if (trackids.size() <= sampleSize) {
    // use all available tracks
    ret = musly_jukebox_setmusicstyle(mj, tracks.data(),
        tracks.size());
    std::cout << "Jukebox::ready setmusicstyle full - " << ret << "\n";
  }
  else {
    // use a random subset of tracks
    std::vector<musly_track*> tracks2(tracks);
    std::random_shuffle(tracks2.begin(), tracks2.end());
    ret = musly_jukebox_setmusicstyle(mj, tracks2.data(), sampleSize);
    std::cout << "Jukebox::ready setmusicstyle subset - " << ret << "\n";
  }
  if (ret != 0) {
    return false;
  }

  // add the tracks to the jukebox
  ret = musly_jukebox_addtracks(mj, tracks.data(), trackids.data(),
      tracks.size(), 0);
  std::cout << "Jukebox::ready addtracks - " << ret << "\n";
  if (ret != 0) {
    return false;
  }

  return true;
}

std::vector<similarity_knn> Jukebox::recommend(musly_trackid seed, int topN, int guessLength) {

  std::cout << "Jukebox::recommend\n";

  std::vector<musly_trackid> guess_ids;
  if (guessLength > 0) {
    guess_ids.resize(guessLength);
    guessLength = musly_jukebox_guessneighbors(mj, seed, guess_ids.data(), guessLength);
  }

  std::vector<float> similarities;
  std::vector<similarity_knn> knn_sim;
  std::vector<musly_trackid>* ids;

  int ret;
  if (guessLength <= 0) {
    //Full Similarity
    std::cout << "Jukebox::recommend Performing Full Similarity on " << trackids.size() << " tracks\n";
    similarities.resize(tracks.size());
    //NOTE: Assumes tracks and trackids are in consecutive order...
    ret = musly_jukebox_similarity(mj, tracks[seed], seed, tracks.data(), trackids.data(), trackids.size(), similarities.data());
    std::cout << "Jukebox::recommend Full Similarity return - " << ret << "\n";
    ids = &trackids;
  } else {
    //Similarity over filtered
    std::vector<musly_track*> guess_tracks(guessLength);
    for (int i = 0; i < guessLength; i++) {
      guess_tracks[i] = tracks[guess_ids[i]];
    }
    similarities.resize(guess_ids.size());
    ret = musly_jukebox_similarity(mj, tracks[seed], seed, guess_tracks.data(), guess_ids.data(), guess_ids.size(), similarities.data());
    ids = &guess_ids;
  }

  std:: cout << "Similarity Size: " << similarities.size() << "\n";
  for (int j=0; j < similarities.size(); j++) {
    std::cout << "Similarity: " << similarities[j] << "\n";
  }

  if (ret != 0) {
    return knn_sim;
  } else {
    for (int i = 0; i < (int) ids->size(); i++) {

      musly_trackid curid = ids->at(i);
      std::cout << "Jukebox::recommend Looping through IDs - " << curid << "\n";

      // skip self
      if (seed == curid) {
        continue;
      }

      // artist filter
      /*if ((artists.size() > 0) && (artists[seed] == artists[curid])) {
        continue;
      }*/

      std::cout << "Jukebox::recommend knn_sim.size() = " << knn_sim.size() << ", topN = " << topN << "\n";
      if ((int) knn_sim.size() < topN) {
        knn_sim.push_back(std::make_pair(curid, similarities[i]));
        std::push_heap(knn_sim.begin(), knn_sim.end(), similarity_comp());

        // if the neighbors are already filled && our distance is smaller
        // than the maximum in the heap, update the heap
      } else if (similarities[i] < knn_sim.front().second) {
        std::pop_heap(knn_sim.begin(), knn_sim.end(), similarity_comp());
        knn_sim.back() = std::make_pair(curid, similarities[i]);
        std::push_heap(knn_sim.begin(), knn_sim.end(), similarity_comp());
      }

      std::cout << "Jukebox::recommend Finished Loop through IDs - " << curid << "\n";
    }
  }

  std::sort_heap(knn_sim.begin(), knn_sim.end(), similarity_comp());
  std::cout << "Jukebox::recommend Sorting the heap " << "\n";

  return knn_sim;
}


void Jukebox::Init(v8::Handle<v8::Object> exports) {
  NanScope();

  // Prepare constructor template
  v8::Local<v8::FunctionTemplate> tpl = NanNew<v8::FunctionTemplate>(New);
  tpl->SetClassName(NanNew("Jukebox"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototype
  tpl->PrototypeTemplate()->Set(NanNew("addTrack"),
      NanNew<v8::FunctionTemplate>(AddTrack)->GetFunction());
  tpl->PrototypeTemplate()->Set(NanNew("analyzeAudiofile"),
      NanNew<v8::FunctionTemplate>(AnalyzeAudiofile)->GetFunction());
  tpl->PrototypeTemplate()->Set(NanNew("ready"),
      NanNew<v8::FunctionTemplate>(Ready)->GetFunction());
  tpl->PrototypeTemplate()->Set(NanNew("recommend"),
      NanNew<v8::FunctionTemplate>(Recommend)->GetFunction());


  NanAssignPersistent(constructor, tpl->GetFunction());
  exports->Set(NanNew("Jukebox"), tpl->GetFunction());
}

NAN_METHOD(Jukebox::New) {
  NanScope();

  if (args.IsConstructCall()) {
    const char * method = args[0]->IsString() ? *NanAsciiString(args[0]->ToString()) : NULL;
    const char * decoder = args[1]->IsString() ? *NanAsciiString(args[1]->ToString()) : NULL;

    Jukebox* obj = new Jukebox(method,decoder);
    obj->Wrap(args.This());
    NanReturnValue(args.This());
  } else {
    const int argc = 2;
    v8::Local<v8::Value> argv[argc] = { args[0], args[1] };
    v8::Local<v8::Function> cons = NanNew<v8::Function>(constructor);
    NanReturnValue(cons->NewInstance(argc, argv));
  }
}

NAN_METHOD(Jukebox::AnalyzeAudiofile) {
  NanScope();

  std::cout << "Jukebox::AnalyzeAudiofile\n";
  v8::Local<v8::Object> jukeboxHandle = args.This();
  NanAsciiString* audiofile = new NanAsciiString(args[0]);
  int excerpt_length = args[1]->Int32Value();
  int excerpt_start = args[2]->Int32Value();
  NanCallback* callback = new NanCallback(args[3].As<v8::Function>());

  std::cout << "Jukebox::AnalyzeAudiofile Starting Worker\n";
  NanAsyncQueueWorker(new AnalyzeAudioWorker(callback, jukeboxHandle, audiofile, excerpt_length, excerpt_start));
  NanReturnUndefined();
}

NAN_METHOD(Jukebox::AddTrack) {
  NanScope();
  
  if (args.Length() != 2) {
    NanThrowError("Invalid number of arguments");
  }

  if (!args[0]->IsInt32()) {
    NanThrowTypeError("First argument should be an integer");
  }

  if (!args[1]->IsObject()) {
    NanThrowTypeError("Second argument should be a Buffer");
  }

  Jukebox* obj = ObjectWrap::Unwrap<Jukebox>(args.This());
  v8::Local<v8::Object> jsBuffer = args[1].As<v8::Object>();
  size_t bufferLength = node::Buffer::Length(jsBuffer);
  int id = args[0]->Int32Value();
  
  if (bufferLength != obj->trackBinSize()) {
    NanThrowError("Track binary size is not valid");
  }

  obj->addTrack(id, (unsigned char*)node::Buffer::Data(jsBuffer));

  NanReturnUndefined();
}

NAN_METHOD(Jukebox::Ready) {
  NanScope();

  NanCallback* callback;
  int sampleSize = 1000;
  v8::Local<v8::Object> jukeboxHandle = args.This();

  if (args.Length() == 2) {
    callback = new NanCallback(args[1].As<v8::Function>());
    if (!args[0]->IsInt32()) {
      NanThrowTypeError("First argument should be an integer");
    }
    sampleSize = args[0]->Int32Value();
  }
  else {
    callback = new NanCallback(args[0].As<v8::Function>());
  }

  NanAsyncQueueWorker(new ReadyWorker(callback, jukeboxHandle,sampleSize));

  NanReturnUndefined();
}

NAN_METHOD(Jukebox::Recommend) {
  NanScope();

  NanCallback* callback;
  musly_trackid seed;
  int topN;
  int guessLength = 0;

  v8::Local<v8::Object> jukeboxHandle = args.This();

  if (args.Length() < 3 || args.Length() > 4) {
    NanThrowError("Invalid number of arguments");
  }

  if (!args[0]->IsInt32()) {
    NanThrowTypeError("First argument should be an integer");
  }
  seed = args[0]->Int32Value();

  if (!args[1]->IsInt32()) {
    NanThrowTypeError("Second argument should be an integer");
  }
  topN = args[1]->Int32Value();

  if (args.Length() == 4) {
    callback = new NanCallback(args[3].As<v8::Function>());
    if (!args[0]->IsInt32()) {
      NanThrowTypeError("First argument should be an integer");
    }
    guessLength = args[2]->Int32Value();
  } else {
    callback = new NanCallback(args[2].As<v8::Function>());
  }

  NanAsyncQueueWorker(new RecommendWorker(callback,jukeboxHandle,seed,topN,guessLength));

  NanReturnUndefined();
}

