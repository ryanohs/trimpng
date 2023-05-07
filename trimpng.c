#include<spng.h>
#include<stdio.h>
#include<stdbool.h>
#include<string.h>

// https://github.com/randy408/libspng/blob/v0.7.3/examples/example.c
int encode_image(FILE *outputFile, void *image, size_t length, uint32_t width, uint32_t height, enum spng_color_type color_type, int bit_depth)
{
    int fmt;
    int ret = 0;
    spng_ctx *ctx = NULL;
    struct spng_ihdr ihdr = {0}; /* zero-initialize to set valid defaults */

    /* Creating an encoder context requires a flag */
    ctx = spng_ctx_new(SPNG_CTX_ENCODER);

    /* Set an output FILE* or stream with spng_set_png_file() or spng_set_png_stream() */
    spng_set_png_file(ctx, outputFile);

    /* Set image properties, this determines the destination image format */
    ihdr.width = width;
    ihdr.height = height;
    ihdr.color_type = color_type;
    ihdr.bit_depth = bit_depth;
    /* Valid color type, bit depth combinations: https://www.w3.org/TR/2003/REC-PNG-20031110/#table111 */

    spng_set_ihdr(ctx, &ihdr);

    /* When encoding fmt is the source format */
    /* SPNG_FMT_PNG is a special value that matches the format in ihdr */
    fmt = SPNG_FMT_PNG;

    /* SPNG_ENCODE_FINALIZE will finalize the PNG with the end-of-file marker */
    ret = spng_encode_image(ctx, image, length, fmt, SPNG_ENCODE_FINALIZE);

    if(ret)
    {
        printf("spng_encode_image() error: %s\n", spng_strerror(ret));
    }

    spng_ctx_free(ctx);

    return ret;
}

int main(int argc, char **argv)
{
    if(argc < 3)
    {
        printf("Usage: trimpng inputFileName outputFileName\n");
    }

    char *filename = argv[1];
    char *outputFilename = argv[2];

    FILE *png = fopen(filename, "rb");
    if(png == NULL) printf("Could not open file: %s\n", filename);

    spng_ctx *ctx = spng_ctx_new(0);
    spng_set_png_file(ctx, png);

    size_t imageSize;
    spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &imageSize);

    unsigned char *rgba = malloc(imageSize);

    /* Get an 8-bit RGBA image regardless of PNG format */
    spng_decode_image(ctx, rgba, imageSize, SPNG_FMT_RGBA8, 0);

    struct spng_ihdr ihdr;
    int ret = spng_get_ihdr(ctx, &ihdr);

    if(ret)
    {
        printf("spng_get_ihdr() error: %s\n", spng_strerror(ret));
        goto error;
    }

    printf("Width: %u\nHeight: %u\n", ihdr.width, ihdr.height);


    // begin trimming logic

    unsigned int xl, xr, yt, yb; // bounding box
    unsigned char r, g, b;       // border color reference
    unsigned int x, y;
    bool uniqueFound;


    // top boundary

    x = 0;
    y = 0;
    r = rgba[0];
    g = rgba[1];
    b = rgba[2];
    uniqueFound = false;

    do
    {
        x = 0;
        do
        {
            uniqueFound = 
                   (r != rgba[4 * ihdr.width * y + 4 * x])
                || (g != rgba[4 * ihdr.width * y + 4 * x + 1])
                || (b != rgba[4 * ihdr.width * y + 4 * x + 2]);
            x++;
        } while (!uniqueFound && x < ihdr.width);
        y++;
    } while(!uniqueFound && y < ihdr.height);

    yt = y-1;


    // bottom boundary

    x = 0;
    y = ihdr.height - 1;
    uniqueFound = false;

    do
    {
        x = 0;
        do
        {
            uniqueFound = 
                   (r != rgba[4 * ihdr.width * y + 4 * x])
                || (g != rgba[4 * ihdr.width * y + 4 * x + 1])
                || (b != rgba[4 * ihdr.width * y + 4 * x + 2]);
            x++;
        } while (!uniqueFound && x < ihdr.width);
        y--;
    } while(!uniqueFound && y >= 0);

    yb = y+2;


    // left boundary

    x = 0;
    y = yt;
    uniqueFound = false;

    do
    {
        y = yt;
        do
        {
            uniqueFound = 
                   (r != rgba[4 * ihdr.width * y + 4 * x])
                || (g != rgba[4 * ihdr.width * y + 4 * x + 1])
                || (b != rgba[4 * ihdr.width * y + 4 * x + 2]);
            y++;
        } while(!uniqueFound && y <= yb);
        x++;
    } while (!uniqueFound && x < ihdr.width);

    xl = x-1;


    // right boundary

    x = ihdr.width - 1;
    y = yt;
    uniqueFound = false;

    do
    {
        y = yt;
        do
        {
            uniqueFound = 
                   (r != rgba[4 * ihdr.width * y + 4 * x])
                || (g != rgba[4 * ihdr.width * y + 4 * x + 1])
                || (b != rgba[4 * ihdr.width * y + 4 * x + 2]);
            y++;
        } while(!uniqueFound && y <= yb);
        x--;
    } while (!uniqueFound && x > xl);

    xr = x+2;


    printf("Top boundary: %u \n", yt);
    printf("Bottom boundary: %u \n", yb);
    printf("Left boundary: %u\n", xl);
    printf("Right boundary: %u\n", xr);


    // Create trimmed image

    int w = (int)(xr-xl);
    int h = (int)(yb-yt);
    size_t outputSize = 4 * w * h * sizeof(char);
    char *trimmedBuffer = malloc(outputSize);

    for(int i = 0; i < h; i++)
    {
        memcpy(&trimmedBuffer[4 * i * w], &rgba[4 * ihdr.width * (i + yt) + 4 * xl], 4 * w);
    }

    FILE *outputPng = fopen(outputFilename, "wb");
    if(outputPng == NULL) printf("Could not create output file: %s\n", outputFilename);

    encode_image(outputPng, trimmedBuffer, outputSize, w, h, ihdr.color_type, ihdr.bit_depth);

    fclose(outputPng);

    free(trimmedBuffer);
error:
    free(rgba);
    spng_ctx_free(ctx);
    return 0;
}