#include <fox.h>

static void fox_out(struct fox *fox) {
    fox->callback(fox->lower >> 16, fox->user);
    fox->lower = (fox->lower & 0xFFFF) << 8;
    fox->upper = (fox->upper & 0xFFFF) << 8;
}

static void fox_enc(struct fox *fox, unsigned ctx, unsigned n, unsigned sym) {
    for (unsigned off = 1 << n | sym; n;) {
        while ((fox->lower ^ fox->upper) >= 0xFF0000) fox_out(fox);

        signed char *cell = fox->model + ctx + (off >> n) - 1, prb = *cell;
        unsigned long rng = 0xFFFFFF - fox->lower - fox->upper;
        unsigned long mid = rng * (prb + 128) >> 8;

        sym >> --n & 1
            ? (fox->upper += rng - mid, *cell = prb + ((128 - prb) >> 3))
            : (fox->lower += mid + 1,   *cell = prb - ((128 + prb) >> 3));
    }
}

void fox_write(struct fox *fox, unsigned long col) {
    if (fox->color != col) {
        if (fox->run) fox_enc(fox, 0x000, 9, 255 + fox->run), fox->run = 0;

        unsigned hash = ((col * 0x57B70101) >> 25) & 0x7F;
        if (fox->cache[hash] != col) {

            unsigned long prev = fox->color, sym = 0xFF & (col - prev);
            fox_enc(fox, 0x000, 9, sym);
            fox_enc(fox, 0x1FF, 8, 0xFF & ((col >>  8) - (prev >>  8) - sym));
            fox_enc(fox, 0x2FE, 8, 0xFF & ((col >> 16) - (prev >> 16) - sym));
            fox_enc(fox, 0x3FD, 8, 0xFF & ((col >> 24) - (prev >> 24)));

            fox->cache[hash] = col;
        } else fox_enc(fox, 0x000, 9, 384 + hash);

        fox->color = col;
    } else if (++fox->run == 128) {
        fox_enc(fox, 0x000, 9, 255 + fox->run), fox->run = 0;
    }
}

void fox_close(struct fox *fox) {
    if (fox->run) fox_enc(fox, 0x000, 9, 255 + fox->run);
    fox_out(fox), fox_out(fox), fox_out(fox);
}
