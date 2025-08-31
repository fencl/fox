#include <fox.h>

static void fox_out(struct fox *fox) {
    fox->callback(fox->lower >> 8, fox->user);
    fox->lower <<= 8, fox->upper <<= 8;
}

static void fox_enc_bit(struct fox *fox, unsigned int bit, unsigned int ctx) {
    while ((fox->lower ^ fox->upper) >= 0xFF00u) fox_out(fox);

    signed char *mod = fox->model + ctx, prb = *mod;
    unsigned int rng = 0xFFFFu - fox->lower - fox->upper;
    unsigned int mid = (unsigned long) rng * (prb + 128) >> 8;

    if (bit) fox->upper += rng - mid, *mod = prb + ((128 - prb) >> 3);
        else fox->lower += mid + 1,   *mod = prb - ((128 + prb) >> 3);
}

static void fox_enc(struct fox *fox, unsigned int mod, unsigned int sym) {
    unsigned int len = mod & 0xF, ctx = mod >> 4, off = (1 << len | sym) >> 1;
    while (len--) fox_enc_bit(fox, sym >> len & 1, ctx + (off >> len) - 1);
}

static void fox_flush_run(struct fox *fox) {
    fox_enc(fox, 0x0009, 255 + fox->run), fox->run = 0;
}

void fox_write(struct fox *fox, struct fox_argb color) {
    unsigned int a = color.a, r = color.r, g = color.g, b = color.b;
    struct fox_argb prev = fox->color;

    if (prev.a != a || prev.r != r || prev.g != g || prev.b != b) {
        if (fox->run) fox_flush_run(fox);

        unsigned int hash = (r + g * 3 + b * 5 + a * 7) & 0x7F;
        struct fox_argb cache = fox->cache[hash];

        if (cache.a != a || cache.r != r || cache.g != g || cache.b != b) {
            fox_enc(fox, 0x0009, 0xFF & (g -= prev.g));
            fox_enc(fox, 0x1FF8, 0xFF & (r -= prev.r + g));
            fox_enc(fox, 0x2FE8, 0xFF & (b -= prev.b + g));
            fox_enc(fox, 0x3FD8, 0xFF & (a -= prev.a));

            fox->cache[hash] = color;
        } else fox_enc(fox, 0x0009, 384 + hash);

        fox->color = color;
    } else if (++fox->run == 128) fox_flush_run(fox);
}

void fox_close(struct fox *fox) {
    if (fox->run) fox_flush_run(fox);
    fox_out(fox), fox_out(fox);
}
