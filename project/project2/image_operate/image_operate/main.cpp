#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image_utils.h"
#include "robust.h"

void embed_watermark(BMPImage* img, const char* watermark) {
    int len = strlen(watermark);
    int k = 0;
    for (int i = 0; i < img->height && k < len * 8; i++) {
        for (int j = 0; j < img->width && k < len * 8; j++) {
            unsigned char ch = watermark[k / 8];
            unsigned char bit = (ch >> (7 - (k % 8))) & 1;
            img->data[i][j].b = (img->data[i][j].b & 0xFE) | bit;
            k++;
        }
    }
}

void extract_watermark(BMPImage* img, int len) {
    char* result = (char*)malloc(len + 1);
    memset(result, 0, len + 1);
    int k = 0;
    for (int i = 0; i < img->height && k < len * 8; i++) {
        for (int j = 0; j < img->width && k < len * 8; j++) {
            result[k / 8] <<= 1;
            result[k / 8] |= (img->data[i][j].b & 1);
            k++;
        }
    }
    for (int i = 0; i < len; i++) result[i] = result[i];
    printf("Extracted: %s\n", result);
    free(result);
}

int main() {
    BMPImage* img = read_bmp("original.bmp");
    if (!img) { printf("Read error\n"); return 1; }

    embed_watermark(img, "Secret!");
    write_bmp("watermarked.bmp", img);

    // 鲁棒性测试示例
    flip_horizontal(img);
    write_bmp("flip_h.bmp", img);

    flip_vertical(img);
    write_bmp("flip_v.bmp", img);

    shift_image(img, 5, 5);
    write_bmp("shifted.bmp", img);

    crop_image(img, img->width - 10, img->height - 10);
    write_bmp("cropped.bmp", img);

    adjust_contrast(img, 1.5f);
    write_bmp("contrast.bmp", img);

    extract_watermark(img, 7);

    free_bmp(img);
    return 0;
}
