#pragma once
#ifndef ROBUST_H
#define ROBUST_H

#include "image_utils.h"

void flip_horizontal(BMPImage* img);
void flip_vertical(BMPImage* img);
void shift_image(BMPImage* img, int dx, int dy);
void crop_image(BMPImage* img, int new_w, int new_h);
void adjust_contrast(BMPImage* img, float factor);

#endif
