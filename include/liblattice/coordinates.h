#ifndef COORDINATES_H
#define COORDINATES_H

// int coords
typedef int32_t ix;
typedef int32_t iy;
typedef int32_t iz;

// network coordinates (nx ny nz)
typedef struct n_coord {
    ix x;
    iy y;
    iz z;
} n_coord;

// world coordinates (wx wy wz)
typedef struct w_coord {
    ix x;
    iy y;
    iz z;
} w_coord;

// flatland coordinates (wx wz)
typedef struct f_coord {
    ix x;
    iz z;
} f_coord;

// block coordinates (bx by bz)
typedef struct b_coord {
    ix x;
    iy y;
    iz z;
} b_coord;

#endif
