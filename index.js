var musly = require('bindings')('binding.node');
musly.debug(5);
console.log(musly);
var juke = new musly.Jukebox();
console.log(juke);

juke.analyzeAudiofile("test.wav", 30, 0, function (err, buffer) {
   if (err) {
       throw err;
   } else {
       console.log(buffer);
       console.log(buffer.length);
       for (var i=0; i < 5; i++) {
           juke.addTrack(i,buffer);
       }
       juke.ready(function (err) {
           if (err) {
               throw err;
           } else {
               juke.recommend(0,5,0, function (err, similarities) {
                   if (err) {
                       throw err;

                   } else {
                       console.log(similarities);
                       console.log("DONE");
                   }
               })
           }
       })
   }
});