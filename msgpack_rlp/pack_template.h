/*
 * MessagePack packing routine template
 *
 * Copyright (C) 2008-2010 FURUHASHI Sadayuki
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <math.h>

#if defined(__LITTLE_ENDIAN__)
#define TAKE8_8(d)  ((uint8_t*)&d)[0]
#define TAKE8_16(d) ((uint8_t*)&d)[0]
#define TAKE8_32(d) ((uint8_t*)&d)[0]
#define TAKE8_64(d) ((uint8_t*)&d)[0]
#elif defined(__BIG_ENDIAN__)
#define TAKE8_8(d)  ((uint8_t*)&d)[0]
#define TAKE8_16(d) ((uint8_t*)&d)[1]
#define TAKE8_32(d) ((uint8_t*)&d)[3]
#define TAKE8_64(d) ((uint8_t*)&d)[7]
#endif

#ifndef msgpack_pack_append_buffer
#error msgpack_pack_append_buffer callback is not defined
#endif




/*
 * Integer
 */

#define msgpack_pack_real_uint8(x, d) \
do { \
    if(d < (1<<7)) { \
        /* fixnum */ \
        msgpack_pack_append_buffer(x, &TAKE8_8(d), 1); \
    } else { \
        /* unsigned 8 */ \
        unsigned char buf[2] = {0xcc, TAKE8_8(d)}; \
        msgpack_pack_append_buffer(x, buf, 2); \
    } \
} while(0)

#define msgpack_pack_real_uint16(x, d) \
do { \
    if(d < (1<<7)) { \
        /* fixnum */ \
        msgpack_pack_append_buffer(x, &TAKE8_16(d), 1); \
    } else if(d < (1<<8)) { \
        /* unsigned 8 */ \
        unsigned char buf[2] = {0xcc, TAKE8_16(d)}; \
        msgpack_pack_append_buffer(x, buf, 2); \
    } else { \
        /* unsigned 16 */ \
        unsigned char buf[3]; \
        buf[0] = 0xcd; _msgpack_store16(&buf[1], (uint16_t)d); \
        msgpack_pack_append_buffer(x, buf, 3); \
    } \
} while(0)

#define msgpack_pack_real_uint32(x, d) \
do { \
    if(d < (1<<8)) { \
        if(d < (1<<7)) { \
            /* fixnum */ \
            msgpack_pack_append_buffer(x, &TAKE8_32(d), 1); \
        } else { \
            /* unsigned 8 */ \
            unsigned char buf[2] = {0xcc, TAKE8_32(d)}; \
            msgpack_pack_append_buffer(x, buf, 2); \
        } \
    } else { \
        if(d < (1<<16)) { \
            /* unsigned 16 */ \
            unsigned char buf[3]; \
            buf[0] = 0xcd; _msgpack_store16(&buf[1], (uint16_t)d); \
            msgpack_pack_append_buffer(x, buf, 3); \
        } else { \
            /* unsigned 32 */ \
            unsigned char buf[5]; \
            buf[0] = 0xce; _msgpack_store32(&buf[1], (uint32_t)d); \
            msgpack_pack_append_buffer(x, buf, 5); \
        } \
    } \
} while(0)

#define msgpack_pack_real_uint64(x, d) \
do { \
    if(d < (1ULL<<8)) { \
        if(d < (1ULL<<7)) { \
            /* fixnum */ \
            msgpack_pack_append_buffer(x, &TAKE8_64(d), 1); \
        } else { \
            /* unsigned 8 */ \
            unsigned char s = 0x80 | (uint8_t)1; \
            unsigned char buf[2] = {s, TAKE8_64(d)}; \
            msgpack_pack_append_buffer(x, buf, 2); \
        } \
    } else { \
        if(d < (1ULL<<16)) { \
            /* unsigned 16 */ \
            unsigned char s = 0x80 | (uint8_t)2; \
            unsigned char buf[3]; \
            buf[0] = s; _msgpack_store16(&buf[1], (uint16_t)d); \
            msgpack_pack_append_buffer(x, buf, 3); \
        } else if(d < (1ULL<<24)) { \
            /* unsigned 24 */ \
            unsigned char s = 0x80 | (uint8_t)3; \
            unsigned char buf[4]; \
            unsigned char padded_buf[4]; \
            buf[0] = s; _msgpack_store32(&padded_buf[0], (uint32_t)d); \
            buf[1] = padded_buf[1]; \
            buf[2] = padded_buf[2]; \
            buf[3] = padded_buf[3]; \
            msgpack_pack_append_buffer(x, buf, 4); \
        } else if(d < (1ULL<<32)) { \
            /* unsigned 32 */ \
            unsigned char s = 0x80 | (uint8_t)4; \
            unsigned char buf[5]; \
            buf[0] = s; _msgpack_store32(&buf[1], (uint32_t)d); \
            msgpack_pack_append_buffer(x, buf, 5); \
        } else if(d < (1ULL<<40)) { \
            /* unsigned 40 */ \
            unsigned char s = 0x80 | (uint8_t)5; \
            unsigned char buf[6]; \
            unsigned char padded_buf[8]; \
            buf[0] = s; _msgpack_store64(&padded_buf[0], (uint64_t)d); \
            buf[1] = padded_buf[3]; \
            buf[2] = padded_buf[4]; \
            buf[3] = padded_buf[5]; \
            buf[4] = padded_buf[6]; \
            buf[5] = padded_buf[7]; \
            msgpack_pack_append_buffer(x, buf, 6); \
        } else if(d < (1ULL<<48)) { \
            /* unsigned 48 */ \
            unsigned char s = 0x80 | (uint8_t)6; \
            unsigned char buf[7]; \
            unsigned char padded_buf[8]; \
            buf[0] = s; _msgpack_store64(&padded_buf[0], (uint64_t)d); \
            buf[1] = padded_buf[2]; \
            buf[2] = padded_buf[3]; \
            buf[3] = padded_buf[4]; \
            buf[4] = padded_buf[5]; \
            buf[5] = padded_buf[6]; \
            buf[6] = padded_buf[7]; \
            msgpack_pack_append_buffer(x, buf, 7); \
        } else if(d < (1ULL<<56)) { \
            /* unsigned 56 */ \
            unsigned char s = 0x80 | (uint8_t)7; \
            unsigned char buf[8]; \
            unsigned char padded_buf[8]; \
            buf[0] = s; _msgpack_store64(&padded_buf[0], (uint64_t)d); \
            buf[1] = padded_buf[1]; \
            buf[2] = padded_buf[2]; \
            buf[3] = padded_buf[3]; \
            buf[4] = padded_buf[4]; \
            buf[5] = padded_buf[5]; \
            buf[6] = padded_buf[6]; \
            buf[7] = padded_buf[7]; \
            msgpack_pack_append_buffer(x, buf, 8); \
        } else { \
            /* unsigned 64 */ \
            unsigned char s = 0x80 | (uint8_t)8; \
            unsigned char buf[9]; \
            buf[0] = s; _msgpack_store64(&buf[1], d); \
            msgpack_pack_append_buffer(x, buf, 9); \
        } \
    } \
} while(0)

#define msgpack_pack_real_int8(x, d) \
do { \
    if(d < -(1<<5)) { \
        /* signed 8 */ \
        unsigned char buf[2] = {0xd0, TAKE8_8(d)}; \
        msgpack_pack_append_buffer(x, buf, 2); \
    } else { \
        /* fixnum */ \
        msgpack_pack_append_buffer(x, &TAKE8_8(d), 1); \
    } \
} while(0)

#define msgpack_pack_real_int16(x, d) \
do { \
    if(d < -(1<<5)) { \
        if(d < -(1<<7)) { \
            /* signed 16 */ \
            unsigned char buf[3]; \
            buf[0] = 0xd1; _msgpack_store16(&buf[1], (int16_t)d); \
            msgpack_pack_append_buffer(x, buf, 3); \
        } else { \
            /* signed 8 */ \
            unsigned char buf[2] = {0xd0, TAKE8_16(d)}; \
            msgpack_pack_append_buffer(x, buf, 2); \
        } \
    } else if(d < (1<<7)) { \
        /* fixnum */ \
        msgpack_pack_append_buffer(x, &TAKE8_16(d), 1); \
    } else { \
        if(d < (1<<8)) { \
            /* unsigned 8 */ \
            unsigned char buf[2] = {0xcc, TAKE8_16(d)}; \
            msgpack_pack_append_buffer(x, buf, 2); \
        } else { \
            /* unsigned 16 */ \
            unsigned char buf[3]; \
            buf[0] = 0xcd; _msgpack_store16(&buf[1], (uint16_t)d); \
            msgpack_pack_append_buffer(x, buf, 3); \
        } \
    } \
} while(0)

#define msgpack_pack_real_int32(x, d) \
do { \
    if(d < -(1<<5)) { \
        if(d < -(1<<15)) { \
            /* signed 32 */ \
            unsigned char buf[5]; \
            buf[0] = 0xd2; _msgpack_store32(&buf[1], (int32_t)d); \
            msgpack_pack_append_buffer(x, buf, 5); \
        } else if(d < -(1<<7)) { \
            /* signed 16 */ \
            unsigned char buf[3]; \
            buf[0] = 0xd1; _msgpack_store16(&buf[1], (int16_t)d); \
            msgpack_pack_append_buffer(x, buf, 3); \
        } else { \
            /* signed 8 */ \
            unsigned char buf[2] = {0xd0, TAKE8_32(d)}; \
            msgpack_pack_append_buffer(x, buf, 2); \
        } \
    } else if(d < (1<<7)) { \
        /* fixnum */ \
        msgpack_pack_append_buffer(x, &TAKE8_32(d), 1); \
    } else { \
        if(d < (1<<8)) { \
            /* unsigned 8 */ \
            unsigned char buf[2] = {0xcc, TAKE8_32(d)}; \
            msgpack_pack_append_buffer(x, buf, 2); \
        } else if(d < (1<<16)) { \
            /* unsigned 16 */ \
            unsigned char buf[3]; \
            buf[0] = 0xcd; _msgpack_store16(&buf[1], (uint16_t)d); \
            msgpack_pack_append_buffer(x, buf, 3); \
        } else { \
            /* unsigned 32 */ \
            unsigned char buf[5]; \
            buf[0] = 0xce; _msgpack_store32(&buf[1], (uint32_t)d); \
            msgpack_pack_append_buffer(x, buf, 5); \
        } \
    } \
} while(0)

#define msgpack_pack_real_int64(x, d) \
do { \
    if(d < -(1LL<<5)) { \
        if(d < -(1LL<<15)) { \
            if(d < -(1LL<<31)) { \
                /* signed 64 */ \
                unsigned char buf[9]; \
                buf[0] = 0xd3; _msgpack_store64(&buf[1], d); \
                msgpack_pack_append_buffer(x, buf, 9); \
            } else { \
                /* signed 32 */ \
                unsigned char buf[5]; \
                buf[0] = 0xd2; _msgpack_store32(&buf[1], (int32_t)d); \
                msgpack_pack_append_buffer(x, buf, 5); \
            } \
        } else { \
            if(d < -(1<<7)) { \
                /* signed 16 */ \
                unsigned char buf[3]; \
                buf[0] = 0xd1; _msgpack_store16(&buf[1], (int16_t)d); \
                msgpack_pack_append_buffer(x, buf, 3); \
            } else { \
                /* signed 8 */ \
                unsigned char buf[2] = {0xd0, TAKE8_64(d)}; \
                msgpack_pack_append_buffer(x, buf, 2); \
            } \
        } \
    } else if(d < (1<<7)) { \
        /* fixnum */ \
        msgpack_pack_append_buffer(x, &TAKE8_64(d), 1); \
    } else { \
        if(d < (1LL<<16)) { \
            if(d < (1<<8)) { \
                /* unsigned 8 */ \
                unsigned char buf[2] = {0xcc, TAKE8_64(d)}; \
                msgpack_pack_append_buffer(x, buf, 2); \
            } else { \
                /* unsigned 16 */ \
                unsigned char buf[3]; \
                buf[0] = 0xcd; _msgpack_store16(&buf[1], (uint16_t)d); \
                msgpack_pack_append_buffer(x, buf, 3); \
            } \
        } else { \
            if(d < (1LL<<32)) { \
                /* unsigned 32 */ \
                unsigned char buf[5]; \
                buf[0] = 0xce; _msgpack_store32(&buf[1], (uint32_t)d); \
                msgpack_pack_append_buffer(x, buf, 5); \
            } else { \
                /* unsigned 64 */ \
                unsigned char buf[9]; \
                buf[0] = 0xcf; _msgpack_store64(&buf[1], d); \
                msgpack_pack_append_buffer(x, buf, 9); \
            } \
        } \
    } \
} while(0)


static inline int msgpack_pack_uint8(msgpack_packer* x, uint8_t d)
{
    msgpack_pack_real_uint8(x, d);
}

static inline int msgpack_pack_uint16(msgpack_packer* x, uint16_t d)
{
    msgpack_pack_real_uint16(x, d);
}

static inline int msgpack_pack_uint32(msgpack_packer* x, uint32_t d)
{
    msgpack_pack_real_uint32(x, d);
}

static inline int msgpack_pack_uint64(msgpack_packer* x, uint64_t d)
{
    msgpack_pack_real_uint64(x, d);
}

static inline int msgpack_pack_int8(msgpack_packer* x, int8_t d)
{
    msgpack_pack_real_int8(x, d);
}

static inline int msgpack_pack_int16(msgpack_packer* x, int16_t d)
{
    msgpack_pack_real_int16(x, d);
}

static inline int msgpack_pack_int32(msgpack_packer* x, int32_t d)
{
    msgpack_pack_real_int32(x, d);
}

static inline int msgpack_pack_int64(msgpack_packer* x, int64_t d)
{
    msgpack_pack_real_int64(x, d);
}


//#ifdef msgpack_pack_inline_func_cint

static inline int msgpack_pack_short(msgpack_packer* x, short d)
{
#if defined(SIZEOF_SHORT)
#if SIZEOF_SHORT == 2
    msgpack_pack_real_int16(x, d);
#elif SIZEOF_SHORT == 4
    msgpack_pack_real_int32(x, d);
#else
    msgpack_pack_real_int64(x, d);
#endif

#elif defined(SHRT_MAX)
#if SHRT_MAX == 0x7fff
    msgpack_pack_real_int16(x, d);
#elif SHRT_MAX == 0x7fffffff
    msgpack_pack_real_int32(x, d);
#else
    msgpack_pack_real_int64(x, d);
#endif

#else
if(sizeof(short) == 2) {
    msgpack_pack_real_int16(x, d);
} else if(sizeof(short) == 4) {
    msgpack_pack_real_int32(x, d);
} else {
    msgpack_pack_real_int64(x, d);
}
#endif
}

static inline int msgpack_pack_int(msgpack_packer* x, int d)
{
#if defined(SIZEOF_INT)
#if SIZEOF_INT == 2
    msgpack_pack_real_int16(x, d);
#elif SIZEOF_INT == 4
    msgpack_pack_real_int32(x, d);
#else
    msgpack_pack_real_int64(x, d);
#endif

#elif defined(INT_MAX)
#if INT_MAX == 0x7fff
    msgpack_pack_real_int16(x, d);
#elif INT_MAX == 0x7fffffff
    msgpack_pack_real_int32(x, d);
#else
    msgpack_pack_real_int64(x, d);
#endif

#else
if(sizeof(int) == 2) {
    msgpack_pack_real_int16(x, d);
} else if(sizeof(int) == 4) {
    msgpack_pack_real_int32(x, d);
} else {
    msgpack_pack_real_int64(x, d);
}
#endif
}

static inline int msgpack_pack_long(msgpack_packer* x, long d)
{
#if defined(SIZEOF_LONG)
#if SIZEOF_LONG == 2
    msgpack_pack_real_int16(x, d);
#elif SIZEOF_LONG == 4
    msgpack_pack_real_int32(x, d);
#else
    msgpack_pack_real_int64(x, d);
#endif

#elif defined(LONG_MAX)
#if LONG_MAX == 0x7fffL
    msgpack_pack_real_int16(x, d);
#elif LONG_MAX == 0x7fffffffL
    msgpack_pack_real_int32(x, d);
#else
    msgpack_pack_real_int64(x, d);
#endif

#else
if(sizeof(long) == 2) {
    msgpack_pack_real_int16(x, d);
} else if(sizeof(long) == 4) {
    msgpack_pack_real_int32(x, d);
} else {
    msgpack_pack_real_int64(x, d);
}
#endif
}

static inline int msgpack_pack_long_long(msgpack_packer* x, long long d)
{
#if defined(SIZEOF_LONG_LONG)
#if SIZEOF_LONG_LONG == 2
    msgpack_pack_real_int16(x, d);
#elif SIZEOF_LONG_LONG == 4
    msgpack_pack_real_int32(x, d);
#else
    msgpack_pack_real_int64(x, d);
#endif

#elif defined(LLONG_MAX)
#if LLONG_MAX == 0x7fffL
    msgpack_pack_real_int16(x, d);
#elif LLONG_MAX == 0x7fffffffL
    msgpack_pack_real_int32(x, d);
#else
    msgpack_pack_real_int64(x, d);
#endif

#else
if(sizeof(long long) == 2) {
    msgpack_pack_real_int16(x, d);
} else if(sizeof(long long) == 4) {
    msgpack_pack_real_int32(x, d);
} else {
    msgpack_pack_real_int64(x, d);
}
#endif
}

static inline int msgpack_pack_unsigned_short(msgpack_packer* x, unsigned short d)
{
#if defined(SIZEOF_SHORT)
#if SIZEOF_SHORT == 2
    msgpack_pack_real_uint16(x, d);
#elif SIZEOF_SHORT == 4
    msgpack_pack_real_uint32(x, d);
#else
    msgpack_pack_real_uint64(x, d);
#endif

#elif defined(USHRT_MAX)
#if USHRT_MAX == 0xffffU
    msgpack_pack_real_uint16(x, d);
#elif USHRT_MAX == 0xffffffffU
    msgpack_pack_real_uint32(x, d);
#else
    msgpack_pack_real_uint64(x, d);
#endif

#else
if(sizeof(unsigned short) == 2) {
    msgpack_pack_real_uint16(x, d);
} else if(sizeof(unsigned short) == 4) {
    msgpack_pack_real_uint32(x, d);
} else {
    msgpack_pack_real_uint64(x, d);
}
#endif
}

static inline int msgpack_pack_unsigned_int(msgpack_packer* x, unsigned int d)
{
#if defined(SIZEOF_INT)
#if SIZEOF_INT == 2
    msgpack_pack_real_uint16(x, d);
#elif SIZEOF_INT == 4
    msgpack_pack_real_uint32(x, d);
#else
    msgpack_pack_real_uint64(x, d);
#endif

#elif defined(UINT_MAX)
#if UINT_MAX == 0xffffU
    msgpack_pack_real_uint16(x, d);
#elif UINT_MAX == 0xffffffffU
    msgpack_pack_real_uint32(x, d);
#else
    msgpack_pack_real_uint64(x, d);
#endif

#else
if(sizeof(unsigned int) == 2) {
    msgpack_pack_real_uint16(x, d);
} else if(sizeof(unsigned int) == 4) {
    msgpack_pack_real_uint32(x, d);
} else {
    msgpack_pack_real_uint64(x, d);
}
#endif
}

static inline int msgpack_pack_unsigned_long(msgpack_packer* x, unsigned long d)
{
#if defined(SIZEOF_LONG)
#if SIZEOF_LONG == 2
    msgpack_pack_real_uint16(x, d);
#elif SIZEOF_LONG == 4
    msgpack_pack_real_uint32(x, d);
#else
    msgpack_pack_real_uint64(x, d);
#endif

#elif defined(ULONG_MAX)
#if ULONG_MAX == 0xffffUL
    msgpack_pack_real_uint16(x, d);
#elif ULONG_MAX == 0xffffffffUL
    msgpack_pack_real_uint32(x, d);
#else
    msgpack_pack_real_uint64(x, d);
#endif

#else
if(sizeof(unsigned long) == 2) {
    msgpack_pack_real_uint16(x, d);
} else if(sizeof(unsigned long) == 4) {
    msgpack_pack_real_uint32(x, d);
} else {
    msgpack_pack_real_uint64(x, d);
}
#endif
}

static inline int msgpack_pack_unsigned_long_long(msgpack_packer* x, unsigned long long d)
{
#if defined(SIZEOF_LONG_LONG)
#if SIZEOF_LONG_LONG == 2
    msgpack_pack_real_uint16(x, d);
#elif SIZEOF_LONG_LONG == 4
    msgpack_pack_real_uint32(x, d);
#else
    msgpack_pack_real_uint64(x, d);
#endif

#elif defined(ULLONG_MAX)
#if ULLONG_MAX == 0xffffUL
    msgpack_pack_real_uint16(x, d);
#elif ULLONG_MAX == 0xffffffffUL
    msgpack_pack_real_uint32(x, d);
#else
    msgpack_pack_real_uint64(x, d);
#endif

#else
if(sizeof(unsigned long long) == 2) {
    msgpack_pack_real_uint16(x, d);
} else if(sizeof(unsigned long long) == 4) {
    msgpack_pack_real_uint32(x, d);
} else {
    msgpack_pack_real_uint64(x, d);
}
#endif
}

//#undef msgpack_pack_inline_func_cint
//#endif



/*
 * Float
 */

static inline int msgpack_pack_float(msgpack_packer* x, float d)
{
    union { float f; uint32_t i; } mem;
    mem.f = d;
    unsigned char buf[5];
    buf[0] = 0xca; _msgpack_store32(&buf[1], mem.i);
    msgpack_pack_append_buffer(x, buf, 5);
}

static inline int msgpack_pack_double(msgpack_packer* x, double d)
{
    union { double f; uint64_t i; } mem;
    mem.f = d;
    unsigned char buf[9];
    buf[0] = 0xcb;
#if defined(__arm__) && !(__ARM_EABI__) // arm-oabi
    // https://github.com/msgpack/msgpack-perl/pull/1
    mem.i = (mem.i & 0xFFFFFFFFUL) << 32UL | (mem.i >> 32UL);
#endif
    _msgpack_store64(&buf[1], mem.i);
    msgpack_pack_append_buffer(x, buf, 9);
}


/*
 * Nil
 */

static inline int msgpack_pack_nil(msgpack_packer* x)
{
    static const unsigned char d = 0xc0;
    msgpack_pack_append_buffer(x, &d, 1);
}


/*
 * Boolean
 */

static inline int msgpack_pack_true(msgpack_packer* x)
{
    static const unsigned char d = 0xc3;
    msgpack_pack_append_buffer(x, &d, 1);
}

static inline int msgpack_pack_false(msgpack_packer* x)
{
    static const unsigned char d = 0xc2;
    msgpack_pack_append_buffer(x, &d, 1);
}


/*
 * Array
 */


static inline int msgpack_clear_spot_for_start_byte(msgpack_packer* x)
{
    unsigned char d = 0x00;
    msgpack_pack_append_buffer(x, &d, 1);

}
static inline int msgpack_pack_array(msgpack_packer* x, size_t l, size_t position)
{
    if(l < 56) {
        unsigned char d = 0xc0 | (uint8_t)l;
        msgpack_pack_insert_buffer(x, &d, 1, position);
    } else if ( l < 256) {
        unsigned char buf[2] = {0xf8, (uint8_t)l};
        msgpack_pack_insert_buffer(x, buf, 2, position);
    } else if (l < 65536) {
        unsigned char buf[3];
        buf[0] = 0xf9; _msgpack_store16(&buf[1], (uint16_t)l);
        msgpack_pack_insert_buffer(x, buf, 3, position);
    } else if (l < 16777216){
        //24 bit, 1 null bit to remove from 32 bit
        unsigned char buf[4];
        //uint32_t length_in_bytes = (uint32_t)l;
        buf[0] = 0xfa;
        unsigned char padded_buf[4];
        //memcpy(&padded_buf[0], &length_in_bytes, 5);
        _msgpack_store32(&padded_buf[0], (uint32_t)l);

        buf[1] = padded_buf[1];
        buf[2] = padded_buf[2];
        buf[3] = padded_buf[3];
        //buf[4] = padded_buf[0];

        msgpack_pack_insert_buffer(x, buf, 4, position);
    } else if (l < 4294967296){
        //32 bit
        unsigned char buf[5];
        buf[0] = 0xfb; _msgpack_store32(&buf[1], (uint32_t)l);
        msgpack_pack_insert_buffer(x, buf, 5, position);

    } else if (l < 1099511627776){
        //40 bit, 3 null bit to remove from 64 bit
        unsigned char buf[6];
        //uint64_t length_in_bytes = (uint64_t)l;
        buf[0] = 0xfc;
        unsigned char padded_buf[8];
        //memcpy(&padded_buf[0], &length_in_bytes, 9);
        _msgpack_store64(&padded_buf[0], (uint64_t)l);

        buf[1] = padded_buf[3];
        buf[2] = padded_buf[4];
        buf[3] = padded_buf[5];
        buf[4] = padded_buf[6];
        buf[5] = padded_buf[7];

        msgpack_pack_insert_buffer(x, buf, 6, position);
    } else if (l < 281474976710656){
        //48 bit, 2 null bit to remove from 64 bit
        unsigned char buf[7];
        //uint64_t length_in_bytes = (uint64_t)l;
        buf[0] = 0xfd;
        unsigned char padded_buf[8];
        //memcpy(&padded_buf[0], &length_in_bytes, 9);
        _msgpack_store64(&padded_buf[0], (uint64_t)l);

        buf[1] = padded_buf[2];
        buf[2] = padded_buf[3];
        buf[3] = padded_buf[4];
        buf[4] = padded_buf[5];
        buf[5] = padded_buf[6];
        buf[6] = padded_buf[7];

        msgpack_pack_insert_buffer(x, buf, 7, position);
    } else if (l < 72057594037927936){
        //56 bit, 1 null bit to remove from 64 bit
        unsigned char buf[8];
        //uint64_t length_in_bytes = (uint64_t)l;
        buf[0] = 0xfe;
        unsigned char padded_buf[8];
        //memcpy(&padded_buf[0], &length_in_bytes, 9);
        _msgpack_store64(&padded_buf[0], (uint64_t)l);

        buf[1] = padded_buf[1];
        buf[2] = padded_buf[2];
        buf[3] = padded_buf[3];
        buf[4] = padded_buf[4];
        buf[5] = padded_buf[5];
        buf[6] = padded_buf[6];
        buf[7] = padded_buf[7];

        msgpack_pack_insert_buffer(x, buf, 8, position);
    } else {
        unsigned char buf[9];
        buf[0] = 0xff; _msgpack_store64(&buf[1], (uint64_t)l);
        msgpack_pack_insert_buffer(x, buf, 9, position);
    }





//    else if(n < 65536) {
//        unsigned char buf[3];
//        buf[0] = 0xdc; _msgpack_store16(&buf[1], (uint16_t)n);
//        msgpack_pack_append_buffer(x, buf, 3);
//    } else {
//        unsigned char buf[5];
//        buf[0] = 0xdd; _msgpack_store32(&buf[1], (uint32_t)n);
//        msgpack_pack_append_buffer(x, buf, 5);
//    }
}


/*
 * Map
 */

static inline int msgpack_pack_map(msgpack_packer* x, unsigned int n)
{
    if(n < 16) {
        unsigned char d = 0x80 | n;
        msgpack_pack_append_buffer(x, &TAKE8_8(d), 1);
    } else if(n < 65536) {
        unsigned char buf[3];
        buf[0] = 0xde; _msgpack_store16(&buf[1], (uint16_t)n);
        msgpack_pack_append_buffer(x, buf, 3);
    } else {
        unsigned char buf[5];
        buf[0] = 0xdf; _msgpack_store32(&buf[1], (uint32_t)n);
        msgpack_pack_append_buffer(x, buf, 5);
    }
}


/*
 * Raw
 */

/*
 *#int len_len = ceil((floor(log2(l)) + 1)/8);
 *#int first_digit = len_len + 55 + 128;
 *
 */
static inline int msgpack_pack_raw(msgpack_packer* x, size_t l)
{


    if (l < 56) {
        unsigned char d = 0x80 | (uint8_t)l;
        msgpack_pack_append_buffer(x, &TAKE8_8(d), 1);
    } else if ( l < 256) {
        unsigned char buf[2] = {0xb8, (uint8_t)l};
        msgpack_pack_append_buffer(x, buf, 2);
    } else if (l < 65536) {
        unsigned char buf[3];
        buf[0] = 0xb9; _msgpack_store16(&buf[1], (uint16_t)l);
        msgpack_pack_append_buffer(x, buf, 3);
    } else if (l < 16777216){
        //24 bit, 1 null bit to remove from 32 bit
        unsigned char buf[4];
        //uint32_t length_in_bytes = (uint32_t)l;
        buf[0] = 0xba;
        unsigned char padded_buf[4];
        //memcpy(&padded_buf[0], &length_in_bytes, 4);
        _msgpack_store32(&padded_buf[0], (uint32_t)l);

        buf[1] = padded_buf[1];
        buf[2] = padded_buf[2];
        buf[3] = padded_buf[3];
        //buf[4] = padded_buf[0];

        msgpack_pack_append_buffer(x, buf, 4);
    } else if (l < 4294967296){
        //32 bit
        unsigned char buf[5];
        buf[0] = 0xbb; _msgpack_store32(&buf[1], (uint32_t)l);
        msgpack_pack_append_buffer(x, buf, 5);

    } else if (l < 1099511627776){
        //40 bit, 3 null bit to remove from 64 bit
        unsigned char buf[6];
        //uint64_t length_in_bytes = (uint64_t)l;
        buf[0] = 0xbc;
        unsigned char padded_buf[8];
        //memcpy(&padded_buf[0], &length_in_bytes, 9);
        _msgpack_store64(&padded_buf[0], (uint64_t)l);

        buf[1] = padded_buf[3];
        buf[2] = padded_buf[4];
        buf[3] = padded_buf[5];
        buf[4] = padded_buf[6];
        buf[5] = padded_buf[7];

        msgpack_pack_append_buffer(x, buf, 6);
    } else if (l < 281474976710656){
        //48 bit, 2 null bit to remove from 64 bit
        unsigned char buf[7];
        //uint64_t length_in_bytes = (uint64_t)l;
        buf[0] = 0xbd;
        unsigned char padded_buf[8];
        //memcpy(&padded_buf[0], &length_in_bytes, 9);
        _msgpack_store64(&padded_buf[0], (uint64_t)l);

        buf[1] = padded_buf[2];
        buf[2] = padded_buf[3];
        buf[3] = padded_buf[4];
        buf[4] = padded_buf[5];
        buf[5] = padded_buf[6];
        buf[6] = padded_buf[7];

        msgpack_pack_append_buffer(x, buf, 7);
    } else if (l < 72057594037927936){
        //56 bit, 1 null bit to remove from 64 bit
        unsigned char buf[8];
        //uint64_t length_in_bytes = (uint64_t)l;
        buf[0] = 0xbe;
        unsigned char padded_buf[8];
        //memcpy(&padded_buf[0], &length_in_bytes, 9);
        _msgpack_store64(&padded_buf[0], (uint64_t)l);

        buf[1] = padded_buf[1];
        buf[2] = padded_buf[2];
        buf[3] = padded_buf[3];
        buf[4] = padded_buf[4];
        buf[5] = padded_buf[5];
        buf[6] = padded_buf[6];
        buf[7] = padded_buf[7];

        msgpack_pack_append_buffer(x, buf, 8);
    } else {
        unsigned char buf[9];
        buf[0] = 0xbf; _msgpack_store64(&buf[1], (uint64_t)l);
        msgpack_pack_append_buffer(x, buf, 9);
    }


//       if (l < 56) {
//        unsigned char d = 0x80 | (uint8_t)l;
//        msgpack_pack_append_buffer(x, &TAKE8_8(d), 1);
//    }else if ( l < 256) {
//        unsigned char buf[2] = {0xb8, (uint8_t)l};
//        msgpack_pack_append_buffer(x, buf, 2);
//    } else if (l < 65536) {
//        unsigned char buf[3];
//        buf[0] = 0xb9; _msgpack_store16(&buf[1], (uint16_t)l);
//        msgpack_pack_append_buffer(x, buf, 3);
//    } else if (l < 16777216){
//        //24 bit
//        unsigned char buf[4];
//        unsigned char test = (uint32_t)l;
//        buf[0] = 0xba; memcpy(&buf[1], &test, 3);
//        msgpack_pack_append_buffer(x, buf, 4);
//    } else if (l < 4294967296){
//        //32 bit
//        unsigned char buf[5];
//        buf[0] = 0xba; _msgpack_store32(&buf[1], (uint32_t)l);
//        msgpack_pack_append_buffer(x, buf, 5);
//    } else {
//        unsigned char buf[9];
//        buf[0] = 0xbb; _msgpack_store64(&buf[1], (uint64_t)l);
//        msgpack_pack_append_buffer(x, buf, 9);
//    }


//    if (l < 56) {
//        unsigned char d = 0x80 | (uint8_t)l;
//        msgpack_pack_append_buffer(x, &TAKE8_8(d), 1);
//    }else if ( l < 256) {
//        unsigned char buf[2] = {0xb8, (uint8_t)l};
//        msgpack_pack_append_buffer(x, buf, 2);
//    } else if (l < 65536) {
//        unsigned char buf[3];
//        buf[0] = 0xb9; _msgpack_store16(&buf[1], (uint16_t)l);
//        msgpack_pack_append_buffer(x, buf, 3);
//    } else if (l < 16777216){
//        unsigned char buf[4];
//        buf[0] = 0xba;
//        unsigned char extra_buf[3];
//        _msgpack_store32(&extra_buf[0], (uint32_t)l);
//        memcpy(extra_buf[1], &buf, )
//        msgpack_pack_append_buffer(x, buf, 5);
//    } else if (l < 4294967296){
//        unsigned char buf[4];
//        buf[0] = 0xba; _msgpack_store32(&buf[1], (uint32_t)l);
//        msgpack_pack_append_buffer(x, buf, 5);
//    } else {
//        unsigned char buf[9];
//        buf[0] = 0xbb; _msgpack_store64(&buf[1], (uint64_t)l);
//        msgpack_pack_append_buffer(x, buf, 9);
//    }


    if (l < 56) {
        unsigned char d = 0x80 | (uint8_t)l;
        msgpack_pack_append_buffer(x, &TAKE8_8(d), 1);
    }else if ( l < 256) {
        unsigned char buf[2] = {0xb8, (uint8_t)l};
        msgpack_pack_append_buffer(x, buf, 2);
    } else if (l < 65536) {
        unsigned char buf[3];
        buf[0] = 0xb9; _msgpack_store16(&buf[1], (uint16_t)l);
        msgpack_pack_append_buffer(x, buf, 3);
    } else if (l < 4294967296){
        unsigned char buf[5];
        buf[0] = 0xba; _msgpack_store32(&buf[1], (uint32_t)l);
        msgpack_pack_append_buffer(x, buf, 5);
    } else {
        unsigned char buf[9];
        buf[0] = 0xbb; _msgpack_store64(&buf[1], (uint64_t)l);
        msgpack_pack_append_buffer(x, buf, 9);
    }
}

/*
 * bin
 */
static inline int msgpack_pack_bin(msgpack_packer *x, size_t l)
{
    if (!x->use_bin_type) {
        return msgpack_pack_raw(x, l);
    }
    if (l < 256) {
        unsigned char buf[2] = {0xc4, (unsigned char)l};
        msgpack_pack_append_buffer(x, buf, 2);
    } else if (l < 65536) {
        unsigned char buf[3] = {0xc5};
        _msgpack_store16(&buf[1], (uint16_t)l);
        msgpack_pack_append_buffer(x, buf, 3);
    } else {
        unsigned char buf[5] = {0xc6};
        _msgpack_store32(&buf[1], (uint32_t)l);
        msgpack_pack_append_buffer(x, buf, 5);
    }
}

static inline int msgpack_pack_raw_body(msgpack_packer* x, const void* b, size_t l)
{
    if (l > 0) msgpack_pack_append_buffer(x, (const unsigned char*)b, l);
    return 0;
}

/*
 * Ext
 */
static inline int msgpack_pack_ext(msgpack_packer* x, char typecode, size_t l)
{
    if (l == 1) {
        unsigned char buf[2];
        buf[0] = 0xd4;
        buf[1] = (unsigned char)typecode;
        msgpack_pack_append_buffer(x, buf, 2);
    }
    else if(l == 2) {
        unsigned char buf[2];
        buf[0] = 0xd5;
        buf[1] = (unsigned char)typecode;
        msgpack_pack_append_buffer(x, buf, 2);
    }
    else if(l == 4) {
        unsigned char buf[2];
        buf[0] = 0xd6;
        buf[1] = (unsigned char)typecode;
        msgpack_pack_append_buffer(x, buf, 2);
    }
    else if(l == 8) {
        unsigned char buf[2];
        buf[0] = 0xd7;
        buf[1] = (unsigned char)typecode;
        msgpack_pack_append_buffer(x, buf, 2);
    }
    else if(l == 16) {
        unsigned char buf[2];
        buf[0] = 0xd8;
        buf[1] = (unsigned char)typecode;
        msgpack_pack_append_buffer(x, buf, 2);
    }
    else if(l < 256) {
        unsigned char buf[3];
        buf[0] = 0xc7;
        buf[1] = l;
        buf[2] = (unsigned char)typecode;
        msgpack_pack_append_buffer(x, buf, 3);
    } else if(l < 65536) {
        unsigned char buf[4];
        buf[0] = 0xc8;
        _msgpack_store16(&buf[1], (uint16_t)l);
        buf[3] = (unsigned char)typecode;
        msgpack_pack_append_buffer(x, buf, 4);
    } else {
        unsigned char buf[6];
        buf[0] = 0xc9;
        _msgpack_store32(&buf[1], (uint32_t)l);
        buf[5] = (unsigned char)typecode;
        msgpack_pack_append_buffer(x, buf, 6);
    }

}



#undef msgpack_pack_append_buffer

#undef TAKE8_8
#undef TAKE8_16
#undef TAKE8_32
#undef TAKE8_64

#undef msgpack_pack_real_uint8
#undef msgpack_pack_real_uint16
#undef msgpack_pack_real_uint32
#undef msgpack_pack_real_uint64
#undef msgpack_pack_real_int8
#undef msgpack_pack_real_int16
#undef msgpack_pack_real_int32
#undef msgpack_pack_real_int64
