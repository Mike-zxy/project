#include "robust.h"
#include <stdlib.h>

void flip_horizontal(BMPImage* img) {
    for (int i = 0; i < img->height; i++)
        for (int j = 0; j < img->width / 2; j++) {
            Pixel tmp = img->data[i][j];
            img->data[i][j] = img->data[i][img->width - 1 - j];
            img->data[i][img->width - 1 - j] = tmp;
        }
}

void flip_vertical(BMPImage* img) {
    for (int i = 0; i < img->height / 2; i++) {
        Pixel* tmp = img->data[i];
        img->data[i] = img->data[img->height - 1 - i];
        img->data[img->height - 1 - i] = tmp;
    }
}

void shift_image(BMPImage* img, int dx, int dy) {
    Pixel** new_data = malloc(sizeof(Pixel*) * img->height);
    for (int i = 0; i < img->height; i++) {
        new_data[i] = malloc(sizeof(Pixel) * img->width);
        for (int j = 0; j < img->width; j++) {
            int ni = i - dy, nj = j - dx;
            if (ni >= 0 && ni < img->height && nj >= 0 && nj < img->width)
                new_data[i][j] = img->data[ni][nj];
            else
                new_data[i][j] = (Pixel){ 0, 0, 0 };
        }
    }
    for (int i = 0; i < img->height; i++) free(img->data[i]);
    free(img->data);
    img->data = new_data;
}

void crop_image(BMPImage* img, int new_w, int new_h) {
    if (new_w > img->width) new_w = img->width;
    if (new_h > img->height) new_h = img->height;
    Pixel** new_data = malloc(sizeof(Pixel*) * new_h);
    for (int i = 0; i < new_h; i++) {
        new_data[i] = malloc(sizeof(Pixel) * new_w);
        for (int j = 0; j < new_w; j++) {
            new_data[i][j] = img->data[i][j];
        }
    }
    for (int i = 0; i < img->height; i++) free(img->data[i]);
    free(img->data);
    img->data = new_data;
    img->width = new_w;
    img->height = new_h;
}

void adjust_contrast(BMPImage* img, float factor) {
    for (int i = 0; i < img->height; i++)
        for (int j = 0; j < img->width; j++) {
            Pixel* p = &img->data[i][j];
            int adjust = (int)((p->r - 128) * factor + 128);
            p->r = adjust < 0 ? 0 : adjust > 255 ? 255 : adjust;

            adjust = (int)((p->g - 128) * factor + 128);
            p->g = adjust < 0 ? 0 : adjust > 255 ? 255 : adjust;

            adjust = (int)((p->b - 128) * factor + 128);
            p->b = adjust < 0 ? 0 : adjust > 255 ? 255 : adjust;
        }
}
