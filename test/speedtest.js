#!/usr/bin/node

var pack = require( "hipack" );


// Loop overhead is 50ms for 1m items.
// Calling pack.N is a 500ms overhead doing 'nothing'
function SpeedTest() 
{
  var raw;
  console.log("Performing a speed test, 1M packs()" );
    
  var t1 = new Date();
  var tot = 1e6;
  for(i=0;i<tot;i++){
     //pack.setdebug(0);
     raw = pack.pack( "CCCCVVVV", 65,66,66,65,12123,1231234,12312345,123123456 );
  }
  var t2 = new Date();
  var dur = t2-t1;
        
  console.log( "Time taken for 1M pack() = %d ms, (%d packs/sec) ", dur, parseInt((1000*tot)/dur) );
                         
}

SpeedTest();
