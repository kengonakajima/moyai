var path=require("path");
var fs=require("fs")
var bytes = fs.readFileSync(process.argv[2]);

var name = path.basename(process.argv[2]).replace(".","_");

var buf = Buffer(bytes)

console.log("var ", name, " = new Uint8Array( [");
var ind=0;
for(var line=0;;line++) {
    var line=[];
    for(var col=0;col<32;col++) {
        line.push( buf[ind]);
        ind++;
        if(ind==buf.length)break;
    }
    console.log(line.join(),",");
    if(ind==buf.length)break;    
}
console.log("]);")
