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

#define T_P       1
#define T_QUIT    2
#define T_PC      3
#define T_PR      4
#define T_PH      5
#define T_CHAT    6
#define T_ACTION  7
#define T_S       8
#define T_SC      9
#define T_BO      10
#define T_MO      11

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

typedef struct lattice_pc {

    uint32_t color;

} lattice_pc;

typedef struct lattice_pr {

    head_rot rot;

} lattice_pr;

typedef struct lattice_ph {

    hand_hold hand;

} lattice_ph;

typedef struct lattice_chat {

    char string[MTU];

} lattice_chat;

typedef struct lattice_action {

    char string[MTU];

} lattice_action;

typedef struct lattice_s {

    int32_t mid;
    int32_t sid;

} lattice_s;

typedef struct lattice_sc {

    int32_t csid;

} lattice_sc;

typedef struct lattice_bo {

    w_coord wcoord;
    b_coord bcoord;
    int32_t id;

} lattice_bo;

typedef struct lattice_mo {

    w_coord wcoord;
    b_coord bcoord;
    int32_t id;
    int32_t count;

} lattice_mo;


#endif
