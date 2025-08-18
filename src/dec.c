#include <fox.h>

static void fox_in(struct fox *fox) {
    fox->input &= 0xFFFF, fox->lower &= 0xFFFF, fox->upper &= 0xFFFF;
    fox->input <<= 8,     fox->lower <<= 8,     fox->upper <<= 8;
    fox->input |= fox->callback(0, fox->user);
}

static unsigned int fox_dec_bit(struct fox *fox, unsigned int ctx) {
    while (fox->lower >> 16 == 0xFF - (fox->upper >> 16)) fox_in(fox);

    signed   char *mod = fox->model + ctx, prb = *mod;
    unsigned long  rng = 0xFFFFFF - fox->lower - fox->upper;
    unsigned long  mid = rng * (prb + 128) >> 8;

    return fox->input <= fox->lower + mid
        ? (fox->upper += rng - mid, *mod = prb + ((128 - prb) >> 3), 1)
        : (fox->lower += mid + 1,   *mod = prb - ((128 + prb) >> 3), 0);
}

static unsigned fox_dec(struct fox *fox, unsigned int ctx, unsigned int len) {
    unsigned int sym = 1;
    while (len--) sym = sym << 1 | fox_dec_bit(fox, ctx + sym - 1);
    return sym;
}

void fox_open(struct fox *fox) {
    for (int i = 3; i--;) fox_in(fox);
}

unsigned long fox_read(struct fox *fox) {
    if (fox->run) return --fox->run, fox->color;

    unsigned int sym = fox_dec(fox, 0, 9);

    if (sym >= 896) {
        const unsigned char *color = fox->cache[sym - 896];
        unsigned long r = color[0], g = color[1], b = color[2], a = color[3];
        return fox->color = a << 24 | r << 16 | g << 8 | b;

    } else if (sym < 768) {
        unsigned long prev = fox->color;
        unsigned long g = 0xFF & ((prev >>  8) + sym);
        unsigned long r = 0xFF & ((prev >> 16) + fox_dec(fox, 0x1FF, 8) + sym);
        unsigned long b = 0xFF & ((prev      ) + fox_dec(fox, 0x2FE, 8) + sym);
        unsigned long a = 0xFF & ((prev >> 24) + fox_dec(fox, 0x3FD, 8));

        unsigned long color = fox->color = a << 24 | r << 16 | g << 8 | b;
        unsigned char *cache = fox->cache[color * 0x57B70101 >> 25 & 0x7F];
        return cache[0] = r, cache[1] = g, cache[2] = b, cache[3] = a, color;

    } else return fox->run = sym - 768, fox->color;
}
