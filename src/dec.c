#include <fox.h>

static void fox_in(struct fox *fox) {
    fox->input = (fox->input & 0xFFFF) << 8 | fox->callback(0, fox->user);
    fox->lower = (fox->lower & 0xFFFF) << 8;
    fox->upper = (fox->upper & 0xFFFF) << 8;
}

static unsigned fox_dec(struct fox *fox, unsigned ctx, unsigned n) {
    for (unsigned sym = 1, top = 1 << n;;) {
        if (sym >= top) return sym - top;

        while ((fox->lower ^ fox->upper) >= 0xFF0000) fox_in(fox);

        signed char *mod = fox->model + ctx + sym - 1, prb = *mod;
        unsigned long rng = 0xFFFFFF - fox->lower - fox->upper;
        unsigned long mid = rng * (prb + 128) >> 8;

        sym <<= 1;
        sym |= fox->input <= fox->lower + mid
            ? (fox->upper += rng - mid, *mod = prb + ((128 - prb) >> 3), 1)
            : (fox->lower += mid + 1,   *mod = prb - ((128 + prb) >> 3), 0);
    }
}

void fox_open(struct fox *fox) {
    fox_in(fox), fox_in(fox), fox_in(fox);
}

unsigned long fox_read(struct fox *fox) {
    if (fox->run) return --fox->run, fox->color;

    unsigned sym = fox_dec(fox, 0x000, 9);
    if (sym >= 384) return fox->color = fox->cache[sym - 384];
    if (sym >= 256) return fox->run = sym - 256, fox->color;

    unsigned long prev = fox->color, col = 0xFF & (prev + sym);
    col |= (0xFF & ((prev >>  8) + fox_dec(fox, 0x1FF, 8) + sym)) <<  8;
    col |= (0xFF & ((prev >> 16) + fox_dec(fox, 0x2FE, 8) + sym)) << 16;
    col |= (0xFF & ((prev >> 24) + fox_dec(fox, 0x3FD, 8)      )) << 24;

    return fox->cache[((col * 0x57B70101) >> 25) & 0x7F] = fox->color = col;
}
