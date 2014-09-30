var musly = require('bindings')('binding.node');

console.log("METHODS");
console.log(musly.listMethods() + "\n");
var juke = musly.jukebox();
console.log("ABOUT")
console.log(juke.about() + "\n");
juke = null;