#include <fox.h>

static void fox_out(struct fox *fox) {
    fox->callback(fox->lower >> 16 & 0xFF, fox->user);
    fox->lower &= 0xFFFF, fox->upper &= 0xFFFF;
    fox->lower <<= 8,     fox->upper <<= 8;
}

static void fox_enc_bit(struct fox *fox, signed int bit, unsigned int ctx) {
    while (fox->lower >> 16 == 0xFF - (fox->upper >> 16)) fox_out(fox);

    signed   char *mod = fox->model + ctx, prb = *mod;
    unsigned long  rng = 0xFFFFFF - fox->lower - fox->upper;
    unsigned long  mid = rng * (prb + 128) >> 8;

    if (bit) fox->upper += rng - mid, *mod = prb + ((128 - prb) >> 3);
        else fox->lower += mid + 1,   *mod = prb - ((128 + prb) >> 3);
}

static void fox_enc(struct fox *fox, unsigned int sym, unsigned int ctx) {
    for (unsigned int i = 8 + (sym >> 9), off = sym >> 1; i--;)
        fox_enc_bit(fox, sym >> i & 1, ctx + (off >> i) - 1);
}

static void fox_flush_run(struct fox *fox) {
    fox_enc(fox, 0x200 | (255 + fox->run), 0), fox->run = 0;
}

void fox_write(struct fox *fox, unsigned long color) {
    unsigned long prev = fox->color;

    if (prev != color) {
        if (fox->run) fox_flush_run(fox);

        unsigned int i = color * 0x57B70101 >> 25 & 0x7F,
            r = color >> 16 & 0xFF, g = color >>  8 & 0xFF,
            b = color       & 0xFF, a = color >> 24 & 0xFF;

        unsigned char *cache = fox->cache[i];
        if (cache[0] != r || cache[1] != g || cache[2] != b || cache[3] != a) {
            unsigned int dg = 0xFF & ((cache[1] = g) - (prev >> 8));
            unsigned int dr = 0xFF & ((cache[0] = r) - (prev >> 16) - dg);
            unsigned int db = 0xFF & ((cache[2] = b) - (prev      ) - dg);
            unsigned int da = 0xFF & ((cache[3] = a) - (prev >> 24));

            fox_enc(fox, 0x200 | dg, 0x000);
            fox_enc(fox, 0x100 | dr, 0x1FF);
            fox_enc(fox, 0x100 | db, 0x2FE);
            fox_enc(fox, 0x100 | da, 0x3FD);
        } else fox_enc(fox, 0x200 | (384 + i), 0);

        fox->color = color;
    } else if (++fox->run == 128) fox_flush_run(fox);
}

void fox_close(struct fox *fox) {
    if (fox->run) fox_flush_run(fox);
    for (int i = 3; i--;) fox_out(fox);
}
