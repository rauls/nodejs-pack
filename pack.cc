#include <v8.h>

#include <node.h>
#include <node_object_wrap.h>
#include <node_buffer.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/param.h>
//#include "ext/standard/head.h"
//#include "NODEJS_string.h"

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <assert.h>
#include "pack.h"


using namespace v8;
using namespace node;


#define THROW_ERROR(bufname, x) { \
   char bufname[256] = {0}; \
   sprintf x; \
   return ThrowException(Exception::Error (String::New(bufname))); \
   }

#define THROW_IF_NOT(condition, text) if (!(condition)) { \
      return ThrowException(Exception::Error (String::New(text))); \
    }

#define THROW_IF_NOT_A(condition, bufname, x) if (!(condition)) { \
   char bufname[256] = {0}; \
   sprintf x; \
   return ThrowException(Exception::Error (String::New(bufname))); \
   }


#define TARGET			(v8::Handle<v8::Object> target)

#define NODEJS_ERROR( strfmt, arg ) 	THROW_ERROR( _buf, (_buf, (const char*)strfmt, arg) )


#define INC_OUTPUTPOS(a,b) \
	if ((a) < 0 || ((INT_MAX - outputpos)/((int)b)) < (a)) { \
		NODEJS_ERROR( "Type %c: integer overflow in format string", code); \
		return Undefined(); \
	} \
	outputpos += (a)*(b);

#define safe_emalloc(len,objsize,setwith)	malloc( len*objsize )

#define ToCString(value)   (*value ? *value : "<string conversion failed>")

inline double ToDouble(v8::Local<v8::Value> value)
{
	return static_cast<double>(value->NumberValue());
}
inline float ToFloat(v8::Local<v8::Value> value)
{
	return static_cast<float>(value->NumberValue());
}



char *BufferData(node::Buffer *b) {
    return node::Buffer::Data(b->handle_);
}
size_t BufferLength(node::Buffer *b) {
    return node::Buffer::Length(b->handle_);
}
char *BufferData(v8::Local<v8::Object> buf_obj) {
    v8::HandleScope scope;
    return node::Buffer::Data(buf_obj);
}
size_t BufferLength(v8::Local<v8::Object> buf_obj) {
    v8::HandleScope scope;
    return node::Buffer::Length(buf_obj);
}

static int debug_mode;
/* Whether machine is little endian */
static char machine_little_endian;

/* Mapping of byte from char (8bit) to long for machine endian */
static int byte_map[1];

/* Mappings of bytes from int (machine dependant) to int for machine endian */
static int int_map[sizeof(int)];

/* Mappings of bytes from shorts (16bit) for all endian environments */
static int machine_endian_short_map[2];
static int big_endian_short_map[2];
static int little_endian_short_map[2];

/* Mappings of bytes from longs (32bit) for all endian environments */
static int machine_endian_long_map[4];
static int big_endian_long_map[4];
static int little_endian_long_map[4];


class HiPack : public ObjectWrap {
	public:
	static void SetDebug(int n) { debug_mode = n; }
	static int IsDebug() { return debug_mode; }

	static Handle<Value> New(const Arguments& args) {
    	HandleScope scope;

        HiPack* hipack = new HiPack();
        hipack->Wrap(args.This());

        return args.This();
    }
/* {{{ NODEJS_pack
 */
static void NODEJS_pack(int32_t val, int size, int *map, char *output)
{
	int i;
	char *v;

	//convert_to_long_ex(val);
	v = (char *) &val;

	for (i = 0; i < size; i++) {
		*output++ = v[map[i]];
	}
}
/* }}} */

// We define a max size, since we want to avoid any mallocs() theres are on the stack and faster.
// I think 48 is enough for 99.99% cases. Next release we might have a init option
#define MAX_FORMAT_CODES		(48)

// We also store the output locally in the stack if its under 256 bytes in size, else we malloc it.

/* pack() idea stolen from Perl (implemented formats behave the same as there)
 * Implemented formats are A, a, h, H, c, C, s, S, i, I, l, L, n, N, f, d, x, X, V, v, @.
 */
/* {{{ proto string pack(String format, mixed arg1 [, mixed arg2 [, mixed ...]])
   Takes one or more arguments and packs them into a binary string according to the format argument 
 * If the first ARG is a Buffer(), is that as the output instead of creating its own buffer.
 */
static Handle<Value> pack(const Arguments& argv)
{
    HandleScope scope;
	int     outputpos = 0, outputsize = 0;
	int     num_args, i;
	int     currentarg = 0, firstarg = 0;
	char    *output = NULL;
	char    formatcodes[MAX_FORMAT_CODES];				// Local stored codes for faster access/ avoid mallocs
	int     formatargs[MAX_FORMAT_CODES];				// Local index store.
	int     formatcount = 0;
	int     formatlen;
	const char *format;
	bool argsFromArray = false;
	bool parentBuffer = false;
	Local<Object>	outputBuffer;
	char localOutput[256];
  
    num_args = argv.Length();
	if (argv.Length() < 2 ) {
        NODEJS_ERROR( "Not enough arguements, total args: %d", argv.Length() );
    }
	
	if (argv.Length() >= 1 ) 
	{
		currentarg = 0;
		if (argv[currentarg]->IsObject() ) 
		{
			if( IsDebug() ) printf("Using passed in buffer as the output\n");
			outputBuffer = ( argv[currentarg]->ToObject() );
			parentBuffer = true;
			currentarg = 1;
			if (!argv[currentarg]->IsString()) {
				NODEJS_ERROR( "Second argument must be a string or buffer, type of arg is %d", argv[currentarg]->IsString() );
			}
		} else
		if (!argv[currentarg]->IsString()) {
			NODEJS_ERROR( "First argument must be a string or buffer, type of arg is %d", argv[currentarg]->IsString() );
		}
	}

    //printf( "arg1 is : %s", *args[1] );
    //Handle<Object> arg2 = args[1]->ToObject();
    //Array<Object> array = args[1]->ToObject()->
    //num_args = 3;
    //Local<Value> argv[32];
    //for( int i=0;i<num_args;i++) {
    //    argv[i+1] = arg2->Get(i);
    // }

    String::Utf8Value	formatString( argv[currentarg] );
	formatlen = formatString.length();
	format = ToCString( formatString );
    
	/* We have a maximum of <formatlen> format codes to deal with */
	//formatcodes = (char*)safe_emalloc(formatlen, sizeof(*formatcodes), 0);
	//formatargs = (int*)safe_emalloc(formatlen, sizeof(*formatargs), 0);
    
    if( IsDebug()>0 ) printf("starting.. numargs=%d, formatlen=%d\n", num_args, formatlen);
	
	currentarg++;
	firstarg = currentarg;
	
	Local<Array> args;
	if( argv[currentarg]->IsArray() == true )
	{
		args = Array::Cast(*argv[currentarg]);
		printf("Args size is %d\n", args->Length() );
		num_args = args->Length();
		currentarg = firstarg = 0;
		//for( i=0;i<num_args;i++){
		//	printf("value at #%d is %d\n", i, args->Get(i)->Int32Value() );
		//}
		argsFromArray = true;
	}
#define GETARG(n)	(argsFromArray ? args->Get(n) : argv[n])
	
	
	/* Preprocess format into formatcodes and formatargs */
	for (i = 0; i < formatlen; formatcount++) {
		char code = format[i++];
		int arg = 1;
		
	    if( IsDebug()>0 ) printf("starting..format args  currentarg=%d \n", currentarg );

		/* Handle format arguments if any */
		if (i < formatlen) {
			char c = format[i];

			if (c == '*') {
				arg = -1;
				i++;
			}
			else if (c >= '0' && c <= '9') {
				arg = atoi(&format[i]);
		  
				while (format[i] >= '0' && format[i] <= '9' && i < formatlen) {
					i++;
				}
			}
		}
		
		if( IsDebug()>0 ) printf("starting..format codes  currentargs=%d\n", currentarg );

		/* Handle special arg '*' for all codes and check argv overflows */
		switch ((int) code) {
			/* Never uses any args */
			case 'x': 
			case 'X':	
			case '@':
				if (arg < 0) {
					NODEJS_ERROR( "Type %c: '*' ignored", code);
					arg = 1;
				}
				break;

			/* Always uses one arg */
			case 'a': 
			case 'A': 
			case 'Z': 
			case 'h': 
			case 'H':
				if (currentarg >= num_args) {
					NODEJS_ERROR( "Type %c: not enough arguments", code);
					return Undefined();
				}

				if (arg < 0) {
					Local<String> currstr = GETARG(currentarg)->ToString();
					arg = currstr->Length();
					if( code == 'Z' ) {
						arg++;
					}
				}
				currentarg++;
				break;

			/* Use as many args as specified */
			case 'c': 
			case 'C': 
			case 's': 
			case 'S': 
			case 'i': 
			case 'I':
			case 'l': 
			case 'L': 
			case 'n': 
			case 'N': 
			case 'v': 
			case 'V':
			case 'f': 
			case 'd': 
				if (arg < 0) {
					arg = num_args - currentarg;
				}
				//printf("currentarg=%d, arg=%d, code=%c\n", currentarg, arg, code );
				currentarg += arg;

				if (currentarg > num_args) {
					NODEJS_ERROR( "Type %c: too few arguments", code);
					return Undefined();
				}
				break;

			default:
				NODEJS_ERROR( "Type %c: unknown format code", code);
				return Undefined();
		}

		formatcodes[formatcount] = code;
		formatargs[formatcount] = arg;
	}

	if (currentarg < num_args) {
		NODEJS_ERROR( "%d arguments unused", (num_args - currentarg));
	}

	/* Calculate output length and upper bound while processing*/
	for (i = 0; i < formatcount; i++) {
	    int code = (int) formatcodes[i];
		int arg = formatargs[i];

		switch ((int) code) {
			case 'h': 
			case 'H': 
				INC_OUTPUTPOS((arg + (arg % 2)) / 2,1)	/* 4 bit per arg */
				break;

			case 'a': 
			case 'A':
			case 'Z':
			case 'c': 
			case 'C':
			case 'x':
				INC_OUTPUTPOS(arg,1)		/* 8 bit per arg */
				break;

			case 's': 
			case 'S': 
			case 'n': 
			case 'v':
				INC_OUTPUTPOS(arg,2)		/* 16 bit per arg */
				break;

			case 'i': 
			case 'I':
				INC_OUTPUTPOS(arg,sizeof(int))
				break;

			case 'l': 
			case 'L': 
			case 'N': 
			case 'V':
				INC_OUTPUTPOS(arg,4)		/* 32 bit per arg */
				break;

			case 'f':
				INC_OUTPUTPOS(arg,sizeof(float))
				break;

			case 'd':
				INC_OUTPUTPOS(arg,sizeof(double))
				break;

			case 'X':
				outputpos -= arg;

				if (outputpos < 0) {
					NODEJS_ERROR( "Type %c: outside of string", code);
					outputpos = 0;
				}
				break;

			case '@':
				outputpos = arg;
				break;
		}

		if (outputsize < outputpos) {
			outputsize = outputpos;
		}
	}

	
	if( parentBuffer ) 
	{
		output = BufferData( outputBuffer );
	} else
	if( outputsize > 255 ) {
		output = (char*)malloc(outputsize + 1);
	} else {
		output = localOutput;			// Use local storage
	}
	
	outputpos = 0;
	currentarg = firstarg; 

	Local<Value>            val;
	Local<String>           strval;
	/* Do actual packing */
	for (i = 0; i < formatcount; i++) {
        
	    int code = (int) formatcodes[i];
		int arg = formatargs[i];

		switch ((int) code) {
			case 'a': 
			case 'A': 
			case 'Z': 
                {
					int arg_cp = (code != 'Z') ? arg : MAX(0, arg - 1);
                    memset(&output[outputpos], (code == 'a' || code == 'Z') ? '\0' : ' ', arg);
                    //String::Utf8Value utf( argv[currentarg++]->ToString() );
                    String::Utf8Value utf( GETARG(currentarg)->ToString() );
					currentarg++;
                    memcpy(&output[outputpos], *utf, (utf.length() < arg_cp) ? utf.length() : arg_cp);
                    outputpos += arg;
                }
				break;

			case 'h': 
			case 'H': {
				int nibbleshift = (code == 'h') ? 0 : 4;
				int first = 1;

                String::Utf8Value utf( GETARG(currentarg)->ToString() );
				currentarg++;
				
				if(arg > utf.length() ) {
					NODEJS_ERROR( "Type %c: not enough characters in string", code );
					arg = utf.length();
				}

				outputpos--;
				int vi=0;
				char *v = *utf;
				while (arg-- > 0) {
					char n = v[vi++];

					if (n >= '0' && n <= '9') {
						n -= '0';
					} else if (n >= 'A' && n <= 'F') {
						n -= ('A' - 10);
					} else if (n >= 'a' && n <= 'f') {
						n -= ('a' - 10);
					} else {
						THROW_ERROR( _buf, (_buf, "Type %c: illegal hex digit %c", code, n) );
						n = 0;
					}

					if (first--) {
						output[++outputpos] = 0;
					} else {
					  first = 1;
					}

					output[outputpos] |= (n << nibbleshift);
					nibbleshift = (nibbleshift + 4) & 7;
				}

				outputpos++;
				break;
			}

			case 'c': 
			case 'C':
				while (arg-- > 0) {
					val = GETARG(currentarg);
					currentarg++;
					NODEJS_pack( (int)val->Int32Value(), 1, byte_map, &output[outputpos]);
					outputpos++;
				}
				break;

			case 's': 
			case 'S': 
			case 'n': 
			case 'v': {
				int *map = machine_endian_short_map;

				if (code == 'n') {
					map = big_endian_short_map;
				} else if (code == 'v') {
					map = little_endian_short_map;
				}

				while (arg-- > 0) {
					val = GETARG(currentarg);
					currentarg++;
					NODEJS_pack(val->Int32Value(), 2, map, &output[outputpos]);
					outputpos += 2;
				}
				break;
			}

			case 'i': 
			case 'I': 
				while (arg-- > 0) {
					val = GETARG(currentarg);
					currentarg++;
					NODEJS_pack(val->Int32Value(), sizeof(int), int_map, &output[outputpos]);
					outputpos += sizeof(int);
				}
				break;

			case 'l': 
			case 'L': 
			case 'N': 
			case 'V': {
				int *map = machine_endian_long_map;

				if (code == 'N') {
					map = big_endian_long_map;
				} else if (code == 'V') {
					map = little_endian_long_map;
				}

				while (arg-- > 0) {
					val = GETARG(currentarg);
					currentarg++;
					NODEJS_pack(val->Int32Value(), 4, map, &output[outputpos]);
					outputpos += 4;
				}
				break;
			}

			case 'f': {
				float v;

				while (arg-- > 0) {
					val = GETARG(currentarg);
					currentarg++;
					v = ToFloat( val );
					memcpy(&output[outputpos], &v, sizeof(v));
					outputpos += sizeof(v);
				}
				break;
			}

			case 'd': {
				double v;

				while (arg-- > 0) {
					val = GETARG(currentarg);
					currentarg++;
					v = ToDouble( val );
					memcpy(&output[outputpos], &v, sizeof(v));
					outputpos += sizeof(v);
				}
				break;
			}

			case 'x':
				memset(&output[outputpos], '\0', arg);
				outputpos += arg;
				break;

			case 'X':
				outputpos -= arg;

				if (outputpos < 0) {
					outputpos = 0;
				}
				break;

			case '@':
				if (arg > outputpos) {
					memset(&output[outputpos], '\0', arg - outputpos);
				}
				outputpos = arg;
				break;
		}
	}

	output[outputpos] = '\0';
	
	//return Undefined();
	if( parentBuffer ) {
		return scope.Close( outputBuffer );
	}
	
	Local<Buffer> return_buffer = Buffer::New( output, outputpos );
	if( outputsize > 255 ) {
		free(output);
	}
	return scope.Close( return_buffer->handle_ );
}
/* }}} */








/* {{{ NODEJS_unpack
 */
static long NODEJS_unpack(char *data, int size, int issigned, int *map)
{
	long result;
	char *cresult = (char *) &result;
	int i;

	result = issigned ? -1 : 0;

	for (i = 0; i < size; i++) {
		cresult[map[i]] = *data++;
	}

	return result;
}
/* }}} */





/* unpack() is based on Perl's unpack(), but is modified a bit from there.
 * Rather than depending on error-prone ordered lists or syntactically
 * unpleasant pass-by-reference, we return an object with named paramters 
 * (like *_fetch_object()). Syntax is "f[repeat]name/...", where "f" is the
 * formatter char (like pack()), "[repeat]" is the optional repeater argument,
 * and "name" is the name of the variable to use.
 * Example: "c2chars/nints" will return an object with fields
 * chars1, chars2, and ints.
 * Numeric pack types will return numbers, a and A will return strings,
 * f and d will return doubles.
 * Implemented formats are Z, A, a, h, H, c, C, s, S, i, I, l, L, n, N, f, d, x, X, @.
 */
/* {{{ proto array unpack(String format, Buffer input)
   Unpack binary string into named array elements according to format argument */

static Handle<Value> unpack(const Arguments& argv)
{
    HandleScope scope;
    const char  *format;
	char        *input = NULL;
	int         formatlen, formatidx=0;
	int         inputpos, inputlen=0, i;
	bool        usePerlFormat = false;

	if (argv.Length() == 0 || !argv[0]->IsString()) {
		return ThrowException(Exception::TypeError(
            String::New("First argument must be a string")));
    }

	String::Utf8Value	formatString( argv[0]->ToString() );
	format = *(formatString);
	formatlen = formatString.length();
    
    if (Buffer::HasInstance(argv[1])) {
       // buffer
       Local<Object> buffer = argv[1]->ToObject();
       inputlen = BufferLength(buffer);
       input = BufferData(buffer);
    }    
        
	inputpos = 0;
	if (argv.Length() >= 3 && argv[2].IsEmpty() == false ) {
        usePerlFormat = true;
	}

	Local<Object> return_value = Object::New();

	while (formatlen-- > 0) {
		char type = format[ formatidx++ ];
		char c;
		int arg = 1, argb;
		int namelen = 0;
		int size=0;
		Local<String>	name;

		/* Handle format arguments if any */
		if (formatlen > 0) {
			c = format[formatidx];

			if (c >= '0' && c <= '9') {
				arg = c - '0';

				while (formatlen > 0 && format[formatidx] >= '0' && format[formatidx] <= '9') {
					arg = (arg*10) + (format[formatidx] - '0');
					formatidx++;
					formatlen--;
				}
			} else if (c == '*') {
				arg = -1;
				formatidx++;
				formatlen--;
			}
		}
		argb = arg;

		/* Get of new value in array */
		if( usePerlFormat == false ) {
			namelen = 0;
			while (formatlen > 0 && format[formatidx] != '/') {
				formatlen--;
				formatidx++;
				namelen++;
			}
			if (namelen > 200)
				namelen = 200;
			name = String::New( format+formatidx-namelen, namelen );
		} else {
			char temp[8]; sprintf( temp, "%d", arg );
			namelen = strlen(temp);
			name = String::New( temp, namelen );
		}

		switch ((int) type) {
			/* Never use any input */
			case 'X': 
				size = -1;
				break;

			case '@':
				size = 0;
				break;

			case 'a': 
			case 'A':
			case 'Z':
				size = arg;
				arg = 1;
				break;

			case 'h': 
			case 'H': 
				size = (arg > 0) ? (arg + (arg % 2)) / 2 : arg;
				arg = 1;
				break;

			/* Use 1 byte of input */
			case 'c': 
			case 'C':
			case 'x':
				size = 1;
				break;

			/* Use 2 bytes of input */
			case 's': 
			case 'S': 
			case 'n': 
			case 'v':
				size = 2;
				break;

			/* Use sizeof(int) bytes of input */
			case 'i': 
			case 'I':
				size = sizeof(int);
				break;

			/* Use 4 bytes of input */
			case 'l': 
			case 'L': 
			case 'N': 
			case 'V':
				size = 4;
				break;

			/* Use sizeof(float) bytes of input */
			case 'f':
				size = sizeof(float);
				break;

			/* Use sizeof(double) bytes of input */
			case 'd':
				size = sizeof(double);
				break;

			default:
				NODEJS_ERROR( "Invalid format type %c", type);
				return Undefined();
				break;
		}

		/* Do actual unpacking */
		for (i = 0; i != arg; i++ ) {
			/* Space for name + number, safe as namelen is ensured <= 200 */
			char n[256];

			if (arg != 1 || namelen == 0) {
				/* Need to add element number to name */
				snprintf(n, sizeof(n), "%.*s%d", namelen, *String::Utf8Value(name), i + 1);
			} else {
				/* Truncate name to next format code or end of string */
				snprintf(n, sizeof(n), "%.*s", namelen, *String::Utf8Value(name));
			}

			if (size != 0 && size != -1 && INT_MAX - size + 1 < inputpos) {
				NODEJS_ERROR( "Type %c: integer overflow", type);
				inputpos = 0;
			}

			if ((inputpos + size) <= inputlen && input != NULL) {
				switch ((int) type) {
					case 'a': 
					case 'A': {
						char pad = (type == 'a') ? '\0' : ' ';
						int len = inputlen - inputpos;	/* Remaining string */

						/* If size was given take minimum of len and size */
						if ((size >= 0) && (len > size)) {
							len = size;
						}

						size = len;

						/* Remove padding chars from unpacked data */
						while (--len >= 0) {
							if (input[inputpos + len] != pad)
								break;
						}

						return_value->Set( String::New(n), String::New( &input[inputpos], len + 1 ) );
						break;
					}
					case 'Z': {
					  /* Z will strip everything after the first null character */
					  char pad = '\0';
					  int      s,
							   len = inputlen - inputpos;     /* Remaining string */

					  /* If size was given take minimum of len and size */
					  if ((size >= 0) && (len > size)) {
							  len = size;
					  }

					  size = len;

					  /* Remove everything after the first null */
					  for (s=0 ; s < len ; s++) {
							  if (input[inputpos + s] == pad)
									  break;
					  }
					  len = s;

					  return_value->Set( String::New(n), String::New( &input[inputpos], len ) );
					  break;
                                  }					
					case 'h': 
					case 'H': {
						int len = (inputlen - inputpos) * 2;	/* Remaining */
						int nibbleshift = (type == 'h') ? 0 : 4;
						int first = 1;
						char *buf;
						int ipos, opos;

						/* If size was given take minimum of len and size */
						if (size >= 0 && len > (size * 2)) {
							len = size * 2;
						} 

						if (argb > 0) {	
							len -= argb % 2;
						}

						buf = (char*)malloc(len + 1);

						for (ipos = opos = 0; opos < len; opos++) {
							char cc = (input[inputpos + ipos] >> nibbleshift) & 0xf;

							if (cc < 10) {
								cc += '0';
							} else {
								cc += 'a' - 10;
							}

							buf[opos] = cc;
							nibbleshift = (nibbleshift + 4) & 7;

							if (first-- == 0) {
								ipos++;
								first = 1;
							}
						}

						buf[len] = '\0';
						//add_assoc_stringl(return_value, n, buf, len, 1);
						return_value->Set( String::New(n), String::New(buf,len) );
						free(buf);
						break;
					}

					case 'c': 
					case 'C': {
						int issigned = (type == 'c') ? (input[inputpos] & 0x80) : 0;
						long v = NODEJS_unpack(&input[inputpos], 1, issigned, byte_map);
						//add_assoc_long(return_value, n, v);
						return_value->Set( String::New(n), Integer::New(v) );
						break;
					}

					case 's': 
					case 'S': 
					case 'n': 
					case 'v': {
						long v;
						int issigned = 0;
						int *map = machine_endian_short_map;

						if (type == 's') {
							issigned = input[inputpos + (machine_little_endian ? 1 : 0)] & 0x80;
						} else if (type == 'n') {
							map = big_endian_short_map;
						} else if (type == 'v') {
							map = little_endian_short_map;
						}

						v = NODEJS_unpack(&input[inputpos], 2, issigned, map);
						//add_assoc_long(return_value, n, v);
						return_value->Set( String::New(n), Integer::New(v) );
						break;
					}

					case 'i': 
					case 'I': {
						long v = 0;
						int issigned = 0;

						if (type == 'i') {
							issigned = input[inputpos + (machine_little_endian ? (sizeof(int) - 1) : 0)] & 0x80;
						} else if (sizeof(long) > 4 && (input[inputpos + machine_endian_long_map[3]] & 0x80) == 0x80) {
							v = ~INT_MAX;
						}

						v |= NODEJS_unpack(&input[inputpos], sizeof(int), issigned, int_map);
						//add_assoc_long(return_value, n, v);
						return_value->Set( String::New(n), Integer::New(v) );
						break;
					}

					case 'l': 
					case 'L': 
					case 'N': 
					case 'V': {
						int issigned = 0;
						int *map = machine_endian_long_map;
						long v = 0;

						if (type == 'l' || type == 'L') {
							issigned = input[inputpos + (machine_little_endian ? 3 : 0)] & 0x80;
						} else if (type == 'N') {
							issigned = input[inputpos] & 0x80;
							map = big_endian_long_map;
						} else if (type == 'V') {
							issigned = input[inputpos + 3] & 0x80;
							map = little_endian_long_map;
						}

						if (sizeof(long) > 4 && issigned) {
							v = ~INT_MAX;
						}

						v |= NODEJS_unpack(&input[inputpos], 4, issigned, map);
						if (sizeof(long) > 4) {
 							if (type == 'l') {
								v = (signed int) v; 
							} else {
								v = (unsigned int) v;
							}
						}
						//add_assoc_long(return_value, n, v);
						return_value->Set( String::New(n), Integer::New(v) );
						break;
					}

					case 'f': {
						float v;

						memcpy(&v, &input[inputpos], sizeof(float));
						//add_assoc_double(return_value, n, (double)v);
						return_value->Set( String::New(n), Integer::New(v) );
						break;
					}

					case 'd': {
						double v;

						memcpy(&v, &input[inputpos], sizeof(double));
						//add_assoc_double(return_value, n, v);
						return_value->Set( String::New(n), Integer::New(v) );
						break;
					}

					case 'x':
						/* Do nothing with input, just skip it */
						break;

					case 'X':
						if (inputpos < size) {
							inputpos = -size;
							i = arg - 1;		/* Break out of for loop */

							if (arg >= 0) {
								NODEJS_ERROR( "Type %c: outside of string", type);
							}
						}
						break;

					case '@':
						if (arg <= inputlen) {
							inputpos = arg;
						} else {
							NODEJS_ERROR( "Type %c: outside of string", type);
						}

						i = arg - 1;	/* Done, break out of for loop */
						break;
				}

				inputpos += size;
				if (inputpos < 0) {
					if (size != -1) { /* only print warning if not working with * */
						NODEJS_ERROR( "Type %c: outside of string", type);
					}
					inputpos = 0;
				}
			} else if (arg < 0) {
				/* Reached end of input for '*' repeater */
				break;
			} else {
				THROW_ERROR( _buf, ( _buf, "Type %c: not enough input, need %d, have %d", type, size, inputlen - inputpos) );
				return Undefined();
			}
		}

		formatlen--;	/* Skip '/' separator, does no harm if inputlen == 0 */
		format++;
	}
	
	return scope.Close( return_value );
}
/* }}} */


static Handle<Value> setdebug( const Arguments& argv )
{
	HandleScope scope;
	if( argv[0]->IsInt32() )
	{
		Local<Object> var = argv[0]->ToObject();
		SetDebug( var->Int32Value() );
	}
	return scope.Close( True() );
}


static Handle<Value> debug( void )
{
	HandleScope scope;
	return scope.Close( IsDebug() ? True() : False() );
}


/* {{{ NODEJS_MINIT_FUNCTION
 */
static void Initialize(Handle<Object> target)
{
	HandleScope scope;
	
	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	t->InstanceTemplate()->SetInternalFieldCount(1);

	target->Set(String::NewSymbol("HiPack"), t->GetFunction());
	
	//NODE_SET_PROTOTYPE_METHOD(t, "pack", pack);
	NODE_SET_METHOD(target, "pack", pack);
	NODE_SET_METHOD(target, "unpack", unpack);
	NODE_SET_METHOD(target, "setdebug", setdebug);
	//NODE_SET_METHOD(target, "debug", debug);
	
	SetDebug(0);
	int machine_endian_check = 1;
	int i;

	machine_little_endian = ((char *)&machine_endian_check)[0];

	if (machine_little_endian) {
		/* Where to get lo to hi bytes from */
		byte_map[0] = 0;

		for (i = 0; i < (int)sizeof(int); i++) {
			int_map[i] = i;
		}

		machine_endian_short_map[0] = 0;
		machine_endian_short_map[1] = 1;
		big_endian_short_map[0] = 1;
		big_endian_short_map[1] = 0;
		little_endian_short_map[0] = 0;
		little_endian_short_map[1] = 1;

		machine_endian_long_map[0] = 0;
		machine_endian_long_map[1] = 1;
		machine_endian_long_map[2] = 2;
		machine_endian_long_map[3] = 3;
		big_endian_long_map[0] = 3;
		big_endian_long_map[1] = 2;
		big_endian_long_map[2] = 1;
		big_endian_long_map[3] = 0;
		little_endian_long_map[0] = 0;
		little_endian_long_map[1] = 1;
		little_endian_long_map[2] = 2;
		little_endian_long_map[3] = 3;
	}
	else {
		int val;
		int size = sizeof((val));

		/* Where to get hi to lo bytes from */
		byte_map[0] = size - 1;

		for (i = 0; i < (int)sizeof(int); i++) {
			int_map[i] = size - (sizeof(int) - i);
		}

		machine_endian_short_map[0] = size - 2;
		machine_endian_short_map[1] = size - 1;
		big_endian_short_map[0] = size - 2;
		big_endian_short_map[1] = size - 1;
		little_endian_short_map[0] = size - 1;
		little_endian_short_map[1] = size - 2;

		machine_endian_long_map[0] = size - 4;
		machine_endian_long_map[1] = size - 3;
		machine_endian_long_map[2] = size - 2;
		machine_endian_long_map[3] = size - 1;
		big_endian_long_map[0] = size - 4;
		big_endian_long_map[1] = size - 3;
		big_endian_long_map[2] = size - 2;
		big_endian_long_map[3] = size - 1;
		little_endian_long_map[0] = size - 1;
		little_endian_long_map[1] = size - 2;
		little_endian_long_map[2] = size - 3;
		little_endian_long_map[3] = size - 4;
	}
}
/* }}} */

};

extern "C" void init (Handle<Object> target) 
{
  HandleScope scope;
  HiPack::Initialize(target);
}
    
NODE_MODULE(hipack, init);

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
