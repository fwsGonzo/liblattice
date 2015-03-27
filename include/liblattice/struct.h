#ifndef STRUCT_H
#define STRUCT_H

#include <stdint.h>

#include "lattice_config.h"
#include "coordinates.h"

#define LATTICE_NICKLEN 32
#define LATTICE_STRLEN 512

typedef uint16_t block_t;

#define blockid(x) ( (x) & 1023 )
#define blockbf(x) ( ((x) >> 10) & 63 )
#define islight(id) (  (id) == BLOCKSDB_TORCH        || \
                       (id) == BLOCKSDB_MOLTENSTONE  || \
                       (id) == BLOCKSDB_LAVABLOCK    || \
                       (id) == BLOCKSDB_REDSTONE     || \
                       (id) == BLOCKSDB_GREENSTONE   || \
                       (id) == BLOCKSDB_FIRE         || \
                       (id) == BLOCKSDB_LANTERN  )

// number of colors in flatland data
#define FLATLAND_TERRAIN_COLORS 8
// the terrain color datatype

typedef uint32_t flatland_color_t;

typedef struct flatdata_t {
    flatland_color_t color[FLATLAND_TERRAIN_COLORS];
    uint8_t terrain;
    uint8_t skylevel;
    uint8_t groundlevel;
    uint8_t unused1;
} flatdata_t;

typedef struct sectorblock_t {
    //sectorblock_t() {}
    block_t  b[BLOCKSDB_COUNT_BX][BLOCKSDB_COUNT_BZ][BLOCKSDB_COUNT_BY];
    int16_t  blocks;
    int16_t  lights;
    uint8_t  hardsolid;
    uint8_t  special;
    uint16_t version;
} sectorblock_t;

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
    char nickname[LATTICE_NICKLEN];

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

/*
struct message {
  char *cmd;
  int (*func)();
  uint32_t flags;
};
*/

struct message {
  int type;
  int (*func)();
  uint32_t flags;
};


// Messages

#define T_UNKNOWN        0
#define T_P              1
#define T_QUIT           2
#define T_PC             3
#define T_PR             4
#define T_PH             5
#define T_CHAT           6
#define T_ACTION         7
#define T_S              8
#define T_SC             9
#define T_BO             10
#define T_MO             11
#define T_BADD           12
#define T_BSET           13
#define T_BREM           14
#define T_PMINE          15
#define T_SCHAT          16
#define T_LUSERS         17
#define T_LOG            18
#define T_SATSTEP        19
#define T_SAT            20
#define T_FADE           21
#define T_USER           22
#define T_SERVER         23
#define T_BUMP           24
#define T_CONNECTED      25
#define T_DISCONNECTED   26
#define T_MOVETO         27
#define T_MOVEFROM       28
#define T_CLOSING        29
#define T_CENTEREDINTRO  30
#define T_SIDEDINTRO     31
#define T_CENTEREDMOVE   32
#define T_SIDEDMOVE      33
#define T_ENDP           34
#define T_SLIDEOVER      35
#define T_PING           36
#define T_PONG           37
#define T_IAMSERVER      38
#define T_DELSERVER      39
#define T_TRACKERFAILURE 40
#define T_SERVEREOL      41
#define T_PCHAT          42
#define T_EMPTYSECTOR    43
#define T_SECTOR         44
#define T_FLATLAND       45

#define MFLAG_FROM      0x00000001         // Is fromuid set

#define TstFlagFrom(m) ((m)->flags & MFLAG_FROM)
#define SetFlagFrom(m) ((m)->flags |= MFLAG_FROM)
#define ClrFlagFrom(m) ((m)->flags &= (~MFLAG_FROM))


typedef struct lattice_message {

    int type;
    int flags;
    uint32_t fromuid;
    int length;
    void * args;

} lattice_message;


typedef struct lattice_p {

    w_coord wcoord;
    b_coord bcoord;

} lattice_p;

typedef struct lattice_quit {

    uint32_t numeric;
    char desc[LATTICE_STRLEN];

} lattice_quit;

typedef struct lattice_closing {

    uint32_t numeric;
    char desc[LATTICE_STRLEN];

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

    char chat_text[LATTICE_STRLEN];

} lattice_chat;

typedef struct lattice_pchat {

    uint32_t uid;
    char pchat_text[LATTICE_STRLEN];

} lattice_pchat;

typedef struct lattice_action {

    char action_text[LATTICE_STRLEN];

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

    char nickname[LATTICE_NICKLEN];
    uint32_t color;
    char schat_text[LATTICE_STRLEN];

} lattice_schat;

typedef struct lattice_log {

    char log_text[LATTICE_STRLEN];

} lattice_log;

typedef struct lattice_satstep {

    uint32_t satstep;
    double sat;

} lattice_satstep;

typedef struct lattice_sat {

    double sat;

} lattice_sat;


typedef struct lattice_user {

    uint16_t model;
    uint32_t color;
    char nickname[LATTICE_NICKLEN];

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

    w_coord bad_wcoord;
    b_coord bad_bcoord;

} lattice_bump;

typedef struct lattice_emptysector {

    w_coord wcoord;

} lattice_emptysector;

typedef struct lattice_sector {

    w_coord wcoord;
    sectorblock_t s;

} lattice_sector;

typedef struct lattice_flatland {

    f_coord fcoord;

    flatdata_t fdata[BLOCKSDB_FLATDATA_COUNT]; // bx bz

} lattice_flatland;


// ----------------------------------

#define SCHED_SERVER_RETRY 1
#define SCHED_SERVER_HANDSHAKE_TIMEOUT 2
#define SCHED_SERVER_PING_TIMEOUT 3
#define SCHED_SERVER_SEND_PING 4

typedef struct sched_usec_link {
    long usec;
    int type;
    server_socket *socket;
    struct sched_usec_link *prev;
    struct sched_usec_link *next;
} sched_usec_link;

typedef struct sched_usec_header {
    struct sched_usec_link *head;
    struct sched_usec_link *tail;
} sched_usec_header;


typedef struct sched_sec_link {
    long sec;
    sched_usec_header usec_header;
    struct sched_sec_link *prev;
    struct sched_sec_link *next;
} sched_sec_link;

typedef struct sched_header {
    struct sched_sec_link *head;
    struct sched_sec_link *tail;
} sched_header;


// binary protocol stuff

#define PFLAG_FROM      0x0001         // Is fromuid set

#define TstPFlagFrom(p) ((p)->header.flags & PFLAG_FROM)
#define SetPFlagFrom(p) ((p)->header.flags |= PFLAG_FROM)
#define ClrPFlagFrom(p) ((p)->header.flags &= (~PFLAG_FROM))

#define PMarker(p)    ((p)->header.marker)
#define PFromuid(p)   ((p)->header.fromuid)
#define PFlags(p)     ((p)->header.flags)
#define PArgc(p)      ((p)->header.payload_argc)
#define PType(p)      ((p)->header.payload_type)
#define PLength(p)    ((p)->header.payload_length)

typedef struct lt_packet_h {

    uint32_t marker;
    uint32_t fromuid;
    uint16_t flags;
    uint16_t payload_argc;
    uint16_t payload_type;
    uint16_t payload_length;

} lt_packet_h;

typedef struct lt_packet {

    lt_packet_h header;
    char payload[PAYLOAD_MTU];

} lt_packet;

#define makepacket(packet, type) do {           \
    (packet)->header.marker = SYNCH_MARKER;     \
    (packet)->header.fromuid = 0;               \
    (packet)->header.flags = 0;                 \
    ClrPFlagFrom((packet));                     \
    (packet)->header.payload_argc = 0;          \
    (packet)->header.payload_type = (type);     \
    (packet)->header.payload_length = 0;        \
} while(0)

#define makepacketfromuid(packet, type, uid) do {  \
    (packet)->header.marker = SYNCH_MARKER;        \
    (packet)->header.fromuid = (uid);              \
    (packet)->header.flags = 0;                    \
    SetPFlagFrom((packet));                        \
    (packet)->header.payload_argc = 0;             \
    (packet)->header.payload_type = (type);        \
    (packet)->header.payload_length = 0;           \
} while(0)


#endif
