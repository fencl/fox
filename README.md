# Fox
Lossless image format

Fox is a lossless image format aiming to provide a satisfying compression ratio with a tiny encoder/decoder.
Fox bitstream encodes a stream of 8bit rgba pixels, no additional information is included.
This makes fox a good choice for embeding into custom formats.

## Tiny
Both reference encoder and decoder are currenty written in under 80 LOC.
There is no additional complexity in encoder compared to decoder.

## Compression ratio
Fox usually provides the same or slightly better compression ratio than PNG.
There are outliers as with any format but generally Fox is better for very small images, photos, some textures and images with large areas of the same color.
PNG works better for gradients, repeating patterns and images with logically detached RGB channels (normal maps, etc).

Actuall numbers comming soon...

## Space complexity
Fox compresses images in a streaming fashion with O(1) space complexity (acually < 2k bytes).
This means that there is effectively no limit on the image size.

## Time complexity
There is no guesswork involved, each pixel is encoded in O(1) time. Therefore the whole image is encoded in O(n) time.
The actual speed is not as impressive as it seems though, mainly because each bit of a symbol (color, hash index or run length) is encoded in a separate step.

## Documentation
WIP

## Patents and license
IANAL! Format uses similar pixel encoding to QOI format, with range coding and adaptive probability model on top.
I am not aware of any patents regarding the techniques, as very similar techniques are widely used in many open formats (QOI, VP8L, CABAC, ...).

The reference implementation is Boost Software Licensed (as it is very much the least intrusive license i know of except for CC0 and WTFPL), however feel free to make your own.

# Reference implementation
Reference implementation consists of a sorce code for encoder (enc.c), decoder (dec.c) and single header file (fox.h)

## Building
As with many projects I have published, no build system is provided as it is not necessary.
Simply add `include` folder to your header paths and `src/enc.c` and/or `src/dec.c` to your sources. Boom. Done.

## Usage
Implementation has simple interface, however we have some setup to do.
Both encoder and decoder share the same structure.
First create the object.
```c
fox_coder_t fox;
```
Then prepare your stream. Wrap `fox_stream_t` in your stream structure. For example:
```c
typedef struct my_stream {
    fox_stream_t stream;
    FILE        *file;    // This is an example, here you should provide you own interface
} my_stream_t;
```
Prepare stream function. Here are examples for our my_stream implementation. For encoder prepare writable stream:
```c
void my_stream_write(fox_stream_t *stream, fox_u8_t data) {
    my_stream_t *my_stream = (my_stream_t*) stream;

    // stream is pointer to "stream" field in my_stream_t structure
    // in this example this field is at the top of said structure so
    // simple pointer cast is sufficient
    // alternatively use offsetof(my_stream_t, stream)
    // to find how many bytes to move the stream pointer back

    // write data to the stream
    fwrite(&data, 1, 1, my_stream->file);
}
```
or for decoder prepare readable stream:
```c
fox_u8_t my_stream_read(fox_stream_t *stream) {
    my_stream_t *my_stream = (my_stream_t*) stream;
    // see comment in my_stream_write

    // return next byte in the stream
    fox_u8_t data;
    fread(&data, 1, 1, my_stream->file);
    return data;
}
```
and finally initialize the stream

```c
my_stream_t stream = {
    .stream.write = my_stream_write,
    // or .stream.read = my_stream_read,

    .file = fopen("file.bin", "wb") // or rb, dont forget fclose :)
};
```

### Encoding
To encode an image, first open the stream.
```c
fox_enc_open(&fox, &stream);
```

Then for each pixel
```c
fox_enc_write(&fox, (fox_color_t) {
    .r = /* red   component */,
    .g = /* green component */,
    .b = /* blue  component */,
    .a = /* alpha component */,
});

/*
    or alternatively encode color in single u32 number
    bits [0..7]: red, [8..15]: green, [16..23]: blue, [24..31]: alpha

    fox_enc_write(&fox, (fox_color_t) {
        .argb = // here
    });
*/
```

Finally close the stream
```c
fox_enc_close(&fox);
```

### Decoding
To decode an image, first open the stream.
```c
fox_dec_open(&fox, &stream);
```
For each pixel
```c
fox_color_t color = fox_dec_read(&fox);
// now store the color however you like.
// you can access the .r, .g, .b, .a components or .argb value as explained in encoder
```
Unlike in encoder, stream does not have to be closed, to reuse the coder object, just open another stream.

## Example
I will provide some working example(s) later.
