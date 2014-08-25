#!/usr/bin/node

var pack = require( "hipack" );

var str = process.ARGV[2];

var pos = 3;
var Params = [];
while( pos < process.ARGC )
{
	Params.push( process.ARGV[ pos ] );
	pos++;
}

var buf = pack.pack( "VVC", 65,66,67 );

console.log( "Buffers are : " );
console.dir( (buf) );

function SimpleTest()
{
  console.log("Converting VARs to RAW\nCalling pack.pack( 'VVC', 65,66,67 )" );
  var buf = pack.pack( "VVC", 65,66,67 );
  
  console.log( "Buffers are : " );
  console.dir( (buf) );
  
  var test = new Buffer(8);
  console.dir( test );
}


// Loop overhead is 50ms for 1m items.
// Calling pack.N is a 500ms overhead doing 'nothing'
function SpeedTest() 
{
  var raw;
    
  var t1 = new Date();
  var tot = 1e6;
  for(i=0;i<tot;i++){
     //pack.setdebug(0);
     raw = pack.pack( "CCCCVVVV", 65,66,66,65,12123,123123,123123,123123 );
  }
  var t2 = new Date();
  var dur = t2-t1;
        
  console.log( "Time taken for 1M pack() = %d, (%d/s) ", dur, (1000*tot)/dur );
                         
}

SimpleTest();
SpeedTest();
