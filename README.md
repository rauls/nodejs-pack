## HiPack
This is a copy of PHP's pack function made into a nodejs module.
Its a 100% clone with no new features or changed features, yet.

The only speed issue is that doing a Buffer::New() takes a long time.

A new feature where we can populate an existing Buffer is now there, so the caller can
worry about Buffer management. This will be done as the first paramater.


**packnode minifies, obfuscates and/or encrypts node modules for private use.**

To install HiPack, use [npm](http://github.com/rauls/nodejs-pack)

    $ npm install hipack

## Packing Bytes Test
    
To test packing bytes, run

    $ node packit.js CCCC 25 26 27 27
    
