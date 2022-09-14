#include "seamcarving.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Compute dual-gradient energy for every pixel in im, and store it as an image in grad
void calc_energy(struct rgb_img *im, struct rgb_img **grad) {
    int height = im->height;
    int width = im->width;
    create_img(grad, height, width);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            
            // Pixels around central pixel
            int j_front = j+1;
            int j_back = j-1;
            int i_front = i+1;
            int i_back = i-1;

            // For pixels at edge of image, "wrap around" the image
            if (j_front >= width) j_front = 0;
            else if (j_back < 0) j_back = width-1;
            if (i_front >= height) i_front = 0;
            else if (i_back < 0) i_back = height-1;

            // Differences in red, green, and blue components of pixels around central pixel
            int R_x = get_pixel(im, i, j_front, 0) - get_pixel(im, i, j_back, 0);
            int G_x = get_pixel(im, i, j_front, 1) - get_pixel(im, i, j_back, 1);
            int B_x = get_pixel(im, i, j_front, 2) - get_pixel(im, i, j_back, 2);
            int R_y = get_pixel(im, i_front, j, 0) - get_pixel(im, i_back, j, 0);
            int G_y = get_pixel(im, i_front, j, 1) - get_pixel(im, i_back, j, 1);
            int B_y = get_pixel(im, i_front, j, 2) - get_pixel(im, i_back, j, 2);

            // Calculate energy
            int delta_x_squared = pow(R_x, 2) + pow(G_x, 2) + pow(B_x, 2);
            int delta_y_squared = pow(R_y, 2) + pow(G_y, 2) + pow(B_y, 2);
            uint8_t energy = sqrt(delta_x_squared + delta_y_squared)/10;

            // Store dual-gradient energies as an image
            set_pixel(*grad, i, j, energy, energy, energy);
        }  
    }
}

// Find minimum between 2 elems
int min2(int a, int b) {
    if (a < b) 
        return a;
    else 
        return b;
}

// Find minimum between 3 elems
int min3(int a, int b, int c) {
    int cur_min = 100000;
    
    if (a < cur_min)
        cur_min = a;
    if (b < cur_min)
        cur_min = b;
    if (c < cur_min)
        cur_min = c;
    
    return cur_min;
}

// Find minimum cost of a seam from the top of grad to every point (i,j) in grad and store the min costs in array best_arr
void dynamic_seam(struct rgb_img *grad, double **best_arr) {
    int height = grad->height;
    int width = grad->width;
    *best_arr = (double *)malloc(sizeof(double)*width*height);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (i == 0) 
                (*best_arr)[i*width+j] = (double)( get_pixel(grad, i, j, 0) );
            else if (j == 0) 
                (*best_arr)[i*width+j] = (double)( get_pixel(grad, i, j, 0) + min2((*best_arr)[(i-1)*width+j], (*best_arr)[(i-1)*width+(j+1)]) );
            else if (j == width-1)
                (*best_arr)[i*width+j] = (double)( get_pixel(grad, i, j, 0) + min2((*best_arr)[(i-1)*width+j], (*best_arr)[(i-1)*width+(j-1)]) );
            else
                (*best_arr)[i*width+j] = (double)( get_pixel(grad, i, j, 0) + min3((*best_arr)[(i-1)*width+(j-1)], (*best_arr)[(i-1)*width+j], (*best_arr)[(i-1)*width+(j+1)]) );
        }
    }
}

// Find minimum energy path through array best and place the corresponding indices for the min energy point in each level in path
void recover_path(double *best, int height, int width, int **path) {
    *path = (int *)malloc(sizeof(int)*height);
    
    for (int i = height-1; i >= 0; i--) {
        double cur_min = 100000;
        int cur_min_index = -1;
 
        // Find the minimum energy node at the bottom of the best array
        if (i == height-1) {
            for (int j = 0; j < width; j++) {
                if (best[i*width+j] < cur_min) {
                    cur_min = best[i*width+j];
                    cur_min_index = j;
                }
            }
        }

        // Trace upward through the nodes the min energy node is connected to a level above to find the min energy path
        else {
            int j = (*path)[i+1];
            // If last node is at left edge, find min from j = 0, 1 on level above
            if (j == 0) {
                if (best[i*width+j] < best[i*width+(j+1)]) {
                    cur_min = best[i*width+j];
                    cur_min_index = j;
                }
                else {
                    cur_min = best[i*width+(j+1)];
                    cur_min_index = j+1;
                }
            }

            // If last node is at right edge, find min from j = width-1, width-2 on level above
            else if (j == width-1) {
                if (best[i*width+j] < best[i*width+(j-1)]) {
                    cur_min = best[i*width+j];
                    cur_min_index = j;
                }
                else {
                    cur_min = best[i*width+(j-1)];
                    cur_min_index = j-1;
                }
            }

            // Else find min from all 3 nodes on level above
            else {
                cur_min = min3(best[i*width+(j-1)], best[i*width+j], best[i*width+(j+1)]);
                for (int k = j-1; k <= j+1; k++) {
                    if (best[i*width+k] == cur_min)
                        cur_min_index = k;
                }
            }
        }
        (*path)[i] = cur_min_index;
    }
}

// Create destination image dest, and write source image src to it, with the seam removed
void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path) {
    int height = src->height;
    int width = src->width;
    create_img(dest, height, width-1);

    int offset = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (j == path[i]) { 
                offset = 1;
                continue;
            }
            int R = get_pixel(src, i, j, 0);
            int G = get_pixel(src, i, j, 1);
            int B = get_pixel(src, i, j, 2);
            set_pixel(*dest, i, j-offset, R, G, B);
        }
        offset = 0;
    }
}