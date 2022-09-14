#include "seamcarving.h"
#include <stdio.h>

/*
// Print array best_arr produced by dynamic_seam()
void print_best_array(double **best_arr, int height, int width) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            printf("%f\t", (*best_arr)[i*width+j]);
        }
        printf("\n");
    }
}

// Print minimum energy path produced by recover_path()
void print_path(int **path, int height) {
    printf("Path: ");
    for (int i = 0; i < height; i++) {
        printf("%d ", (*path)[i]);
    }
    printf("\n");
}
*/

int main(void) {
    /* // Testing the functions
    struct rgb_img *grad;
    struct rgb_img *im;
    read_in_img(&im, "C:\\Users\\huang\\OneDrive\\Documents\\USB\\UofT\\2020-2021S Work\\ESC190\\Project2\\6x5.bin");
    printf("Image (red pixels):\n");
    print_grad(im);
    printf("========\n");

    calc_energy(im,  &grad);
    printf("Dual-grad image:\n");
    print_grad(grad);
    printf("========\n");
    
    int height = 5;
    int weight = 6;

    double *best_arr;
    dynamic_seam(grad, &best_arr);  
    printf("Best array:\n");
    //print_best_array(&best_arr, height, weight);
    printf("========\n");

    int *path;
    recover_path(best_arr, height, weight, &path);
    //print_path(&path, height);
    printf("========\n");

    struct rgb_img *dest;
    remove_seam(im, &dest, path);
    printf("Image with seam removed (red pixels):\n");
    print_grad(dest);
    printf("========\n");
    */
    
    
    // Removing multiple seams from an image
    struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

    read_in_img(&im, "HJoceanSmall.bin");
    
    for(int i = 0; i < 150; i++){
        printf("i = %d\n", i);
        calc_energy(im,  &grad);
        dynamic_seam(grad, &best);
        recover_path(best, grad->height, grad->width, &path);
        remove_seam(im, &cur_im, path);

        char filename[200];
        sprintf(filename, "img%d.bin", i);
        write_img(cur_im, filename);


        destroy_image(im);
        destroy_image(grad);
        free(best);
        free(path);
        im = cur_im;
    }
    destroy_image(im);
}