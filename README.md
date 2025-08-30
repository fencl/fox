# Fox

Lossless image format and encoder/decoder in C

Fox is a lossless image format aiming to provide a satisfying compression ratio with a tiny encoder/decoder.  
Fox only encodes a stream of 32bit rgba pixels (8-bit per channel), includes no additional metadata like width/height.  
This makes fox a good choice for embedding into a custom container format.

## Tiny

Both encoder and decoder are currenty written in under 55 LOC.

## Dependencies

No dependencies, not event libc. Just plain C99.

## Compression ratio

Fox usually provides similar compression ratio to a PNG (libpng encoded).  
Fox might outperform PNG on photos, smaller images, some textures or screenshots.  
PNG works better for repeating patterns and can be usually brute force optimized to further reduce the image size.  
Both formats are outperformed by lossless versions of modern image formats such as WebP or JpegXL.  

See the [benchmark](BENCHMARK.md) for size comparison on few image sets.

## Time complexity

Each pixel is encoded/decoded in O(1) time. Therefore the whole image is encoded in O(n) time.  
The actual speed is not as impressive, mainly because each symbol (color, hash index or run length) is encoded bit by bit.

## Space complexity

Fox compresses images in a streaming fashion with O(1) space complexity (acually < 2k bytes).  
This means that there is no limit on the image size.

## Building

No build system is currently included as it is not necessary.  
Simply add `include` folder to your header paths and compile/link `src/enc.c` and/or `src/dec.c` with your sources. Boom. Done.

## Usage

### Encoder/Decoder

Both encoder and decoder share the same data structure.  
The structure has to be zero initialized except for user-provided callback and a user pointer.  
In this example we pass a standard stdio FILE through the user pointer.

```c
// fields except callback and user are initialized to zero
struct fox example = { .callback = my_callback, .user = file };
```

Define your callback.  
If used as an encoder, the callback will be called by the encoder to write a byte to the output stream, return value is ignored.  
If used as a decoder, the callback will be called by the decoder to read a byte from the input stream, arg argument is ignored.  
In any case the byte read/written is always expected to be 8-bits.

```c
static unsigned char my_callback(unsigned char arg, void *user) {
    // return fputc(arg, (FILE*) user); // for encoder mode
    // return fgetc((FILE*) user); // for decoder mode
}
```
Now that the structure is initialized we can start encoding/decoding.

### Encoding

Initialize the fox structure as shown above.

Do not use `fox_open` - it is only used by the decoder.

For each pixel call:

```c
// void fox_write(struct fox *encoder, struct fox_argb color);
fox_write(&example, color);
```

Close the stream by calling:

```c
// void fox_close(struct fox *encoder);
fox_close(&example);
```

### Decoding

Initialize the fox structure as shown above.

Open the stream by calling:

```c
// void fox_open(struct fox *decoder);
fox_open(&example);
```

For each pixel call:

```c
// struct fox_argb fox_read(struct fox *decoder);
struct fox_argb color = fox_read(&example);
```

Do not use `fox_close` - it is only used by the encoder.

## Example

Example is provided to show encoding and decoding.  
It converts TGA image to Fox and Fox to TGA.  
Only supported TGA formats are uncompressed 24 bit RGB, 32 bit RGBA and 8 bit grayscale.  
The fox image is saved with minimal header.

To build the example there are two scripts `build.sh` and `build.bat` to build the example using cc and cl respectively.

Usage:

```
fox encode image.tga image.fox
fox decode image.fox image.tga
```
