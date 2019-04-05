#include "client.h"

#include "CharGrid.h"


/*
    Very Strict UTF-8 Decoder

    UTF-8 is a multibyte character encoding of Unicode. A character can be
    represented by 1-4 bytes. The bit pattern of the first byte indicates the
    number of continuation bytes.

    Most UTF-8 decoders tend to be lenient, attempting to recover as much
    information as possible, even from badly encoded input. This UTF-8
    decoder is not lenient. It will reject input which does not include
    proper continuation bytes. It will reject aliases (or suboptimal
    codings). It will reject surrogates. (Surrogate encoding should only be
    used with UTF-16.)

    Code     Contination Minimum Maximum
    0xxxxxxx           0       0     127
    10xxxxxx       error
    110xxxxx           1     128    2047
    1110xxxx           2    2048   65535 excluding 55296 - 57343
    11110xxx           3   65536 1114111
    11111xxx       error
*/

#define UTF8_END   -1
#define UTF8_ERROR -2


static int  the_index = 0;
static int  the_length = 0;
static int  the_char = 0;
static int  the_byte = 0;
static char* the_input;


/*
    Get the next byte. It returns UTF8_END if there are no more bytes.
*/
static int get() {
    int c;
    if (the_index >= the_length) {
        return UTF8_END;
    }
    c = the_input[the_index] & 0xFF;
    the_index += 1;
    return c;
}


/*
    Get the 6-bit payload of the next continuation byte.
    Return UTF8_ERROR if it is not a contination byte.
*/
static int cont() {
    int c = get();
    return ((c & 0xC0) == 0x80)
        ? (c & 0x3F)
        : UTF8_ERROR;
}


/*
    Initialize the UTF-8 decoder. The decoder is not reentrant,
*/
void utf8_decode_init(char p[], int length) {
    the_index = 0;
    the_input = p;
    the_length = length;
    the_char = 0;
    the_byte = 0;
}


/*
    Get the current byte offset. This is generally used in error reporting.
*/
int utf8_decode_at_byte() {
    return the_byte;
}


/*
    Get the current character offset. This is generally used in error reporting.
    The character offset matches the byte offset if the text is strictly ASCII.
*/
int utf8_decode_at_character() {
    return (the_char > 0)
        ? the_char - 1
        : 0;
}


/*
    Extract the next character.
    Returns: the character (between 0 and 1114111)
         or  UTF8_END   (the end)
         or  UTF8_ERROR (error)
*/
int utf8_decode_next() {
    int c;  /* the first byte of the character */
    int c1; /* the first continuation character */
    int c2; /* the second continuation character */
    int c3; /* the third continuation character */
    int r;  /* the result */

    if (the_index >= the_length) {
        return the_index == the_length ? UTF8_END : UTF8_ERROR;
    }
    the_byte = the_index;
    the_char += 1;
    c = get();
/*
    Zero continuation (0 to 127)
*/
    if ((c & 0x80) == 0) {
        return c;
    }
/*
    One continuation (128 to 2047)
*/
    if ((c & 0xE0) == 0xC0) {
        c1 = cont();
        if (c1 >= 0) {
            r = ((c & 0x1F) << 6) | c1;
            if (r >= 128) {
                return r;
            }
        }

/*
    Two continuations (2048 to 55295 and 57344 to 65535)
*/
    } else if ((c & 0xF0) == 0xE0) {
        c1 = cont();
        c2 = cont();
        if ((c1 | c2) >= 0) {
            r = ((c & 0x0F) << 12) | (c1 << 6) | c2;
            if (r >= 2048 && (r < 55296 || r > 57343)) {
                return r;
            }
        }

/*
    Three continuations (65536 to 1114111)
*/
    } else if ((c & 0xF8) == 0xF0) {
        c1 = cont();
        c2 = cont();
        c3 = cont();
        if ((c1 | c2 | c3) >= 0) {
            r = ((c & 0x07) << 18) | (c1 << 12) | (c2 << 6) | c3;
            if (r >= 65536 && r <= 1114111) {
                return r;
            }
        }
    }
    return UTF8_ERROR;
}


void CharGrid::printf( int x, int y, Color c, const char *fmt, ...) {
	va_list argptr;
	char s[1024];
	va_start(argptr, fmt);
	vsnprintf( s, sizeof(s), fmt, argptr );
	va_end(argptr);
    utf8_decode_init(s,strlen(s));
    int dx=0, dy=0;
	for(int i=0;;i++){
        int ch=utf8_decode_next();
        if(ch==0xffffffff)break;
        if(ch=='\n') {
            dx=0;
            dy--;
            continue;
        }
        if(x+dx>=width)continue;
        int ind=ch;
        if(ch>=0&&ch<=0x80) {
            ind += ascii_offset;
        } else if(ch>=0x3041&&ch<=0x3094) {
            ind = hiragana_unicode_offset + (ind-0x3041);
        } else if(ch>=0x30A1&&ch<=0x30FA) {
            ind = katakana_unicode_offset + (ind-0x30A1);
            print("KKKK ind:%d kt:%d",ind, katakana_unicode_offset);
        }
        set(x+dx,y+dy,ind);
		setColor(x+dx,y+dy,c);
        dx++;
	}    
}
