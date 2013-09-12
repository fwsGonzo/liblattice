#include <stdlib.h>
#include <string.h>

#ifdef __linux__
        #include <sys/socket.h>
        #include <netinet/in.h>
        #include <arpa/inet.h>
#else
        #include <winsock2.h>
#endif

#include "lattice_config.h"
#include "serversocket.h"
#include "struct.h"
#include "send.h"
#include "globals.h"

int s_ping(struct server_socket *src, int argc, char **argv) {

    if (!src) return 0;

    if (sendto_one(src, "PONG\n")) return 1;

    return 0;

}

int s_p(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_p submess;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 6) return 0;

    mess.type = T_P;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = &submess;

    submess.wcoord.x = atoi(argv[0]);
    submess.wcoord.y = atoi(argv[1]);
    submess.wcoord.z = atoi(argv[2]);
    submess.bcoord.x = atoi(argv[3]);
    submess.bcoord.y = atoi(argv[4]);
    submess.bcoord.z = atoi(argv[5]);

    (*gcallback)(&mess);

    //if (sendto_one(src, "PONG\n")) return 1;

    return 0;

}


int s_quit(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_quit submess;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 2) return 0;

    mess.type = T_QUIT;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = &submess;

    submess.numeric = atoi(argv[0]);
    strncpy(submess.desc, argv[1] , sizeof(submess.desc));
    submess.desc[sizeof(submess.desc)-1]='\0';

    (*gcallback)(&mess);

    return 0;

}


int s_pc(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_pc submess;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 1) return 0;

    mess.type = T_PC;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = &submess;

    submess.color = (uint32_t)atoi(argv[0]);

    (*gcallback)(&mess);

    return 0;

}


int s_pr(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_pr submess;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 2) return 0;

    mess.type = T_PR;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = &submess;

    submess.rot.xrot = (HEAD_TYPE)atoi(argv[0]);
    submess.rot.yrot = (HEAD_TYPE)atoi(argv[1]);

    (*gcallback)(&mess);

    return 0;

}

int s_ph(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_ph submess;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 2) return 0;

    mess.type = T_PH;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = &submess;

    submess.hand.item_id = (HAND_TYPE)atoi(argv[0]);
    submess.hand.item_type = (HAND_TYPE)atoi(argv[1]);

    (*gcallback)(&mess);

    return 0;

}


int s_chat(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_chat submess;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 1) return 0;

    mess.type = T_CHAT;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = &submess;

    strncpy(submess.string, argv[0] , sizeof(submess.string));
    submess.string[sizeof(submess.string)-1]='\0';

    (*gcallback)(&mess);

    return 0;

}


int s_action(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_action submess;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 1) return 0;

    mess.type = T_ACTION;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = &submess;

    strncpy(submess.string, argv[0] , sizeof(submess.string));
    submess.string[sizeof(submess.string)-1]='\0';

    (*gcallback)(&mess);

    return 0;

}

