/*
 * The c++-module.
 */
var binding = require('../build/default/pack');


/* pack() idea stolen from Perl (implemented formats behave the same as there)
 * Implemented formats are A, a, h, H, c, C, s, S, i, I, l, L, n, N, f, d, x, X, @.
 */
exports.pack = binding.pack;

function call_pack() {
  var arglist;
  for( var i = 0; i < arguments.length; i++ ) {
    console.log( "arg #%d, is %s of %s", i, typeof(arguments[i]), arguments[i] );
    if( arglist ) { arglist += ','; }
    arglist += arguments[i];
  }
  
  if( typeof( arguments[1] ) == 'object' )
  {
    console.log( "calling with 2 args" );
    return binding.pack( arguments[0], arguments[1] );
  }
  
  var arg = arguments[0];   
  delete arguments[0];                                                             
  console.log( "calling with %s %s", arg, arguments );
  return binding.pack( arg, arguments );
};


/* unpack() is based on PHP's unpack(), but is modified a bit from there.
 * Rather than depending on error-prone ordered lists or syntactically
 * unpleasant pass-by-reference, we return an object with named paramters 
 * (like *_fetch_object()). Syntax is "f[repeat]name/...", where "f" is the
 * formatter char (like pack()), "[repeat]" is the optional repeater argument,
 * and "name" is the name of the variable to use.
 * Example: "c2chars/nints" will return an object with fields
 * chars1, chars2, and ints.
 * Numeric pack types will return numbers, a and A will return strings,
 * f and d will return doubles.
 * Implemented formats are A, a, h, H, c, C, s, S, i, I, l, L, n, N, f, d, x, X, @.
 */
exports.unpack = binding.unpack;
exports.setdebug = binding.setdebug;



