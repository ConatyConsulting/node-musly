#ifndef JUKEBOX_H
#define JUKEBOX_H

#include <nan.h>
#include <musly/musly.h>
#include <vector>

typedef std::pair<int, float> similarity_knn;
struct similarity_comp {
  bool
  operator()(
      similarity_knn const& lhs,
      similarity_knn const& rhs) {
    return lhs.second < rhs.second;
  }
};

class Jukebox : public node::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> exports);
  static v8::Local<v8::Object> NewInstance(v8::Local<v8::Value> arg);

  size_t trackBinSize() { return musly_track_binsize(mj); };
  bool addTrack(int id, unsigned char* buffer);
  int analyzeAudiofile(const char* audiofile,float excerpt_length,float excerpt_start,unsigned char** buffer);
  bool ready(int sampleSize);
  std::vector<similarity_knn> recommend(musly_trackid seed, int topN, int guessLength);
  
private:
  Jukebox(const char* method, const char* decoder);
  ~Jukebox();

  musly_jukebox* mj;
  std::vector<musly_track*> tracks;
  std::vector<musly_trackid> trackids;

  static v8::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);
  static NAN_METHOD(AnalyzeAudiofile);
  static NAN_METHOD(AddTrack);
  static NAN_METHOD(Ready);
  static NAN_METHOD(Recommend);
};

#endif