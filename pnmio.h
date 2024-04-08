#ifndef PNMIO_H
#define PNMIO_H

#include <stdio.h>

/* PNM/PFM image data file format definitions. */
#define PBM_ASCII         1
#define PGM_ASCII         2
#define PPM_ASCII         3
#define PBM_BINARY        4
#define PGM_BINARY        5
#define PPM_BINARY        6
#define PAM               7 /* reserved */
                            /* 8-15: reserved */
#define PFM_RGB          16 /* F */
#define PFM_GREYSCALE    17 /* f */

#define IS_BIGENDIAN(x)   ((*(char*)&x) == 0)
#define IS_LITTLE_ENDIAN  (1 == *(unsigned char *)&(const int){1})
#ifndef FALSE
#define FALSE             0
#endif
#ifndef TRUE
#define TRUE              1
#endif


/* PNM/PFM API. */
int  get_pnm_type(FILE *f);
int read_pbm_header(FILE *f, int *img_xdim, int *img_ydim, int *is_ascii);
int read_pgm_header(FILE *f, int *img_xdim, int *img_ydim, int *img_colors,
       int *is_ascii);
int read_ppm_header(FILE *f, int *img_xdim, int *img_ydim, int *img_colors,
       int *is_ascii);
int read_pfm_header(FILE *f, int *img_xdim, int *img_ydim, int *img_type,
       int *endianess);
void read_pbm_data(FILE *f, int *img_in, int is_ascii);
void read_pgm_data(FILE *f, int *img_in, int is_ascii);
void read_ppm_data(FILE *f, int *img_in, int is_ascii);
void read_pfm_data(FILE *f, float *img_in, int img_type, int endianess);
void write_pbm_file(FILE *f, int *img_out,
       int x_size, int y_size, int x_scale_val, int y_scale_val, int linevals, 
       int is_ascii);
void write_pgm_file(FILE *f, int *img_out,
       int x_size, int y_size, int x_scale_val, int y_scale_val, 
       int img_colors, int linevals, int is_ascii);
void write_ppm_file(FILE *f, int *img_out,
       int x_size, int y_size, int x_scale_val, int y_scale_val, 
       int img_colors, int is_ascii);
void write_pfm_file(FILE *f, float *img_out,
       int x_size, int y_size, int img_type, int endianess);

/* Helper/auxiliary functions. */
int   ReadFloat(FILE *fptr, float *f, int swap);
int   WriteFloat(FILE *fptr, float *f, int swap);
int   floatEqualComparison(float A, float B, float maxRelDiff);
float frand(void);

#endif /* PNMIO_H */