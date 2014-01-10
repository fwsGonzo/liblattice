#ifndef COORDINATES_H
#define COORDINATES_H

// int coords
typedef long int ix;
typedef long int iy;
typedef long int iz;

// network coordinates
typedef struct n_coord {
    ix x;
    iy y;
    iz z;
} n_coord;

// world coordinates
typedef struct w_coord {
    ix x;
    iy y;
    iz z;
} w_coord;

// block coordinates
typedef struct b_coord {
    ix x;
    iy y;
    iz z;
} b_coord;

#endif
