#include <fox.h>

static inline void fox_dec_renorm(fox_coder_t *dec) {
    dec->input <<= 8, dec->input |= dec->stream->read(dec->stream);
    dec->lower <<= 8, dec->range <<= 8, dec->range |= 0xFF;
}

static fox_u8_t fox_dec_rc_bit(fox_coder_t *dec, fox_u32_t ctx) {
    while (dec->lower >> 24 == (dec->lower + dec->range) >> 24)
        fox_dec_renorm(dec);

    fox_u8_t  prb = (dec->model[ctx]);
    fox_u32_t mid = (dec->range >> 8) * prb;

    if (dec->input - dec->lower <= mid) {
        dec->model[ctx] = prb + ((0x100 - prb) >> 3);
        dec->range = mid;
        return 1;
    } else {
        dec->model[ctx] = prb - (prb >> 3);
        dec->lower += mid + 1;
        dec->range -= mid + 1;
        return 0;
    }
}

static fox_u32_t fox_dec_rc(fox_coder_t *dec, fox_u32_t ctx, fox_u8_t len) {
    fox_u32_t sym = 1;
    while (len--) sym = sym << 1 | fox_dec_rc_bit(dec, ctx + sym - 1);
    return sym;
}

void fox_dec_open(fox_coder_t *dec, fox_stream_t *stream) {
    dec->stream     = stream;
    dec->color.argb = 0xFF000000;
    dec->range      = 0x00000000;
    dec->lower      = 0x00000000;
    dec->input      = 0x00000000;
    dec->run        = 0;

    for (fox_u32_t i = 0; i < 0x080; ++i) dec->cache[i].argb = 0xFF000000;
    for (fox_u32_t i = 0; i < 0x4FC; ++i) dec->model[i]      = 0x7F;
    for (fox_u8_t i = 0; i < 4; ++i) fox_dec_renorm(dec);
}

fox_color_t fox_dec_read(fox_coder_t *dec) {
    if (!dec->run) {
        fox_u32_t sym = fox_dec_rc(dec, 0, 9);

        if (sym >= 896) dec->color = dec->cache[sym - 896];
        else if (sym >= 768) dec->run = sym - 768;
        else {
            dec->color.g += sym;
            dec->color.r += fox_dec_rc(dec, 0x1FF, 8) + sym;
            dec->color.b += fox_dec_rc(dec, 0x2FE, 8) + sym;
            dec->color.a += fox_dec_rc(dec, 0x3FD, 8);
            dec->cache[dec->color.argb * 0xF86FEBF5 >> 25 & 0x7F] = dec->color;
        }
    } else --dec->run;

    return dec->color;
}
