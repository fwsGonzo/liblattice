#ifndef STRUCT_H
#define STRUCT_H

#include <stdint.h>

#include "coordinates.h"

typedef struct block_t {
    unsigned short id : 10;
    unsigned short bf : 6;
} block_t;

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

typedef struct lattice_player_t {

    uint32_t userid;
    uint32_t flags;
    uint16_t model;
    uint32_t color;
    char *nickname;

    n_coord centeredon;

    w_coord my_min_wcoord;
    w_coord my_max_wcoord;

    w_coord wpos;
    b_coord bpos;
    head_rot hrot;
    hand_hold hhold;

    uint32_t usercolor;

    uint16_t burstdist;

    int mining;

} lattice_player_t;

struct message {
  char *cmd;
  int (*func)();
  uint32_t flags;
};

#define T_UNKNOWN      0
#define T_P            1
#define T_QUIT         2
#define T_PC           3
#define T_PR           4
#define T_PH           5
#define T_CHAT         6
#define T_ACTION       7
#define T_S            8
#define T_SC           9
#define T_BO           10
#define T_MO           11
#define T_BADD         12
#define T_BSET         13
#define T_BREM         14
#define T_PMINE        15
#define T_SCHAT        16
#define T_LUSERS       17
#define T_LOG          18
#define T_SATSTEP      19
#define T_SAT          20
#define T_FADE         21
#define T_USER         22
#define T_SERVER       23
#define T_BUMP         24
#define T_CONNECTED    25
#define T_DISCONNECTED 26
#define T_MOVETO       27
#define T_MOVEFROM     28
#define T_CLOSING      29

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

typedef struct lattice_closing {

    uint32_t numeric;
    char desc[MTU];

} lattice_closing;

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

typedef struct lattice_badd {

    w_coord wcoord;
    b_coord bcoord;
    block_t block;

} lattice_badd;

typedef struct lattice_bset {

    w_coord wcoord;
    b_coord bcoord;
    block_t block;

} lattice_bset;

typedef struct lattice_brem {

    w_coord wcoord;
    b_coord bcoord;

} lattice_brem;

typedef struct lattice_pmine {

    int mining;  // 0 or 1

} lattice_pmine;

typedef struct lattice_schat {

    char nickname[MTU/2];
    uint32_t color;
    char string[MTU];

} lattice_schat;

typedef struct lattice_log {

    char string[MTU];

} lattice_log;

typedef struct lattice_satstep {

    uint32_t satstep;

} lattice_satstep;

typedef struct lattice_sat {

    double sat;

} lattice_sat;


typedef struct lattice_user {

    uint16_t model;
    uint32_t color;
    char nickname[MTU/2];

    w_coord wpos;
    b_coord bpos;

    head_rot hrot;
    hand_hold hhold;

    int mining;

    uint32_t usercolor;

} lattice_user;

typedef struct lattice_server {

    n_coord ncoord;
    struct in_addr ip;
    uint16_t port;

} lattice_server;

typedef struct lattice_bump {

    w_coord wcoord;
    b_coord bcoord;

} lattice_bump;

#endif
