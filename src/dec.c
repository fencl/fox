#include <fox.h>

static void fox_in(struct fox *fox) {
    fox->input <<= 8, fox->lower <<= 8, fox->upper <<= 8;
    fox->input |= fox->callback(0, fox->user);
}

static unsigned int fox_dec_bit(struct fox *fox, unsigned int ctx) {
    while ((fox->lower ^ fox->upper) >= 0xFF00u) fox_in(fox);

    signed char *mod = fox->model + ctx, prb = *mod;
    unsigned int rng = 0xFFFFu - fox->lower - fox->upper;
    unsigned int mid = (unsigned long) rng * (prb + 128) >> 8;

    return fox->input <= fox->lower + mid
        ? (fox->upper += rng - mid, *mod = prb + ((128 - prb) >> 3), 1)
        : (fox->lower += mid + 1,   *mod = prb - ((128 + prb) >> 3), 0);
}

static unsigned int fox_dec(struct fox *fox, unsigned int mod) {
    unsigned int sym = 1, len = mod & 0xF, ctx = mod >> 4, top = 1 << len;
    while (sym < top) sym = sym << 1 | fox_dec_bit(fox, ctx + sym - 1);
    return sym - top;
}

void fox_open(struct fox *fox) {
    fox_in(fox), fox_in(fox);
}

struct fox_argb fox_read(struct fox *fox) {
    if (fox->run) return --fox->run, fox->color;

    unsigned int sym = fox_dec(fox, 0x0009);
    if (sym >= 384) return fox->color = fox->cache[sym - 384];
    if (sym >= 256) return fox->run = sym - 256, fox->color;

    unsigned int g = fox->color.g += sym;
    unsigned int r = fox->color.r += fox_dec(fox, 0x1FF8) + sym;
    unsigned int b = fox->color.b += fox_dec(fox, 0x2FE8) + sym;
    unsigned int a = fox->color.a += fox_dec(fox, 0x3FD8);

    return fox->cache[(r + g * 3 + b * 5 + a * 7) & 0x7F] = fox->color;
}
