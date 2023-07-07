#include <fox.h>

static inline void fox_enc_u32(fox_stream_t *stream, fox_u32_t v) {
    for (fox_u8_t i = 0; i < 4; ++i) stream->write(stream, v & 0xFF), v >>= 8;
}

void fox_enc_open(fox_coder_t *enc, fox_stream_t *stream, fox_size_t size) {
    fox_enc_u32(stream, size.w);
    fox_enc_u32(stream, size.h);

    enc->stream     = stream;
    enc->color.argb = 0xFF000000;
    enc->lower      = 0x00000000;
    enc->range      = 0xFFFFFFFF;
    enc->run        = 0;

    for (fox_u32_t i = 0; i < 0x080; ++i) enc->cache[i].argb = 0xFF000000;
    for (fox_u32_t i = 0; i < 0x4FC; ++i) enc->model[i]      = 0x7F;
}

static inline void fox_enc_renorm(fox_coder_t *enc) {
    enc->stream->write(enc->stream, enc->lower >> 24);
    enc->lower <<= 8, enc->range <<= 8, enc->range |= 0xFF;
}

static void fox_enc_rc_bit(fox_coder_t *enc, fox_u8_t bit, fox_u32_t ctx) {
    while (enc->lower >> 24 == (enc->lower + enc->range) >> 24)
        fox_enc_renorm(enc);

    fox_u8_t  prb = (enc->model[ctx]);
    fox_u32_t mid = (enc->range >> 8) * prb;

    if (bit) {
        enc->model[ctx] = prb + ((0x100 - prb) >> 3);
        enc->range = mid;
    } else {
        enc->model[ctx] = prb - (prb >> 3);
        enc->lower += mid + 1;
        enc->range -= mid + 1;
    }
}

static void fox_enc_rc(fox_coder_t *enc, fox_u32_t sym, fox_u32_t ctx) {
    for (fox_u8_t i = 8 + (sym >> 9), j = i; i--; j--)
        fox_enc_rc_bit(enc, sym >> i & 1, ctx + (sym >> j) - 1);
}

static inline void fox_enc_flush_run(fox_coder_t *enc) {
    fox_enc_rc(enc, 0x200 | 255 + enc->run, 0), enc->run = 0;
}

void fox_enc_close(fox_coder_t *enc) {
    if (enc->run) fox_enc_flush_run(enc);
    for (fox_u8_t i = 0; i < 4; ++i) fox_enc_renorm(enc);
}

void fox_enc_write(fox_coder_t *enc, fox_color_t color) {
    fox_color_t prev = enc->color;
    if (prev.argb != color.argb) {
        if (enc->run) fox_enc_flush_run(enc);

        fox_u8_t hash = color.argb * 0xF86FEBF5 >> 25 & 0x7F;
        if (enc->cache[hash].argb != color.argb) {

            fox_u8_t diff_g = color.g - prev.g          & 0xFF;
            fox_u8_t diff_r = color.r - prev.r - diff_g & 0xFF;
            fox_u8_t diff_b = color.b - prev.b - diff_g & 0xFF;
            fox_u8_t diff_a = color.a - prev.a          & 0xFF;
            fox_enc_rc(enc, 0x200 | diff_g, 0x000);
            fox_enc_rc(enc, 0x100 | diff_r, 0x1FF);
            fox_enc_rc(enc, 0x100 | diff_b, 0x2FE);
            fox_enc_rc(enc, 0x100 | diff_a, 0x3FD);

            enc->cache[hash] = color;
        } else fox_enc_rc(enc, 0x200 | 384 + hash, 0);

        enc->color = color;
    } else if (++enc->run >> 7) fox_enc_flush_run(enc);
}
