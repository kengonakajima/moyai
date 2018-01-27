// demo2d server main

var moyaisv=require("./moyaisv.js");
var net=require("net");

var server = net.createServer(function(conn) {
    console.log("newconnection");
});
server.listen(22222);