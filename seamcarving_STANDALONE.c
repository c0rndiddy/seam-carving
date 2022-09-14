// #include <seamcarving.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//*****************
#include <stdlib.h>
#include <stdint.h>
struct rgb_img{
    uint8_t *raster;
    size_t height;
    size_t width;
};

void create_img(struct rgb_img **im, size_t height, size_t width){
    *im = (struct rgb_img *)malloc(sizeof(struct rgb_img));
    (*im)->height = height;
    (*im)->width = width;
    (*im)->raster = (uint8_t *)malloc(3 * height * width);
}


// Say the int takes 2 bytes
// 00110001 10101010
// ^bytes[0]  ^bytes[1] ; note big-endian order

int read_2bytes(FILE *fp){
    uint8_t bytes[2];
    fread(bytes, sizeof(uint8_t), 1, fp);
    fread(bytes+1, sizeof(uint8_t), 1, fp);
    return (  ((int)bytes[0]) << 8)  + (int)bytes[1];
    //           ^ convert to int, then bit shift left by 8, since bytes[1] is 8 bits
}

void write_2bytes(FILE *fp, int num){
    uint8_t bytes[2];
    bytes[0] = (uint8_t)((num & 0XFFFF) >> 8);
    bytes[1] = (uint8_t)(num & 0XFF);
    // 0X means hexadecimal
    // F comes from hexadecimal, which goes 0, 1, ..., 9, A, B, ...,  F
    // F means 15, which is 1111 in binary, so binary & w 0XFF means compare
    // 00110001 10101010 with
    // 00000000 11111111 
    // Result: 00000000 10101010, which preserves the num
    // FF means 255
    fwrite(bytes, 1, 1, fp);
    fwrite(bytes+1, 1, 1, fp);
}

void read_in_img(struct rgb_img **im, char *filename){
    FILE *fp = fopen(filename, "rb");
    size_t height = read_2bytes(fp);
    size_t width = read_2bytes(fp);
    create_img(im, height, width);
    fread((*im)->raster, 1, 3*width*height, fp);
    fclose(fp);
}

void write_img(struct rgb_img *im, char *filename){
    FILE *fp = fopen(filename, "wb");
    write_2bytes(fp, im->height);
    write_2bytes(fp, im->width);
    fwrite(im->raster, 1, im->height * im->width * 3, fp);
    fclose(fp);
}

uint8_t get_pixel(struct rgb_img *im, int y, int x, int col){
    return im->raster[3 * (y*(im->width) + x) + col];
}

void set_pixel(struct rgb_img *im, int y, int x, int r, int g, int b){
    im->raster[3 * (y*(im->width) + x) + 0] = r;
    im->raster[3 * (y*(im->width) + x) + 1] = g;
    im->raster[3 * (y*(im->width) + x) + 2] = b;
}

void destroy_image(struct rgb_img *im)
{
    free(im->raster);
    free(im);
}


void print_grad(struct rgb_img *grad){
    int height = grad->height;
    int width = grad->width;
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            printf("%d\t", get_pixel(grad, i, j, 0));
        }
    printf("\n");    
    }
}
//*****************

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

// Print array best_arr produced by dynamic_seam()
void print_best_array(double **best_arr, int height, int width) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            printf("%f\t", (*best_arr)[i*width+j]);
        }
        printf("\n");
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

// Print minimum energy path produced by recover_path()
void print_path(int **path, int height) {
    printf("Path: ");
    for (int i = 0; i < height; i++) {
        printf("%d ", (*path)[i]);
    }
    printf("\n");
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

int main(void) {
    // Testing the functions
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
    print_best_array(&best_arr, height, weight);
    printf("========\n");

    int *path;
    recover_path(best_arr, height, weight, &path);
    print_path(&path, height);
    printf("========\n");

    struct rgb_img *dest;
    remove_seam(im, &dest, path);
    printf("Image with seam removed (red pixels):\n");
    print_grad(dest);
    printf("========\n");
    
    
    /* // Removing multiple seams from an image
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
    */
}