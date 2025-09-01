#pragma once
#ifndef fox_h_
#define fox_h_

#ifdef __cplusplus
extern "C" {
#endif

struct fox_argb { unsigned char a, r, g, b; };

struct fox {
    // user-defined
    unsigned char (*callback) (unsigned char arg, void *user);
    void *user;

    // zero-initialized
    unsigned long lower, upper, input;
    unsigned int run;
    struct fox_argb cache[0x80], color;
    signed char model[0x4FC];
};

// encoding
void fox_write(struct fox *fox, struct fox_argb color);
void fox_close(struct fox *fox);

// decoding
void fox_open(struct fox *fox);
struct fox_argb fox_read(struct fox *fox);

#ifdef __cplusplus
}
#endif

#endif
