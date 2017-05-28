/*
  dummycli.js : connect to tcp server and calc stats
  */

var net = require('net');


var total_recv_bytes=0;
var total_recv_count=0;

var num_cli = parseInt( process.argv[2] );
var host = process.argv[3];

if( num_cli == NaN || !host ) {
    console.log("Usage: node dummycli.js NUM_CLI SERVER_ADDR");
    process.exit(1);
}
console.log("num_cli:", num_cli, "host:", host );

function startNewClient(host) {
    var client = new net.Socket();
    client.connect(22222, host, function() {
	    console.log("Connected");
    });

    client.on('data', function(data) {
        total_recv_count ++;
        total_recv_bytes += data.length;	
    });

    client.on('close', function() {
	    console.log('Connection closed');
    });
}





var last_tot_bytes=0;
var last_tot_cnt=0;

var cli_started=0;

setInterval(function() {
    var bytes_per_sec = total_recv_bytes - last_tot_bytes;
    var count_per_sec = total_recv_count - last_tot_cnt;
    
    console.log("num_cli:", num_cli, "cli_started:", cli_started, "total recv bytes:", total_recv_bytes, " count:", total_recv_count,  "bytes/sec:", bytes_per_sec, "count/sec:", count_per_sec );
    last_tot_bytes = total_recv_bytes;
    last_tot_cnt = total_recv_count;
}, 1000 );

setInterval(function() {
    if( cli_started < num_cli) {
        startNewClient(host);
        cli_started++;
    }
},100);

