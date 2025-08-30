#include <fox.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

// =============================================================================

static unsigned char encode_out(unsigned char arg, void *user) {
    return fputc(arg, user), 0;
}

static int main_encode(int argc, char **argv) {
    int res = 1;

    // expect input and output fname
    if (argc < 2) {
        fprintf(stderr, "Not enough arguments.");
        goto err0;
    }

    // open TGA input file
    FILE *in = fopen(argv[0], "rb");
    if (!in) {
        fprintf(stderr, "failed to open '%s' for reading\n", argv[0]);
        goto err0;
    }

    // read TGA header
    unsigned char hdr[18];
    fread(hdr, 18, 1, in);

    unsigned char type = hdr[2];
    unsigned int w = hdr[12] | hdr[13] << 8;
    unsigned int h = hdr[14] | hdr[15] << 8;
    unsigned char depth = hdr[16];

    if ((type != 2 && type != 3) ||
        (type == 2 && depth != 24 && depth != 32) ||
        (type == 3 && depth != 8)) {

        fprintf(stderr, "Unsupported tga format");
        goto err1;
    }

    // open Fox output file
    FILE *out = fopen(argv[1], "wb");

    if (!out) {
        fprintf(stderr, "failed to open '%s' for writing\n", argv[1]);
        goto err1;
    }

    // encode header - this is specific only to this example
    // fox format has no header
    fwrite((unsigned char[]) {
        0x66, 0x6F, 0x78, 0x21,
        w & 0xFF, w >> 8 & 0xFF,
        h & 0xFF, h >> 8 & 0xFF,
    }, 8, 1, out);

    // initialize fox
    struct fox fox = { .callback = encode_out, .user = out };

    for (unsigned int y = 0; y < h; ++y) {
        for (unsigned int x = 0; x < w; ++x) {
            struct fox_argb color;

            // read TGA pixel
            if (type == 3) {
                color.g = color.b = color.r = fgetc(in);
                color.a = 0xFF;
            } else {
                color.b = fgetc(in);
                color.g = fgetc(in);
                color.r = fgetc(in);
                color.a = depth == 32 ? fgetc(in) : 0xFF;
            }

            // write current pixel into the stream
            fox_write(&fox, color);
        }
    }

    // close the stream and file
    fox_close(&fox);

    res = 0;
    fclose(out);
    err1: fclose(in);
    err0: return res;
}

// =============================================================================

static unsigned char decode_in(unsigned char arg, void *user) {
    (void) arg;
    return fgetc(user);
}

static int main_decode(int argc, char **argv) {
    int res = 1;

    // expect input and output fname
    if (argc < 2) {
        fprintf(stderr, "Not enough arguments.");
        goto err0;
    }

    // open the Fox input file
    FILE *in = fopen(argv[0], "rb");

    if (!in) {
        fprintf(stderr, "failed to open '%s' for reading\n", argv[0]);
        goto err0;
    }

    // decode header - this is specific only to this example
    // fox format has no header
    unsigned char hdr[8];
    fread(hdr, 8, 1, in);

    if (hdr[0] != 0x66 || hdr[1] != 0x6F || hdr[2] != 0x78 || hdr[3] != 0x21) {
        fprintf(stderr, "not a fox image\n");
        goto err1;
    }

    // open TGA ouput file
    FILE *out = fopen(argv[1], "wb");
    if (!out) {
        fprintf(stderr, "failed to open '%s' for writing\n", argv[1]);
        goto err1;
    }

    // write TGA header
    fwrite((unsigned char[]) {
        0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        hdr[4], hdr[5], hdr[6], hdr[7], 32, 40,
    }, 18, 1, out);

    // initialize fox
    struct fox fox = { .callback = decode_in, .user = in };

    // open the stream
    fox_open(&fox);

    unsigned int w = hdr[4] | hdr[5] << 8;
    unsigned int h = hdr[6] | hdr[7] << 8;

    for (unsigned int y = 0; y < h; ++y) {
        for (unsigned int x = 0; x < w; ++x) {
            // read a pixel from the stream
            struct fox_argb color = fox_read(&fox);
            fputc(color.b, out);
            fputc(color.g, out);
            fputc(color.r, out);
            fputc(color.a, out);
        }
    }

    res = 0;

    fclose(out);
    err1: fclose(in);
    err0: return res;
}

// =============================================================================

int main(int argc, char **argv) {
    if (argc < 2) goto usage;
    const char *op = argv[1];

    if (!strcmp(op, "encode")) {
        return main_encode(argc - 2, argv + 2);
    } else if (!strcmp(op, "decode")) {
        return main_decode(argc - 2, argv + 2);
    } else {
        usage: return printf("usage:\n"
            "  fox encode <input.tga> <output.fox>\n"
            "  fox decode <input.fox> <output.tga>\n"), 0;
    }
}
