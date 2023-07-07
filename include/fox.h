#pragma once
#ifndef _fox_h_
#define _fox_h_
#include <stdint.h>

typedef uint_fast32_t fox_u32_t;
typedef uint_least8_t fox_u8_t;

typedef union fox_color {
    struct { uint_least32_t argb: 32;               };
    struct { uint_least32_t b: 8, g: 8, r: 8, a: 8; };
} fox_color_t;

typedef union fox_stream {
    void     (*write) (union fox_stream *stream, fox_u8_t data);
    fox_u8_t (*read)  (union fox_stream *stream);
} fox_stream_t;

typedef struct fox_coder {
    fox_stream_t *stream;
    fox_color_t   cache[0x80];
    fox_color_t   color;
    fox_u32_t     lower;
    fox_u32_t     range;
    fox_u32_t     input;
    fox_u8_t      run;
    fox_u8_t      model[0x4FC];
} fox_coder_t;

typedef struct fox_size { fox_u32_t w, h; } fox_size_t;

void fox_enc_open(fox_coder_t *enc, fox_stream_t *stream, fox_size_t size);
void fox_enc_write(fox_coder_t *enc, fox_color_t color);
void fox_enc_close(fox_coder_t *enc);

fox_size_t fox_dec_open(fox_coder_t *dec, fox_stream_t *stream);
fox_color_t fox_dec_read(fox_coder_t *dec);

#endif
