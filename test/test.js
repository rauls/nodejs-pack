#!/usr/bin/node

var pack = require( "hipack" );


function SimpleTest()
{
  console.log("Simple Text Sample : Converting 3 VARs to RAW\nCalling pack.pack( 'VVC', 65,66,67 )" );
  var buf = pack.pack( "VVC", 65,66,67 );
  
  console.log( "Result Buffer : " );
  console.dir( (buf) );

  console.log("\nSimple Text Sample : Converting Array of 3 VARs \nCalling pack.pack( 'VVC', [65,66,67] )" );
  var buf = pack.pack( "VVC", [ 65,66,67 ] );
  console.log( "Result Buffer : " );
  console.dir( (buf) );
	
  console.log("\nUse my Buffer Sample : Converting Array of 3 VARs \nCalling pack.pack( new Buffer(32), 'VVC', [65,66,67] )" );
  buf = new Buffer(32);
  pack.pack( buf, "VVC", [ 65,66,67 ] );
  console.log( "Result Buffer : " );
  console.dir( (buf) );
}

SimpleTest();
