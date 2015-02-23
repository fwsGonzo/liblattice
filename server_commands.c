#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __linux__
#include <sys/time.h>
        #include <sys/socket.h>
        #include <netinet/in.h>
        #include <arpa/inet.h>
#include <stdio.h>
#else
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#ifndef __MINGW32__
#include "forwin.h"
#else
#include <sys/time.h>
#endif
#endif

#include "lattice_config.h"
#include "serversocket.h"
#include "struct.h"
#include "send.h"
#include "globals.h"
#include "neighbors.h"
#include "sched.h"
#include "macros.h"
#include "lattice_packet.h"

int s_ping(struct server_socket *src, lt_packet *packet) {

    lt_packet out_packet;

    if (!src || !packet) return 0;

    makepacket(&out_packet, T_PONG);
    if (sendpacket(src, &out_packet)) return 1;
    //if (sendto_one(src, "PONG\n")) return 1;

    return 0;

}

int s_pong(struct server_socket *src, lt_packet *packet) {

    struct timeval pingtime;

    if (!src || !packet) return 0;

    sched_deltypefrom(&sched, src, SCHED_SERVER_PING_TIMEOUT);

    if(SERVER_SEND_PING > 0) {

        gettimeofday(&now, NULL);
        pingtime = now;
        pingtime.tv_sec += SERVER_SEND_PING;

        if (!sched_add(src, SCHED_SERVER_SEND_PING, pingtime)) {
            closesock(src);
            return 1;
        }
    }

    return 0;

}

int s_iamserver(struct server_socket *src, lt_packet *packet) {

    struct timeval pingtime;

    void *p;
    uint16_t len;
    uint16_t argc;

    n_coord ncoord;
    char *ipstr;
    uint16_t port;
    char *servername;
    char *version;

    if (!src || !packet) return 0;

    SetFlagReg(src);

    sched_deltypefrom(&sched, src, SCHED_SERVER_HANDSHAKE_TIMEOUT);

    if (SERVER_SEND_PING > 0) {

        gettimeofday(&now, NULL);
        pingtime = now;
        pingtime.tv_sec += SERVER_SEND_PING;

        if (!sched_add(src, SCHED_SERVER_SEND_PING, pingtime)) {
            closesock(src);
            return 1;
        }

    }

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    if (PArgc(packet) < 7) return 0;

    if (!get_nx(&p, &(ncoord.x), &len, &argc)) return 0;
    if (!get_ny(&p, &(ncoord.y), &len, &argc)) return 0;
    if (!get_nz(&p, &(ncoord.z), &len, &argc)) return 0;
    if (!get_string(&p, &(ipstr), &len, &argc)) return 0;
    if (!get_uint16(&p, &port, &len, &argc)) return 0;
    if (!get_string(&p, &(servername), &len, &argc)) return 0;
    if (!get_string(&p, &(version), &len, &argc)) return 0;

    if (src->servername) free(src->servername);
    src->servername = strdup(servername);

    if (src->version) free(src->version);
    src->version = strdup(version);

    return 0;

}

int s_p(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;
    lattice_p submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    uid_t *pfrom;

    uid_link *uidlink;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (!pfrom) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_P;
    SetFlagFrom(&mess);
    mess.fromuid = *pfrom;

    if (PArgc(packet) < 6) {
        mess.length = 0;
        mess.args = NULL;
    } else {
        mess.length = sizeof submess;
        mess.args = &submess;

        if (!get_wx(&p, &(submess.wcoord.x), &len, &argc)) return 0;
        if (!get_wy(&p, &(submess.wcoord.y), &len, &argc)) return 0;
        if (!get_wz(&p, &(submess.wcoord.z), &len, &argc)) return 0;

        if (!get_bx(&p, &(submess.bcoord.x), &len, &argc)) return 0;
        if (!get_by(&p, &(submess.bcoord.y), &len, &argc)) return 0;
        if (!get_bz(&p, &(submess.bcoord.z), &len, &argc)) return 0;

        //submess.wcoord.x = atoi(argv[0]);
        //submess.wcoord.y = atoi(argv[1]);
        //submess.wcoord.z = atoi(argv[2]);
        //submess.bcoord.x = atoi(argv[3]);
        //submess.bcoord.y = atoi(argv[4]);
        //submess.bcoord.z = atoi(argv[5]);

        uidlink = uid_link_find(src, mess.fromuid);

        if (uidlink)
            uidlink->standing_on = ncoord_is_equal(src->coord, wcoord_to_ncoord(submess.wcoord));


    }

    (*gcallback)(&mess);

    return 0;

}


int s_quit(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;
    lattice_quit submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    char *str;

    uid_t *pfrom;

    uid_link *uidlink;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (!pfrom) return 0;

    if (PArgc(packet) < 2) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_QUIT;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_numeric(&p, &(submess.numeric), &len, &argc)) return 0;
    if (!get_quitreason(&p, &(str), &len, &argc)) return 0;
    strncpy(submess.desc, str , sizeof(submess.desc));
    submess.desc[sizeof(submess.desc)-1]='\0';

    uidlink = uid_link_find(src, mess.fromuid);

    if (uidlink)
        uid_link_del(src, uidlink);

    (*gcallback)(&mess);

    return 0;

}


int s_pc(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    lattice_pc submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    uid_t *pfrom;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (!pfrom) return 0;

    if (PArgc(packet) < 1) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_PC;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.length = sizeof submess;
    mess.args = &submess;

    //submess.color = (uint32_t)atoi(argv[0]);
    if (!get_usercolor(&p, &(submess.color), &len, &argc)) return 0;

    (*gcallback)(&mess);

    return 0;

}


int s_pr(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    lattice_pr submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    uid_t *pfrom;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (!pfrom) return 0;

    if (PArgc(packet) < 2) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_PR;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_xrot(&p, &(submess.rot.xrot), &len, &argc)) return 0;
    if (!get_yrot(&p, &(submess.rot.yrot), &len, &argc)) return 0;

    //submess.rot.xrot = (HEAD_TYPE)atoi(argv[0]);
    //submess.rot.yrot = (HEAD_TYPE)atoi(argv[1]);

    (*gcallback)(&mess);

    return 0;

}

int s_ph(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    lattice_ph submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    uid_t *pfrom;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (!pfrom) return 0;

    if (PArgc(packet) < 2) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_PH;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_item_id(&p, &(submess.hand.item_id), &len, &argc)) return 0;
    if (!get_item_type(&p, &(submess.hand.item_type), &len, &argc)) return 0;

    //submess.hand.item_id = (HAND_TYPE)atoi(argv[0]);
    //submess.hand.item_type = (HAND_TYPE)atoi(argv[1]);

    (*gcallback)(&mess);

    return 0;

}


int s_chat(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    lattice_chat submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    char *str;

    uid_t *pfrom;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (!pfrom) return 0;

    if (PArgc(packet) < 1) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_CHAT;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_chat(&p, &(str), &len, &argc)) return 0;
    strncpy(submess.chat_text, str , sizeof(submess.chat_text));
    submess.chat_text[sizeof(submess.chat_text)-1]='\0';

    (*gcallback)(&mess);

    return 0;

}


int s_action(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    lattice_action submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    char *str;

    uid_t *pfrom;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (!pfrom) return 0;

    if (PArgc(packet) < 1) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_ACTION;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_action(&p, &(str), &len, &argc)) return 0;
    strncpy(submess.action_text, str , sizeof(submess.action_text));
    submess.action_text[sizeof(submess.action_text)-1]='\0';

    (*gcallback)(&mess);

    return 0;

}

int s_s(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    lattice_s submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    uid_t *pfrom;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (!pfrom) return 0;

    if (PArgc(packet) < 2) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_S;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_mid(&p, &(submess.mid), &len, &argc)) return 0;
    if (!get_sid(&p, &(submess.sid), &len, &argc)) return 0;

    //submess.mid = (int32_t)atoi(argv[0]);
    //submess.sid = (int32_t)atoi(argv[1]);

    (*gcallback)(&mess);

    return 0;

}

int s_sc(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    lattice_sc submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    uid_t *pfrom;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (!pfrom) return 0;

    if (PArgc(packet) < 1) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_SC;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_csid(&p, &(submess.csid), &len, &argc)) return 0;
    //submess.csid = (int32_t)atoi(argv[0]);

    (*gcallback)(&mess);

    return 0;

}


int s_bo(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    lattice_bo submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    uid_t *pfrom;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (!pfrom) return 0;

    if (PArgc(packet) < 7) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_BO;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_wx(&p, &(submess.wcoord.x), &len, &argc)) return 0;
    if (!get_wy(&p, &(submess.wcoord.y), &len, &argc)) return 0;
    if (!get_wz(&p, &(submess.wcoord.z), &len, &argc)) return 0;

    if (!get_bx(&p, &(submess.bcoord.x), &len, &argc)) return 0;
    if (!get_by(&p, &(submess.bcoord.y), &len, &argc)) return 0;
    if (!get_bz(&p, &(submess.bcoord.z), &len, &argc)) return 0;

    if (!get_boid(&p, &(submess.id), &len, &argc)) return 0;

    //submess.wcoord.x = atoi(argv[0]);
    //submess.wcoord.y = atoi(argv[1]);
    //submess.wcoord.z = atoi(argv[2]);
    //submess.bcoord.x = atoi(argv[3]);
    //submess.bcoord.y = atoi(argv[4]);
    //submess.bcoord.z = atoi(argv[5]);
    //submess.id = atoi(argv[6]);

    (*gcallback)(&mess);

    return 0;

}


int s_mo(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    lattice_mo submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    uid_t *pfrom;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (PArgc(packet) < 8) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_MO;

    if (pfrom) {
        SetFlagFrom(&mess);
        mess.fromuid = *pfrom;
    } else {
        ClrFlagFrom(&mess);
        mess.fromuid = 0;
    }

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_wx(&p, &(submess.wcoord.x), &len, &argc)) return 0;
    if (!get_wy(&p, &(submess.wcoord.y), &len, &argc)) return 0;
    if (!get_wz(&p, &(submess.wcoord.z), &len, &argc)) return 0;

    if (!get_bx(&p, &(submess.bcoord.x), &len, &argc)) return 0;
    if (!get_by(&p, &(submess.bcoord.y), &len, &argc)) return 0;
    if (!get_bz(&p, &(submess.bcoord.z), &len, &argc)) return 0;

    if (!get_moid(&p, &(submess.id), &len, &argc)) return 0;
    if (!get_mocount(&p, &(submess.count), &len, &argc)) return 0;

    //submess.wcoord.x = atoi(argv[0]);
    //submess.wcoord.y = atoi(argv[1]);
    //submess.wcoord.z = atoi(argv[2]);
    //submess.bcoord.x = atoi(argv[3]);
    //submess.bcoord.y = atoi(argv[4]);
    //submess.bcoord.z = atoi(argv[5]);
    //submess.id = atoi(argv[6]);
    //submess.count = atoi(argv[7]);

    (*gcallback)(&mess);

    return 0;

}

int s_badd(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    lattice_badd submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    uid_t *pfrom;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (PArgc(packet) < 7) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_BADD;

    if (pfrom) {
        SetFlagFrom(&mess);
        mess.fromuid = *pfrom;
    } else {
        ClrFlagFrom(&mess);
        mess.fromuid = 0;
    }

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_wx(&p, &(submess.wcoord.x), &len, &argc)) return 0;
    if (!get_wy(&p, &(submess.wcoord.y), &len, &argc)) return 0;
    if (!get_wz(&p, &(submess.wcoord.z), &len, &argc)) return 0;

    if (!get_bx(&p, &(submess.bcoord.x), &len, &argc)) return 0;
    if (!get_by(&p, &(submess.bcoord.y), &len, &argc)) return 0;
    if (!get_bz(&p, &(submess.bcoord.z), &len, &argc)) return 0;

    if (!get_block(&p, &(submess.block), &len, &argc)) return 0;

    //submess.wcoord.x = atoi(argv[0]);
    //submess.wcoord.y = atoi(argv[1]);
    //submess.wcoord.z = atoi(argv[2]);
    //submess.bcoord.x = atoi(argv[3]);
    //submess.bcoord.y = atoi(argv[4]);
    //submess.bcoord.z = atoi(argv[5]);
    //submess.block.id = atoi(argv[6]);
    //submess.block.bf = atoi(argv[7]);


    (*gcallback)(&mess);

    return 0;

}


int s_bset(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    lattice_bset submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    uid_t *pfrom;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (PArgc(packet) < 7) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_BSET;

    if (pfrom) {
        SetFlagFrom(&mess);
        mess.fromuid = *pfrom;
    } else {
        ClrFlagFrom(&mess);
        mess.fromuid = 0;
    }

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_wx(&p, &(submess.wcoord.x), &len, &argc)) return 0;
    if (!get_wy(&p, &(submess.wcoord.y), &len, &argc)) return 0;
    if (!get_wz(&p, &(submess.wcoord.z), &len, &argc)) return 0;

    if (!get_bx(&p, &(submess.bcoord.x), &len, &argc)) return 0;
    if (!get_by(&p, &(submess.bcoord.y), &len, &argc)) return 0;
    if (!get_bz(&p, &(submess.bcoord.z), &len, &argc)) return 0;

    if (!get_block(&p, &(submess.block), &len, &argc)) return 0;

    //submess.wcoord.x = atoi(argv[0]);
    //submess.wcoord.y = atoi(argv[1]);
    //submess.wcoord.z = atoi(argv[2]);
    //submess.bcoord.x = atoi(argv[3]);
    //submess.bcoord.y = atoi(argv[4]);
    //submess.bcoord.z = atoi(argv[5]);
    //submess.block.id = atoi(argv[6]);
    //submess.block.bf = atoi(argv[7]);

    (*gcallback)(&mess);

    return 0;

}


int s_brem(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    lattice_brem submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    uid_t *pfrom;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (PArgc(packet) < 6) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_BREM;

    if (pfrom) {
        SetFlagFrom(&mess);
        mess.fromuid = *pfrom;
    } else {
        ClrFlagFrom(&mess);
        mess.fromuid = 0;
    }

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_wx(&p, &(submess.wcoord.x), &len, &argc)) return 0;
    if (!get_wy(&p, &(submess.wcoord.y), &len, &argc)) return 0;
    if (!get_wz(&p, &(submess.wcoord.z), &len, &argc)) return 0;

    if (!get_bx(&p, &(submess.bcoord.x), &len, &argc)) return 0;
    if (!get_by(&p, &(submess.bcoord.y), &len, &argc)) return 0;
    if (!get_bz(&p, &(submess.bcoord.z), &len, &argc)) return 0;

    //submess.wcoord.x = atoi(argv[0]);
    //submess.wcoord.y = atoi(argv[1]);
    //submess.wcoord.z = atoi(argv[2]);
    //submess.bcoord.x = atoi(argv[3]);
    //submess.bcoord.y = atoi(argv[4]);
    //submess.bcoord.z = atoi(argv[5]);

    (*gcallback)(&mess);

    return 0;

}

int s_pmine(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    lattice_pmine submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    mining_t mining;

    uid_t *pfrom;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (!pfrom) return 0;

    if (PArgc(packet) < 1) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_PMINE;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_mining(&p, &(mining), &len, &argc)) return 0;

    submess.mining = (mining ? 1 : 0);

    (*gcallback)(&mess);

    return 0;

}

int s_schat(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    lattice_schat submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    char * nickstr;
    char * chatstr;

    if (!src || !packet) return 0;

    if (PArgc(packet) < 3) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_SCHAT;

    ClrFlagFrom(&mess);

    mess.fromuid = 0;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_nickname(&p, &(nickstr), &len, &argc)) return 0;
    strncpy(submess.nickname, nickstr , sizeof(submess.nickname));
    submess.nickname[sizeof(submess.nickname)-1]='\0';

    if (!get_color(&p, &(submess.color), &len, &argc)) return 0;
    //submess.color = (uint32_t)atoi(argv[1]);

    if (!get_chat(&p, &(chatstr), &len, &argc)) return 0;
    strncpy(submess.schat_text, chatstr , sizeof(submess.schat_text));
    submess.schat_text[sizeof(submess.schat_text)-1]='\0';

    (*gcallback)(&mess);

    return 0;

}

int s_log(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    lattice_log submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    char * str;

    if (!src || !packet) return 0;

    if (PArgc(packet) < 1) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_LOG;

    ClrFlagFrom(&mess);

    mess.fromuid = 0;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_log(&p, &(str), &len, &argc)) return 0;
    strncpy(submess.log_text, str , sizeof(submess.log_text));
    submess.log_text[sizeof(submess.log_text)-1]='\0';

    (*gcallback)(&mess);

    return 0;

}


int s_satstep(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    lattice_satstep submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    if (!src || !packet) return 0;

    if (PArgc(packet) < 1) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_SATSTEP;

    ClrFlagFrom(&mess);

    mess.fromuid = 0;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_satstep(&p, &(submess.satstep), &len, &argc)) return 0;
    if (!get_sat(&p, &(submess.sat), &len, &argc)) return 0;

    //submess.satstep = (uint32_t)atoi(argv[0]);

    (*gcallback)(&mess);

    return 0;

}

int s_sat(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    lattice_sat submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    if (!src || !packet) return 0;

    if (PArgc(packet) < 1) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_SAT;

    ClrFlagFrom(&mess);

    mess.fromuid = 0;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_sat(&p, &(submess.sat), &len, &argc)) return 0;

    //submess.sat = (double)atof(argv[0]);

    (*gcallback)(&mess);

    return 0;

}


int s_fade(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    uid_link *uidlink;

    uid_t *pfrom;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (!pfrom) return 0;

    if (PArgc(packet) < 0) return 0;

    mess.type = T_FADE;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.length = 0;
    mess.args = NULL;

    uidlink = uid_link_find(src, mess.fromuid);

    if (uidlink)
        uid_link_del(src, uidlink);

    (*gcallback)(&mess);

    return 0;

}

int s_user(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;
    lattice_user submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    mining_t mining;

    //uid_link *uidlink;
    int standing;

    char *str;

    uid_t *pfrom;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (!pfrom) return 0;

    if (PArgc(packet) < 15) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_USER;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_model(&p, &(submess.model), &len, &argc)) return 0;
    //submess.model = (uint16_t) atoi(argv[0]);
    if (!get_color(&p, &(submess.color), &len, &argc)) return 0;
    //submess.color = (uint32_t) atoi(argv[1]);

    if (!get_nickname(&p, &(str), &len, &argc)) return 0;
    strncpy(submess.nickname, str , sizeof(submess.nickname));
    submess.nickname[sizeof(submess.nickname)-1]='\0';

    if (!get_wx(&p, &(submess.wpos.x), &len, &argc)) return 0;
    if (!get_wy(&p, &(submess.wpos.y), &len, &argc)) return 0;
    if (!get_wz(&p, &(submess.wpos.z), &len, &argc)) return 0;
    //submess.wpos.x = atoi(argv[3]);
    //submess.wpos.y = atoi(argv[4]);
    //submess.wpos.z = atoi(argv[5]);

    if (!get_bx(&p, &(submess.bpos.x), &len, &argc)) return 0;
    if (!get_by(&p, &(submess.bpos.y), &len, &argc)) return 0;
    if (!get_bz(&p, &(submess.bpos.z), &len, &argc)) return 0;
    //submess.bpos.x = atoi(argv[6]);
    //submess.bpos.y = atoi(argv[7]);
    //submess.bpos.z = atoi(argv[8]);

    if (!get_xrot(&p, &(submess.hrot.xrot), &len, &argc)) return 0;
    if (!get_yrot(&p, &(submess.hrot.yrot), &len, &argc)) return 0;
    //submess.hrot.xrot = atoi(argv[9]);
    //submess.hrot.yrot = atoi(argv[10]);

    if (!get_item_id(&p, &(submess.hhold.item_id), &len, &argc)) return 0;
    if (!get_item_type(&p, &(submess.hhold.item_type), &len, &argc)) return 0;
    //submess.hhold.item_id = atoi(argv[11]);
    //submess.hhold.item_type = atoi(argv[12]);

    if (!get_mining(&p, &(mining), &len, &argc)) return 0;
    submess.mining = mining;
    //submess.mining = atoi(argv[13]);

    if (!get_usercolor(&p, &(submess.usercolor), &len, &argc)) return 0;
    //submess.usercolor = (uint32_t) atoi(argv[14]);

    standing = ncoord_is_equal(src->coord, wcoord_to_ncoord(submess.wpos));

    uid_link_add_end(src, mess.fromuid, standing);

    //if (!uidlink) return 0;

    (*gcallback)(&mess);

    return 0;

}

int s_server(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;

    lattice_server submess;

    void *p;
    uint16_t len;
    uint16_t argc;
    lt_packet out_packet;

    char *str;

    server_socket *dst;

    if (!src || !packet) return 0;

    if (PArgc(packet) < 5) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    if (!ncoord_is_equal(src->coord, lattice_player.centeredon)) return 0;

    mess.type = T_SERVER;

    ClrFlagFrom(&mess);

    mess.fromuid = 0;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_nx(&p, &(submess.ncoord.x), &len, &argc)) return 0;
    if (!get_ny(&p, &(submess.ncoord.y), &len, &argc)) return 0;
    if (!get_nz(&p, &(submess.ncoord.z), &len, &argc)) return 0;
    //submess.ncoord.x = atoi(argv[0]);
    //submess.ncoord.y = atoi(argv[1]);
    //submess.ncoord.z = atoi(argv[2]);
    if (!get_string(&p, &(str), &len, &argc)) return 0;
    submess.ip.s_addr = inet_addr(str);
    if (!get_uint16(&p, &(submess.port), &len, &argc)) return 0;
    //submess.port = atoi(argv[4]);

    dst = connect_server(submess.ncoord, submess.ip, submess.port, NULL);

    if (!dst) return 0;

    add_neighbor(submess.ncoord, dst);

    //(*gcallback)(&mess);

    if (user_is_within_outer_border(lattice_player.wpos, submess.ncoord)) {
        // close enogh to be tracked...
        makepacket(&out_packet, T_SIDEDINTRO);
        p = &out_packet.payload;

        if (!put_uid(&p, lattice_player.userid, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_model(&p, lattice_player.model, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_color(&p, lattice_player.color, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_nickname(&p, lattice_player.nickname, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_burstdist(&p, lattice_player.burstdist, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_nx(&p, lattice_player.centeredon.x, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_ny(&p, lattice_player.centeredon.y, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_nz(&p, lattice_player.centeredon.z, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_wx(&p, lattice_player.wpos.x, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_wy(&p, lattice_player.wpos.y, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_wz(&p, lattice_player.wpos.z, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_bx(&p, lattice_player.bpos.x, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_by(&p, lattice_player.bpos.y, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_bz(&p, lattice_player.bpos.z, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_xrot(&p, lattice_player.hrot.xrot, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_yrot(&p, lattice_player.hrot.yrot, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_item_id(&p, lattice_player.hhold.item_id, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_item_type(&p, lattice_player.hhold.item_type, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_mining(&p, lattice_player.mining, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_usercolor(&p, lattice_player.usercolor, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_version(&p, LIBLATTICE_VERSION, &PLength(&out_packet), &PArgc(&out_packet))) return 1;

        if (sendpacket(dst, &out_packet)) return 1;

        //if (sendto_one(dst, "SIDEDINTRO %lu %d %u %s %d %u %u %u %u %u %u %d %d %d %d %d %d %d %d %u\n",
        //               lattice_player.userid,
        //               lattice_player.model,
        //               lattice_player.color,
        //               lattice_player.nickname,
        //               lattice_player.burstdist,
        //               lattice_player.centeredon.x,
        //               lattice_player.centeredon.y,
        //               lattice_player.centeredon.z,
        //               lattice_player.wpos.x,
        //               lattice_player.wpos.y,
        //               lattice_player.wpos.z,
        //               lattice_player.bpos.x,
        //               lattice_player.bpos.y,
        //               lattice_player.bpos.z,
        //               lattice_player.hrot.xrot,
        //               lattice_player.hrot.yrot,
        //               lattice_player.hhold.item_id,
        //               lattice_player.hhold.item_type,
        //               lattice_player.mining,
        //               lattice_player.usercolor)) return 1;

    } else {
        // too far to be tracked... just the basics....

        makepacket(&out_packet, T_SIDEDINTRO);
        p = &out_packet.payload;

        if (!put_uid(&p, lattice_player.userid, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_model(&p, lattice_player.model, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_color(&p, lattice_player.color, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_nickname(&p, lattice_player.nickname, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_burstdist(&p, lattice_player.burstdist, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_nx(&p, lattice_player.centeredon.x, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_ny(&p, lattice_player.centeredon.y, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_nz(&p, lattice_player.centeredon.z, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
        if (!put_version(&p, LIBLATTICE_VERSION, &PLength(&out_packet), &PArgc(&out_packet))) return 1;

        if (sendpacket(dst, &out_packet)) return 1;

        //if (sendto_one(dst, "SIDEDINTRO %lu %d %u %s %d %u %u %u\n",
        //               lattice_player.userid,
        //               lattice_player.model,
        //               lattice_player.color,
        //               lattice_player.nickname,
        //               lattice_player.burstdist,
        //               lattice_player.centeredon.x,
        //               lattice_player.centeredon.y,
        //               lattice_player.centeredon.z)) return 1;
    }

    return 0;

}


int s_delserver(struct server_socket *src, lt_packet *packet) {

    server_socket *dst;

    void *p;
    uint16_t len;
    uint16_t argc;

    n_coord coord;

    if (!src || !packet) return 0;

    if (PArgc(packet) < 3) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    if (!ncoord_is_equal(src->coord, lattice_player.centeredon)) return 0;

    if (!get_nx(&p, &(coord.x), &len, &argc)) return 0;
    if (!get_ny(&p, &(coord.y), &len, &argc)) return 0;
    if (!get_nz(&p, &(coord.z), &len, &argc)) return 0;

    //coord.x = atoi(argv[0]);
    //coord.y = atoi(argv[1]);
    //coord.z = atoi(argv[2]);

    dst = find_neighbor(coord);

    if (!dst) return 0;

    closesock(dst);

    return 0;

}

int s_moveto(struct server_socket *src, lt_packet *packet) {

    uid_link *uidlink;
    n_coord coord;

    lattice_message mess;
    lattice_quit submess;

    lattice_user usubmess;

    void *p;
    uint16_t len;
    uint16_t argc;

    char *str;

    mining_t mining;

    uid_t *pfrom;

    struct server_socket *dst;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (!pfrom) return 0;

    if (PArgc(packet) < 18) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    if (!get_nx(&p, &(coord.x), &len, &argc)) return 0;
    if (!get_ny(&p, &(coord.y), &len, &argc)) return 0;
    if (!get_nz(&p, &(coord.z), &len, &argc)) return 0;
    //coord.x = atoi(argv[0]);
    //coord.y = atoi(argv[1]);
    //coord.z = atoi(argv[2]);

    uidlink = uid_link_find(src, *pfrom);

    if (uidlink) {
        // normal case
        // we found the uid tracked with the originating server socket
        uid_link_del(src, uidlink);
    } else {
        // a MOVEFROM probably beat us
        // lets see if its already moved
        if (!uid_link_find_any(*pfrom)) {
            // this uid was not tracked??
            // intro user
            mess.type = T_USER;
            SetFlagFrom(&mess);
            mess.fromuid = *pfrom;
            mess.length = sizeof usubmess;
            mess.args = &usubmess;

            if (!get_model(&p, &(usubmess.model), &len, &argc)) return 0;
            //usubmess.model = (uint16_t) atoi(argv[3]);
            if (!get_color(&p, &(usubmess.color), &len, &argc)) return 0;
            //usubmess.color = (uint32_t) atoi(argv[4]);

            if (!get_nickname(&p, &(str), &len, &argc)) return 0;
            strncpy(usubmess.nickname, str , sizeof(usubmess.nickname));
            usubmess.nickname[sizeof(usubmess.nickname)-1]='\0';

            if (!get_wx(&p, &(usubmess.wpos.x), &len, &argc)) return 0;
            if (!get_wy(&p, &(usubmess.wpos.y), &len, &argc)) return 0;
            if (!get_wz(&p, &(usubmess.wpos.z), &len, &argc)) return 0;
            //usubmess.wpos.x = atoi(argv[6]);
            //usubmess.wpos.y = atoi(argv[7]);
            //usubmess.wpos.z = atoi(argv[8]);

            if (!get_bx(&p, &(usubmess.bpos.x), &len, &argc)) return 0;
            if (!get_by(&p, &(usubmess.bpos.y), &len, &argc)) return 0;
            if (!get_bz(&p, &(usubmess.bpos.z), &len, &argc)) return 0;
            //usubmess.bpos.x = atoi(argv[9]);
            //usubmess.bpos.y = atoi(argv[10]);
            //usubmess.bpos.z = atoi(argv[11]);

            if (!get_xrot(&p, &(usubmess.hrot.xrot), &len, &argc)) return 0;
            if (!get_yrot(&p, &(usubmess.hrot.yrot), &len, &argc)) return 0;
            //usubmess.hrot.xrot = atoi(argv[12]);
            //usubmess.hrot.yrot = atoi(argv[13]);

            if (!get_item_id(&p, &(usubmess.hhold.item_id), &len, &argc)) return 0;
            if (!get_item_type(&p, &(usubmess.hhold.item_type), &len, &argc)) return 0;
            //usubmess.hhold.item_id = atoi(argv[14]);
            //usubmess.hhold.item_type = atoi(argv[15]);

            if (!get_mining(&p, &(mining), &len, &argc)) return 0;
            usubmess.mining = mining;

            if (!get_usercolor(&p, &(usubmess.usercolor), &len, &argc)) return 0;
            //usubmess.usercolor = (uint32_t) atoi(argv[17]);

            (*gcallback)(&mess);
        }

    }

    dst = find_neighbor(coord);

    if (dst) {
        //normal case
        if (!uid_link_find(dst, *pfrom))
            uid_link_add_end(dst, *pfrom, 1);
    } else {
        // user is walking into a blackhole
        mess.type = T_QUIT;
        SetFlagFrom(&mess);
        mess.fromuid = *pfrom;
        mess.length = sizeof submess;
        mess.args = &submess;
        submess.numeric = 121; // NUM_BLACKHOLE_RECENTER
        strncpy(submess.desc, "User recentering to nonexistent space (blackhole)" , sizeof(submess.desc));
        submess.desc[sizeof(submess.desc)-1]='\0';
        (*gcallback)(&mess);

    }


    return 0;

}


int s_movefrom(struct server_socket *src, lt_packet *packet) {

    uid_link *uidlink;
    n_coord coord;

    void *p;
    uint16_t len;
    uint16_t argc;

    char *str;

    mining_t mining;

    uid_t *pfrom;

    struct server_socket *from;

    lattice_message mess;
    lattice_user submess;

    if (!src || !packet) return 0;

    if (TstPFlagFrom(packet))
        pfrom = &(packet->header.fromuid);
    else
        pfrom = NULL;

    if (!pfrom) return 0;

    if (PArgc(packet) < 18) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    if (!get_nx(&p, &(coord.x), &len, &argc)) return 0;
    if (!get_ny(&p, &(coord.y), &len, &argc)) return 0;
    if (!get_nz(&p, &(coord.z), &len, &argc)) return 0;
    //coord.x = atoi(argv[0]);
    //coord.y = atoi(argv[1]);
    //coord.z = atoi(argv[2]);

    from = find_neighbor(coord);

    if (from) {
        if ((uidlink=uid_link_find(from, *pfrom))) {
            uid_link_del(from, uidlink);
        } else {
            if (!uid_link_find_any(*pfrom)) {
                // this uid was not tracked??
                // intro user
                mess.type = T_USER;
                SetFlagFrom(&mess);
                mess.fromuid = *pfrom;
                mess.length = sizeof submess;
                mess.args = &submess;

                if (!get_model(&p, &(submess.model), &len, &argc)) return 0;
                //submess.model = (uint16_t) atoi(argv[3]);
                if (!get_color(&p, &(submess.color), &len, &argc)) return 0;
                //submess.color = (uint32_t) atoi(argv[4]);

                if (!get_nickname(&p, &(str), &len, &argc)) return 0;
                strncpy(submess.nickname, str , sizeof(submess.nickname));
                submess.nickname[sizeof(submess.nickname)-1]='\0';

                if (!get_wx(&p, &(submess.wpos.x), &len, &argc)) return 0;
                if (!get_wy(&p, &(submess.wpos.y), &len, &argc)) return 0;
                if (!get_wz(&p, &(submess.wpos.z), &len, &argc)) return 0;
                //submess.wpos.x = atoi(argv[6]);
                //submess.wpos.y = atoi(argv[7]);
                //submess.wpos.z = atoi(argv[8]);

                if (!get_bx(&p, &(submess.bpos.x), &len, &argc)) return 0;
                if (!get_by(&p, &(submess.bpos.y), &len, &argc)) return 0;
                if (!get_bz(&p, &(submess.bpos.z), &len, &argc)) return 0;
                //submess.bpos.x = atoi(argv[9]);
                //submess.bpos.y = atoi(argv[10]);
                //submess.bpos.z = atoi(argv[11]);

                if (!get_xrot(&p, &(submess.hrot.xrot), &len, &argc)) return 0;
                if (!get_yrot(&p, &(submess.hrot.yrot), &len, &argc)) return 0;
                //submess.hrot.xrot = atoi(argv[12]);
                //submess.hrot.yrot = atoi(argv[13]);

                if (!get_item_id(&p, &(submess.hhold.item_id), &len, &argc)) return 0;
                if (!get_item_type(&p, &(submess.hhold.item_type), &len, &argc)) return 0;
                //submess.hhold.item_id = atoi(argv[14]);
                //submess.hhold.item_type = atoi(argv[15]);

                if (!get_mining(&p, &(mining), &len, &argc)) return 0;
                submess.mining = mining;

                if (!get_usercolor(&p, &(submess.usercolor), &len, &argc)) return 0;
                //submess.usercolor = (uint32_t) atoi(argv[17]);

                (*gcallback)(&mess);
            }

        }
    } else {

        mess.type = T_USER;
        SetFlagFrom(&mess);
        mess.fromuid = *pfrom;
        mess.length = sizeof submess;
        mess.args = &submess;

        if (!get_model(&p, &(submess.model), &len, &argc)) return 0;
        //submess.model = (uint16_t) atoi(argv[3]);

        if (!get_color(&p, &(submess.color), &len, &argc)) return 0;
        //submess.color = (uint32_t) atoi(argv[4]);

        if (!get_nickname(&p, &(str), &len, &argc)) return 0;
        strncpy(submess.nickname, str , sizeof(submess.nickname));
        submess.nickname[sizeof(submess.nickname)-1]='\0';

        if (!get_wx(&p, &(submess.wpos.x), &len, &argc)) return 0;
        if (!get_wy(&p, &(submess.wpos.y), &len, &argc)) return 0;
        if (!get_wz(&p, &(submess.wpos.z), &len, &argc)) return 0;
        //submess.wpos.x = atoi(argv[6]);
        //submess.wpos.y = atoi(argv[7]);
        //submess.wpos.z = atoi(argv[8]);

        if (!get_bx(&p, &(submess.bpos.x), &len, &argc)) return 0;
        if (!get_by(&p, &(submess.bpos.y), &len, &argc)) return 0;
        if (!get_bz(&p, &(submess.bpos.z), &len, &argc)) return 0;
        //submess.bpos.x = atoi(argv[9]);
        //submess.bpos.y = atoi(argv[10]);
        //submess.bpos.z = atoi(argv[11]);

        if (!get_xrot(&p, &(submess.hrot.xrot), &len, &argc)) return 0;
        if (!get_yrot(&p, &(submess.hrot.yrot), &len, &argc)) return 0;
        //submess.hrot.xrot = atoi(argv[12]);
        //submess.hrot.yrot = atoi(argv[13]);

        if (!get_item_id(&p, &(submess.hhold.item_id), &len, &argc)) return 0;
        if (!get_item_type(&p, &(submess.hhold.item_type), &len, &argc)) return 0;
        //submess.hhold.item_id = atoi(argv[14]);
        //submess.hhold.item_type = atoi(argv[15]);

        if (!get_mining(&p, &(mining), &len, &argc)) return 0;
        submess.mining = mining;

        if (!get_usercolor(&p, &(submess.usercolor), &len, &argc)) return 0;
        //submess.usercolor = (uint32_t) atoi(argv[17]);

        (*gcallback)(&mess);

    }

    if (!uid_link_find(src, *pfrom))
        uid_link_add_end(src, *pfrom, 1);


    return 0;

}

int s_trackerfailure(struct server_socket *src, lt_packet *packet) {
    server_socket *s;
    int x;
    int y;
    int z;

    lt_packet out_packet;

    if (!src || !packet) return 0;

    if (ncoord_is_equal(src->coord, lattice_player.centeredon)) {
        // this is a centered server
        if (ncoord_is_equal(src->coord, wcoord_to_ncoord(lattice_player.wpos))) {
            // i am standing on this server. slideover kill everything else.
            for (x=0;x<3;x++) for (y=0;y<3;y++) for (z=0;z<3;z++) {
                if ((s=neighbor_table[x][y][z])) {
                    if (s != src) {
                        makepacket(&out_packet, T_SLIDEOVER);
                        sendpacket(s, &out_packet);
                        //sendto_one(s, "SLIDEOVER\n");
                        neighbor_table[x][y][z] = NULL;
                    }
                }
            }
        } else {
            // i am NOT standing on this server. recenter away. slideover kill old center.
            recenter_neighbors(wcoord_to_ncoord(lattice_player.wpos));
            for (x=0;x<3;x++) for (y=0;y<3;y++) for (z=0;z<3;z++) {
                if ((s=neighbor_table[x][y][z])) {
                    if (s == src) {
                        makepacket(&out_packet, T_SLIDEOVER);
                        sendpacket(s, &out_packet);
                        //sendto_one(s, "SLIDEOVER\n");
                        neighbor_table[x][y][z] = NULL;
                    }
                }
            }
        }
    } else {
        // this is a sided server
        if (ncoord_is_equal(src->coord, wcoord_to_ncoord(lattice_player.wpos))) {
            // i am standing on this server. recenter twards. slideover kill everything else.
            recenter_neighbors(src->coord);
            for (x=0;x<3;x++) for (y=0;y<3;y++) for (z=0;z<3;z++) {
                if ((s=neighbor_table[x][y][z])) {
                    if (s != src) {
                        makepacket(&out_packet, T_SLIDEOVER);
                        sendpacket(s, &out_packet);
                        //sendto_one(s, "SLIDEOVER\n");
                        neighbor_table[x][y][z] = NULL;
                    }
                }
            }

        } else {
            // i am NOT standing on this server. slideover kill only this server.
            for (x=0;x<3;x++) for (y=0;y<3;y++) for (z=0;z<3;z++) {
                if ((s=neighbor_table[x][y][z])) {
                    if (s == src) {
                        makepacket(&out_packet, T_SLIDEOVER);
                        sendpacket(s, &out_packet);
                        //sendto_one(s, "SLIDEOVER\n");
                        neighbor_table[x][y][z] = NULL;
                    }
                }
            }

        }

    }


    return 0;

}

int s_closing(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;
    lattice_closing submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    char *str;

    n_coord server_coord;
    n_coord newcenter;

    if (!src || !packet) return 0;

    if (PArgc(packet) < 2) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    server_coord.x = src->coord.x;
    server_coord.y = src->coord.y;
    server_coord.z = src->coord.z;

    if (ncoord_is_equal(server_coord, lattice_player.centeredon)) {
        // this is my centered server
        if ( ncoord_is_equal(server_coord, wcoord_to_ncoord(lattice_player.wpos)) )  {
            // i am standing on my centered server. i must die.
            mess.type = T_CLOSING;
            ClrFlagFrom(&mess);
            mess.fromuid = 0;
            mess.length = sizeof submess;
            mess.args = &submess;

            if (!get_numeric(&p, &(submess.numeric), &len, &argc)) return 0;
            //submess.numeric = atoi(argv[0]);
            if (!get_closingreason(&p, &(str), &len, &argc)) return 0;
            strncpy(submess.desc, str, sizeof(submess.desc));
            submess.desc[sizeof(submess.desc)-1]='\0';

            (*gcallback)(&mess);

            disconnect_servers();
            return 1;
        } else {
            // i am not standing on my centered server. its dieing. i need to recenter.
            newcenter = wcoord_to_ncoord(lattice_player.wpos);
            if (find_neighbor(newcenter)) {
                // i am standing on a neighboring server i can force a recentering to
                recenter_neighbors(newcenter);
            } else {
                // the server i am standing on is somehow not there?
                // this case should not happen
                //disconnect_servers();
            }
        }
    } else {
        // this is one of my sided servers
        if ( ncoord_is_equal(server_coord, wcoord_to_ncoord(lattice_player.wpos)) )  {
            // i am standing on this sided server. i must die.
            mess.type = T_CLOSING;
            ClrFlagFrom(&mess);
            mess.fromuid = 0;
            mess.length = sizeof submess;
            mess.args = &submess;

            if (!get_numeric(&p, &(submess.numeric), &len, &argc)) return 0;
            //submess.numeric = atoi(argv[0]);

            if (!get_closingreason(&p, &(str), &len, &argc)) return 0;
            strncpy(submess.desc, str, sizeof(submess.desc));
            submess.desc[sizeof(submess.desc)-1]='\0';

            (*gcallback)(&mess);


            disconnect_servers();
            return 1;
        } else {
            // i am not standing on this server. i will survive this. do nothing.
        }

    }


    return 0;

}

#ifdef GONZO_HACK

int s_emptysector(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;
    lattice_brem submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    if (!src || !packet) return 0;

    if (PArgc(packet) < 3) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_BREM;

    ClrFlagFrom(&mess);

    mess.fromuid = 0;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_wx(&p, &(submess.wcoord.x), &len, &argc)) return 0;
    if (!get_wy(&p, &(submess.wcoord.y), &len, &argc)) return 0;
    if (!get_wz(&p, &(submess.wcoord.z), &len, &argc)) return 0;

    for(submess.bcoord.x = 0; submess.bcoord.x < BLOCKSDB_COUNT_BX; submess.bcoord.x++)
    for(submess.bcoord.z = 0; submess.bcoord.z < BLOCKSDB_COUNT_BZ; submess.bcoord.z++)
    for(submess.bcoord.y = 0; submess.bcoord.y < BLOCKSDB_COUNT_BY; submess.bcoord.y++)
        (*gcallback)(&mess);

    return 0;

}

#else

int s_emptysector(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;
    lattice_emptysector submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    if (!src || !packet) return 0;

    if (PArgc(packet) < 3) return 0;

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_EMPTYSECTOR;

    ClrFlagFrom(&mess);

    mess.fromuid = 0;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_wx(&p, &(submess.wcoord.x), &len, &argc)) return 0;
    if (!get_wy(&p, &(submess.wcoord.y), &len, &argc)) return 0;
    if (!get_wz(&p, &(submess.wcoord.z), &len, &argc)) return 0;

    (*gcallback)(&mess);

    return 0;

}

#endif

#ifdef GONZO_HACK

int s_sector(struct server_socket *src, lt_packet *packet) {

    lattice_message baddmess;
    lattice_badd baddsubmess;

    lattice_message bsetmess;
    lattice_bset bsetsubmess;

    lattice_message bremmess;
    lattice_brem bremsubmess;

    void *p;
    uint16_t len;
    uint16_t argc;

    w_coord wcoord;

    int bx;
    int by;
    int bz;

    block_t id;
    block_t* b;
    block_t s[BLOCKSDB_BLOCKS_COUNT]; // bx bz by


    if (!src || !packet) return 0;

    if (PArgc(packet) < 4) return 0; // wx wy wz + b[2048]

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    baddmess.type = T_BADD;
    ClrFlagFrom(&baddmess);
    baddmess.fromuid = 0;
    baddmess.length = sizeof baddsubmess;
    baddmess.args = &baddsubmess;

    bsetmess.type = T_BSET;
    ClrFlagFrom(&bsetmess);
    bsetmess.fromuid = 0;
    bsetmess.length = sizeof bsetsubmess;
    bsetmess.args = &bsetsubmess;

    bremmess.type = T_BREM;
    ClrFlagFrom(&bremmess);
    bremmess.fromuid = 0;
    bremmess.length = sizeof bremsubmess;
    bremmess.args = &bremsubmess;


    if (!get_wx(&p, &(wcoord.x), &len, &argc)) return 0;
    if (!get_wy(&p, &(wcoord.y), &len, &argc)) return 0;
    if (!get_wz(&p, &(wcoord.z), &len, &argc)) return 0;

    baddsubmess.wcoord.x = wcoord.x;
    baddsubmess.wcoord.y = wcoord.y;
    baddsubmess.wcoord.z = wcoord.z;

    bsetsubmess.wcoord.x = wcoord.x;
    bsetsubmess.wcoord.y = wcoord.y;
    bsetsubmess.wcoord.z = wcoord.z;

    bremsubmess.wcoord.x = wcoord.x;
    bremsubmess.wcoord.y = wcoord.y;
    bremsubmess.wcoord.z = wcoord.z;

    b = s;

    if (!get_sector(&p, b, &len, &argc)) return 0;

    for (bx = 0; bx < BLOCKSDB_COUNT_BX; bx++)
    for (bz = 0; bz < BLOCKSDB_COUNT_BZ; bz++)
    for (by = 0; by < BLOCKSDB_COUNT_BY; by++) {

        id = blockid(*b);

        if (!id) {
            bremsubmess.bcoord.x = bx;
            bremsubmess.bcoord.y = by;
            bremsubmess.bcoord.z = bz;
            (*gcallback)(&bremmess);
        } else {
            switch(id) {
                case 43:
                case 44:
                case 62:
                case 99:
                case 552:
                case 800:
                case 820:
                    baddsubmess.bcoord.x = bx;
                    baddsubmess.bcoord.y = by;
                    baddsubmess.bcoord.z = bz;
                    baddsubmess.block = *b;
                    (*gcallback)(&baddmess);
                break;
                default:
                    bsetsubmess.bcoord.x = bx;
                    bsetsubmess.bcoord.y = by;
                    bsetsubmess.bcoord.z = bz;
                    bsetsubmess.block = *b;
                    (*gcallback)(&bsetmess);
                break;
            }

        }


        b++; // next block

    }

    //(*gcallback)(&mess);

    return 0;

}

#else

int s_sector(struct server_socket *src, lt_packet *packet) {

    lattice_message mess;
    lattice_sector submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    int bx;
    int by;
    int bz;

    block_t id;
    block_t* b;

    if (!src || !packet) return 0;

    if (PArgc(packet) < 4) return 0; // wx wy wz + b[2048]

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_SECTOR;

    ClrFlagFrom(&mess);

    mess.fromuid = 0;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_wx(&p, &(submess.wcoord.x), &len, &argc)) return 0;
    if (!get_wy(&p, &(submess.wcoord.y), &len, &argc)) return 0;
    if (!get_wz(&p, &(submess.wcoord.z), &len, &argc)) return 0;

    submess.s.blocks = 0;
    submess.s.lights = 0;
    submess.s.hardsolid = BLOCKSDB_MAX_HARDSOLID;
    submess.s.special = 0;
    submess.s.version = 0;

    b = &submess.s.b[0][0][0];

    if (!get_sector(&p, b, &len, &argc)) return 0;

    for (bx = 0; bx < BLOCKSDB_COUNT_BX; bx++)
    for (bz = 0; bz < BLOCKSDB_COUNT_BZ; bz++)
    for (by = 0; by < BLOCKSDB_COUNT_BY; by++) {

        id = blockid(*b);

        // check if block isn't solid
        if (id == BLOCKSDB_AIR || id >= BLOCKSDB_HALFBLOCK_START) {
            // remove solid value from potential edge
            if (bz == 0)
                submess.s.hardsolid &= (63-32);
            else if (bz == BLOCKSDB_COUNT_BZ-1)
                submess.s.hardsolid &= (63-16);

            if (by == 0)
                submess.s.hardsolid &= (63-8);
            else if (by == BLOCKSDB_COUNT_BY-1)
                submess.s.hardsolid &= (63-4);

            if (bx == 0)
                submess.s.hardsolid &= (63-2);
            else if (bx == BLOCKSDB_COUNT_BX-1)
                submess.s.hardsolid &= (63-1);
        } // non-solid block

        if (id != BLOCKSDB_AIR)
        {
            // increase light count if isLight()
            if (islight(id)) submess.s.lights++;
            // increase non-air block count
            submess.s.blocks++;
        }

        b++; // next block

    }

    (*gcallback)(&mess);

    return 0;

}

#endif

int s_flatland(struct server_socket *src, lt_packet *packet) {

#ifndef GONZO_HACK

    lattice_message mess;
    lattice_flatland submess;

    void *p;
    uint16_t len;
    uint16_t argc;

    if (!src || !packet) return 0;

    if (PArgc(packet) < 3) return 0; // wx wz + flatlands[]

    argc = packet->header.payload_argc;
    len = packet->header.payload_length;
    p = packet->payload;

    mess.type = T_FLATLAND;

    ClrFlagFrom(&mess);

    mess.fromuid = 0;

    mess.length = sizeof submess;
    mess.args = &submess;

    if (!get_wx(&p, &(submess.fcoord.x), &len, &argc)) return 0;
    if (!get_wz(&p, &(submess.fcoord.z), &len, &argc)) return 0;

    if (!get_flatsector(&p, submess.fdata, &len, &argc)) return 0;

    (*gcallback)(&mess);

#endif

    return 0;

}

