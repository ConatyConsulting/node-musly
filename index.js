var musly = require('bindings')('binding.node');

musly.debug(5);

console.log("VERSION");
console.log(musly.version() + "\n");

console.log("METHODS");
console.log(musly.listMethods() + "\n");

console.log("DECODERS");
console.log(musly.listDecoders() + "\n");

var juke = musly.jukebox();
console.log("ABOUT")
console.log(juke.about() + "\n");

juke.analyzeAudiofile("test.wav", 30, 0, function (err, track) {
  if (err) {
      throw err;
  } else {
      var buf = track.toBuffer();
      console.log("Exported track to Buffer of length " + buf.length);

      var track2 = juke.createTrack(buf);

      var tracks = [track, track2];
      tracks.forEach(function (t) {
          console.log(t.toString());
      });

      juke.addTracks(tracks, function (err, ids) {
          console.log("Added " + ids.length + " tracks to Jukebox: [" + ids.join(',') + "]");
      })
  }
});