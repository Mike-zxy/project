#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include "image_utils.h"

#pragma pack(push, 1)
typedef struct {
    unsigned short type;
    unsigned int size;
    unsigned short reserved1, reserved2;
    unsigned int offset;
} BMPHeader;

typedef struct {
    unsigned int size;
    int width, height;
    unsigned short planes;
    unsigned short bits;
    unsigned int compression;
    unsigned int imageSize;
    int xResolution, yResolution;
    unsigned int nColors;
    unsigned int importantColors;
} DIBHeader;
#pragma pack(pop)

BMPImage* read_bmp(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;

    BMPHeader header;
    fread(&header, sizeof(BMPHeader), 1, f);
    if (header.type != 0x4D42) { fclose(f); return NULL; }

    DIBHeader dib;
    fread(&dib, sizeof(DIBHeader), 1, f);

    if (dib.bits != 24) { fclose(f); return NULL; }

    BMPImage* img = (BMPImage*)malloc(sizeof(BMPImage));
    img->width = dib.width;
    img->height = dib.height;

    img->data = (Pixel**)malloc(sizeof(Pixel*) * img->height);
    int padding = (4 - (img->width * 3) % 4) % 4;

    for (int i = 0; i < img->height; i++) {
        img->data[i] = (Pixel*)malloc(sizeof(Pixel) * img->width);
        fread(img->data[i], sizeof(Pixel), img->width, f);
        fseek(f, padding, SEEK_CUR);
    }

    fclose(f);
    return img;
}

void write_bmp(const char* filename, BMPImage* img) {
    FILE* f = fopen(filename, "wb");
    if (!f) return;

    int padding = (4 - (img->width * 3) % 4) % 4;
    int rowSize = img->width * 3 + padding;
    int dataSize = rowSize * img->height;

    BMPHeader header = { 0x4D42, (unsigned int)(54 + dataSize), 0, 0, 54 };
    DIBHeader dib = {
        40,
        (unsigned int)img->width,
        (unsigned int)img->height,
        1, 24,
        0,
        (unsigned int)dataSize,
        2835, 2835, 0, 0
    };



    fwrite(&header, sizeof(header), 1, f);
    fwrite(&dib, sizeof(dib), 1, f);
};