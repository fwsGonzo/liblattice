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

#define T_P      1
#define T_QUIT   2

#define MFLAG_FROM      0x00000001         // Is fromuid set

#define TstFlagFrom(m) ((m)->flags & MFLAG_FROM)
#define SetFlagFrom(m) ((m)->flags |= MFLAG_FROM)
#define ClrFlagFrom(m) ((m)->flags &= (~MFLAG_FROM))


typedef struct lattice_message {

    int type;
    int flags;
    uint32_t fromuid;
    void * args;

} lattice_message;


typedef struct lattice_p {

    w_coord wcoord;
    b_coord bcoord;

} lattice_p;

typedef struct lattice_quit {

    uint32_t numeric;
    char desc[MTU];

} lattice_quit;


#endif
