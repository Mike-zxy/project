#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

typedef struct {
    unsigned char r, g, b;
} Pixel;

typedef struct {
    int width;
    int height;
    Pixel** data;
} BMPImage;

BMPImage* read_bmp(const char* filename);
void write_bmp(const char* filename, BMPImage* img);
void free_bmp(BMPImage* img);

#endif
