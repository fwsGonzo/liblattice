#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <stdint.h>
#include "lattice_config.h"
#include "coordinates.h"

typedef uint16_t port_t;

#define SOCKET_UNKNOWN             0
#define SOCKET_SERVER_CONNECTING   1
#define SOCKET_SERVER              2
#define SOCKET_CLIENT              3
#define SOCKET_TRACKER             4

// socket level flags
#define FLAG_REG      0x00000001
#define TstFlagReg(s) ((s)->flags & FLAG_REG)
#define SetFlagReg(s) ((s)->flags |= FLAG_REG)
#define ClrFlagReg(s) ((s)->flags &= (~FLAG_REG))

#define FLAG_CLOSED   0x00000002
#define TstFlagClosed(s) ((s)->flags & FLAG_CLOSED)
#define SetFlagClosed(s) ((s)->flags |= FLAG_CLOSED)
#define ClrFlagClosed(s) ((s)->flags &= (~FLAG_CLOSED))

typedef struct sendq_link {
    char sendq_block[SENDQ_BLOCK];
    int length;
    int offset;
    struct sendq_link *next;
} sendq_link;

typedef struct uid_link {
    uint32_t userid;
    int standing_on;
    struct uid_link *prev;
    struct uid_link *next;
} uid_link;

typedef struct server_socket {
    int socket;

    //char writebuf[WRITE_LENGTH]; //block buffer to fill up for outbounds....
    char *writebuf;
    int wlen;

    //char rmsg[MTU + 1]; // +1 for \0
    char *rmsg;
    int rlen;

    sendq_link *sendq_head;
    sendq_link *sendq_tail;
    int sendq_length;
    int sendq_max;

    struct in_addr ip;
    port_t port;

    uint32_t flags;

    int type;

    n_coord coord;

    port_t c_port;

    struct uid_link *uidlist_head;
    struct uid_link *uidlist_tail;

    char *servername;

    char *version;

    int burstdist;

} server_socket;

typedef int socket_error_t;


extern uid_link * uid_link_add_front(server_socket *s, uint32_t uid, int standing_on);
extern uid_link * uid_link_add_end(server_socket *s, uint32_t uid, int standing_on);
extern void uid_link_del(server_socket *s, uid_link *link);
extern void uid_link_delall(server_socket *s);
extern uid_link *uid_link_find_any(uint32_t uid);
extern uid_link *uid_link_find(server_socket *s, uint32_t uid);
extern uid_link *uid_link_rfind(server_socket *s, uint32_t uid);

extern void closesock(server_socket *s);
extern void closeallservers(void);



#endif
