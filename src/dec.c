#include <fox.h>

static void fox_in(struct fox *fox) {
    fox->input = (fox->input & 0xFFFF) << 8 | fox->callback(0, fox->user);
    fox->lower = (fox->lower & 0xFFFF) << 8;
    fox->upper = (fox->upper & 0xFFFF) << 8;
}

static unsigned int fox_dec_bit(struct fox *fox, unsigned int ctx) {
    while ((fox->lower ^ fox->upper) >= 0xFF0000) fox_in(fox);

    signed   char *mod = fox->model + ctx, prb = *mod;
    unsigned long  rng = 0xFFFFFF - fox->lower - fox->upper;
    unsigned long  mid = rng * (prb + 128) >> 8;

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
    fox_in(fox), fox_in(fox), fox_in(fox);
}

struct fox_argb fox_read(struct fox *fox) {
    if (fox->run) return --fox->run, fox->color;

    unsigned int sym = fox_dec(fox, 0x0009), a, r, g, b;
    if (sym >= 384) return fox->color = fox->cache[sym - 384];
    if (sym >= 256) return fox->run = sym - 256, fox->color;

    g = fox->color.g = 0xFF & (fox->color.g + sym);
    r = fox->color.r = 0xFF & (fox->color.r + fox_dec(fox, 0x1FF8) + sym);
    b = fox->color.b = 0xFF & (fox->color.b + fox_dec(fox, 0x2FE8) + sym);
    a = fox->color.a = 0xFF & (fox->color.a + fox_dec(fox, 0x3FD8));

    return fox->cache[(r + g * 3 + b * 5 + a * 7) & 0x7F] = fox->color;
}
