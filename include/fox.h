#pragma once
#ifndef fox_h_
#define fox_h_

#ifdef __cplusplus
extern "C" {
#endif

struct fox {
    // user-defined
    unsigned char (*callback) (unsigned char arg, void *user);
    void *user;

    // zero-initialized
    unsigned long lower, upper, input, cache[0x80], color;
    unsigned int run;
    signed char model[0x4FC];
};

// encoding
void fox_write(struct fox *fox, unsigned long col);
void fox_close(struct fox *fox);

// decoding
void fox_open(struct fox *fox);
unsigned long fox_read(struct fox *fox);

#ifdef __cplusplus
}
#endif

#endif
