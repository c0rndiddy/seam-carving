#if !defined(CIMG)
#define CIMG

#include <stdlib.h>
#include <stdint.h>


struct rgb_img{
    uint8_t *raster;
    size_t height;
    size_t width;
};

void create_img(struct rgb_img **im, size_t height, size_t width);
void read_in_img(struct rgb_img **im, char *filename);
void write_img(struct rgb_img *im, char *filename);
uint8_t get_pixel(struct rgb_img *im, int y, int x, int col);
void set_pixel(struct rgb_img *im, int y, int x, int r, int g, int b);
void destroy_image(struct rgb_img *im);
void print_grad(struct rgb_img *grad);


#endif
