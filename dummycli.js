/*
  dummycli.js : connect to tcp server and calc stats
  */

var net = require('net');

var cnt=0;
var total_recv_bytes=0;
var total_recv_count=0;

var client = new net.Socket();
client.connect(22222, '127.0.0.1', function() {
	console.log('Connected. cnt:',cnt);
});

client.on('data', function(data) {
    total_recv_count ++;
    total_recv_bytes += data.length;	
});

client.on('close', function() {
	console.log('Connection closed');
});

var last_tot_bytes=0;
var last_tot_cnt=0;

setInterval(function() {
    var bytes_per_sec = total_recv_bytes - last_tot_bytes;
    var count_per_sec = total_recv_count - last_tot_cnt;
    
    console.log("total recv bytes:", total_recv_bytes, " count:", total_recv_count,  "bytes/sec:", bytes_per_sec, "count/sec:", count_per_sec );
    last_tot_bytes = total_recv_bytes;
    last_tot_cnt = total_recv_count;
    
}, 1000 );