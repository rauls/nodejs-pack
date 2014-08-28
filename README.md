## HiPack

This is a copy of PHP's pack/unpack function made into a nodejs module.
Its a 100% clone with an additional options.

* A new feature where we can populate an existing Buffer is now there, so the caller can
worry about Buffer management. Simply make the first parameter a buffer

* Also we can supply an array of values, rather than arguments, so the 2nd parameter is an array.
ie. pack( "VV", [ 23,55 ] );

* Added a Perl style format that was missing, which is format character "V" and "v"



## Install

To install HiPack, use [npm](http://github.com/rauls/nodejs-pack)

    $ npm install hipack

## Packing Bytes Test
    
To test packing bytes, run
	 $ node test/test.js
    $ node test/speedtest.js
    $ node example/packit.js CCCC 25 26 27 27
    
To use in code,
	var pack = require( 'hipack' ).pack;
	var buffer = pack( "CCCV", [ 1,2,3, 12345679 ] );



## Original Pack Documentation 

pack - Pack data into binary Buffer object

### Description 

Buffer object = pack ( [ DestBuffer ] , string format [, mixed $args [, mixed n... ]] )
Pack given arguments into a binary Buffer according to format.

The idea for this function was taken from Perl and all formatting codes work the same as in Perl. However, there are some formatting codes that are missing such as Perl's "u" format code.

Note that the distinction between signed and unsigned values only affects the function unpack(), where as function pack() gives the same result for signed and unsigned format codes.

### Parameters 

## format
The format string consists of format codes followed by an optional repeater argument. The repeater argument can be either an integer value or * for repeating to the end of the input data. For a, A, h, H the repeat count specifies how many characters of one data argument are taken, for @ it is the absolute position where to put the next data, for everything else the repeat count specifies how many data arguments are consumed and packed into the resulting binary string.

## Currently implemented formats are:

### pack() format characters

Code | Description
-----|------------
a | 	NUL-padded string
A | 	SPACE-padded string
h | 	Hex string, low nibble first
H | 	Hex string, high nibble first
c | 	signed char
C | 	unsigned char
s | 	signed short (always 16 bit, machine byte order)
S | 	unsigned short (always 16 bit, machine byte order)
n | 	unsigned short (always 16 bit, big endian byte order)
v | 	unsigned short (always 16 bit, little endian byte order)
i | 	signed integer (machine dependent size and byte order)
I | 	unsigned integer (machine dependent size and byte order)
l | 	signed long (always 32 bit, machine byte order)
L | 	unsigned long (always 32 bit, machine byte order)
N | 	unsigned long (always 32 bit, big endian byte order)
V | 	unsigned long (always 32 bit, little endian byte order)
f | 	float (machine dependent size and representation)
d | 	double (machine dependent size and representation)
x | 	NUL byte
X | 	Back up one byte
Z | 	NUL-padded string (new in PHP 5.5)
@ | 	NUL-fill to absolute position

## Return Values

Returns a Buffer containing data.



# unpack

## unpack - Unpack data from a Buffer

### Description

Object unpack ( String format , Buffer data, [ Boolean PerlFormat ] )
Unpacks from a binary string into an array according to the given format.

The unpacked data is stored in an associative array. To accomplish this you have to name the different format codes and separate them by a slash /. If a repeater argument is present, then each of the array keys will have a sequence number behind the given name.

Example: "c2chars/nints" will return an object with fields
Object = { chars1: 0, chars2: 0, ints: 0 }.

## Parameters 

format
See pack() for an explanation of the format codes.

data
The packed data.

PerlFormat
If this parameter exists and is true, we will ignore the PHP style names, and use just the variables index

Example: "c2n" will return an object with fields
Object = { 1: 0, 2: 0, 3: 0 }

## Return Values

Returns an associative array containing unpacked elements of binary string.
