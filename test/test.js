#!/usr/bin/node

var pack = require( "hipack" );
var assert = require ('assert');

function UnpackFloatTest()
{
  var data = require ('./floats.json');
  data.forEach (function (d) {
    var tolerance = 1e-6;

    var p = pack.pack ('f', d);
    var r = (pack.unpack ('fresult', p)).result;

    var min = Math.min (Math.abs (d), Math.abs (r));
    if (min === 0) {
      /* avoid NaNs */
      min = 1;
    }
    var diff = Math.abs (d - r);
    assert (diff / min < tolerance, d + ' !== ' + r + ' (' + diff / min + ')');
  });
}

function SimpleTest()
{
  console.log("Simple Text Sample : Converting 3 VARs to RAW\nCalling pack.pack( 'VVC', 65,66,67 )" );
  var buf = pack.pack( "VVC", 65,66,67 );
  
  console.log( "Result Buffer : " );
  console.dir( (buf) );

  console.log("\nSimple Text Sample : Converting Array of 3 VARs \nCalling pack.pack( 'VVC', [65,66,67] )" );
  buf = pack.pack( "VVC", [ 65,66,67 ] );
  console.log( "Result Buffer : " );
  console.dir( (buf) );
	
  console.log("\nUse my Buffer Sample : Converting Array of 3 VARs \nCalling pack.pack( new Buffer(32), 'VVC', [65,66,67] )" );
  buf = new Buffer(32);
  pack.pack( buf, "VVC", [ 65,66,67 ] );
  console.log( "Result Buffer : " );
  console.dir( (buf) );
}

SimpleTest();
UnpackFloatTest();
