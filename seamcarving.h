#if !defined(SEAMCARVING_H)
#define SEAMCARVING_H
#include "c_img.h"

void calc_energy(struct rgb_img *im, struct rgb_img **grad);
void dynamic_seam(struct rgb_img *grad, double **best_arr);
void recover_path(double *best, int height, int width, int **path);
void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path);

#endif