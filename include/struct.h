#ifndef STRUCT_H
#define STRUCT_H

#include <stdint.h>

#include "coordinates.h"

#define HAND_TYPE uint16_t

typedef struct hand_hold {
    HAND_TYPE item_id;
    HAND_TYPE item_type;
} hand_hold;

#define HEAD_TYPE int16_t

typedef struct head_rot {
    HEAD_TYPE xrot;
    HEAD_TYPE yrot;
} head_rot;

typedef struct lattice_user {

    uint32_t userid;
    uint32_t flags;
    uint16_t model;
    uint32_t color;
    char *nickname;

    n_coord centeredon;
    w_coord wpos;
    b_coord bpos;
    head_rot hrot;
    hand_hold hhold;

    uint32_t usercolor;

    uint16_t burstdist;

} lattice_user;

struct message {
  char *cmd;
  int (*func)();
  uint32_t flags;
};

#endif
