#include <stdlib.h>

#ifdef __linux__
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
#else
  #include <winsock2.h>
  #include <windows.h>
  #include <ws2tcpip.h>
  #include <stdlib.h>
  #include <stdio.h>
#endif

#include "lattice_config.h"
#include "serversocket.h"
#include "struct.h"
#include "send.h"
#include "globals.h"
#include "neighbors.h"
#include "macros.h"
#include "lattice_packet.h"

#if defined(_WIN32) && !defined(__MINGW32__)
  #include "client_commands.h"
#endif

int c_p(w_coord wcoord, b_coord bcoord) {

    server_socket *s;
    w_coord oldwcoord;
    int x;
    int y;
    int z;
    n_coord newcenter;
    void *p;
    lt_packet out_packet;

    lattice_message mess;
    lattice_bump submess;

    oldwcoord = lattice_player.wpos;

    newcenter = wcoord_to_ncoord(wcoord);

    if (!find_neighbor(newcenter)) {
        mess.type = T_BUMP;
        ClrFlagFrom(&mess);
        mess.fromuid = 0;
        mess.length = sizeof submess;
        mess.args = &submess;
        submess.wcoord = lattice_player.wpos;
        submess.bcoord = lattice_player.bpos;
        submess.bad_wcoord = wcoord;
        submess.bad_bcoord = bcoord;
        (*gcallback)(&mess);
        return 0;
    }

    lattice_player.wpos = wcoord;
    lattice_player.bpos = bcoord;

    if (user_is_within_outer_border(lattice_player.wpos, lattice_player.centeredon)) {
        // we are within the centered servers outer border
        for (x=0;x<3;x++)
        for (y=0;y<3;y++)
        for (z=0;z<3;z++) {
            if ((s=neighbor_table[x][y][z])) {
                if (user_is_within_outer_border(lattice_player.wpos, s->coord)) {
                    // we need to relay the P
                    makepacket(&out_packet, T_P);
                    p = &out_packet.payload;
                    if (!put_wx(&p, wcoord.x, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
                    if (!put_wy(&p, wcoord.y, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
                    if (!put_wz(&p, wcoord.z, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
                    if (!put_bx(&p, bcoord.x, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
                    if (!put_by(&p, bcoord.y, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
                    if (!put_bz(&p, bcoord.z, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
                    sendpacket(s, &out_packet);
                    //sendto_one(s, "P %d %d %d %d %d %d\n", wcoord.x, wcoord.y, wcoord.z, bcoord.x, bcoord.y, bcoord.z);
                } else {
                    // need to ENDP if we are walking away
                    if (user_is_within_outer_border(oldwcoord, s->coord)) {
                        makepacket(&out_packet, T_ENDP);
                        sendpacket(s, &out_packet);
                        //sendto_one(s, "ENDP\n");
                    }
                }
            }
        }
    } else {
        // we need to recenter to the neighboring server

        packet_recenter_neighbors(newcenter);

    }

    return 0;

}


int c_quit(char *reason) {

    void *p;
    lt_packet out_packet;

    makepacket(&out_packet, T_QUIT);

    if (reason)
        put_quitreason(&p, reason, &PLength(&out_packet), &PArgc(&out_packet));

    return sendpacket(neighbor_table[1][1][1], &out_packet);

}

int c_p_empty(void) {

    lt_packet out_packet;

    makepacket(&out_packet, T_P);

    return (sendpacket(neighbor_table[1][1][1], &out_packet));

}

int c_pc(uint32_t color) {

    void *p;
    lt_packet out_packet;

    lattice_player.usercolor = color;

    makepacket(&out_packet, T_PC);
    if (!put_usercolor(&p, color, &PLength(&out_packet), &PArgc(&out_packet))) return 1;

    return (sendpacket(neighbor_table[1][1][1], &out_packet));

}

int c_pr(head_rot rot) {

    void *p;
    lt_packet out_packet;

    lattice_player.hrot = rot;

    makepacket(&out_packet, T_PR);
    if (!put_xrot(&p, rot.xrot, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_yrot(&p, rot.yrot, &PLength(&out_packet), &PArgc(&out_packet))) return 1;

    return (sendpacket(neighbor_table[1][1][1], &out_packet));

}

int c_ph(hand_hold hand) {

    void *p;
    lt_packet out_packet;

    lattice_player.hhold = hand;

    makepacket(&out_packet, T_PH);
    if (!put_item_id(&p, hand.item_id, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_item_type(&p, hand.item_type, &PLength(&out_packet), &PArgc(&out_packet))) return 1;

    return (sendpacket(neighbor_table[1][1][1], &out_packet));

}

int c_chat(char *chat_text) {

    void *p;
    lt_packet out_packet;

    makepacket(&out_packet, T_CHAT);

    if (chat_text)
        put_chat(&p, chat_text, &PLength(&out_packet), &PArgc(&out_packet));

    return sendpacket(neighbor_table[1][1][1], &out_packet);

}

int c_pchat(uint32_t uid, char *chat_text) {

    void *p;
    lt_packet out_packet;

    makepacket(&out_packet, T_PCHAT);

    if (!put_uid(&p, uid, &PLength(&out_packet), &PArgc(&out_packet))) return 1;

    if (chat_text)
        put_chat(&p, chat_text, &PLength(&out_packet), &PArgc(&out_packet));

    return sendpacket(neighbor_table[1][1][1], &out_packet);


}

int c_action(char *action_text) {

    void *p;
    lt_packet out_packet;

    makepacket(&out_packet, T_ACTION);

    if (action_text)
        put_action(&p, action_text, &PLength(&out_packet), &PArgc(&out_packet));

    return sendpacket(neighbor_table[1][1][1], &out_packet);

}

int c_s(int32_t mid, int32_t sid) {

    void *p;
    lt_packet out_packet;

    makepacket(&out_packet, T_S);
    if (!put_mid(&p, mid, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_sid(&p, sid, &PLength(&out_packet), &PArgc(&out_packet))) return 1;

    return (sendpacket(neighbor_table[1][1][1], &out_packet));

}

int c_sc(int32_t csid) {

    void *p;
    lt_packet out_packet;

    makepacket(&out_packet, T_SC);
    if (!put_csid(&p, csid, &PLength(&out_packet), &PArgc(&out_packet))) return 1;

    return (sendpacket(neighbor_table[1][1][1], &out_packet));

}

int c_bo(w_coord wcoord, b_coord bcoord, int32_t id) {

    void *p;
    lt_packet out_packet;

    makepacket(&out_packet, T_BO);
    if (!put_wx(&p, wcoord.x, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_wy(&p, wcoord.y, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_wz(&p, wcoord.z, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_bx(&p, bcoord.x, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_by(&p, bcoord.y, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_bz(&p, bcoord.z, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_boid(&p, id, &PLength(&out_packet), &PArgc(&out_packet))) return 1;

    return (sendpacket(neighbor_table[1][1][1], &out_packet));

}

int c_mo(w_coord wcoord, b_coord bcoord, int32_t id, int32_t count) {

    void *p;
    lt_packet out_packet;

    makepacket(&out_packet, T_MO);
    if (!put_wx(&p, wcoord.x, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_wy(&p, wcoord.y, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_wz(&p, wcoord.z, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_bx(&p, bcoord.x, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_by(&p, bcoord.y, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_bz(&p, bcoord.z, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_moid(&p, id, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_mocount(&p, count, &PLength(&out_packet), &PArgc(&out_packet))) return 1;

    return (sendpacket(neighbor_table[1][1][1], &out_packet));

}

int c_badd(w_coord wcoord, b_coord bcoord, block_t block) {

    server_socket *dst;

    n_coord ncoord;

    void *p;
    lt_packet out_packet;

    ncoord = wcoord_to_ncoord(wcoord);

    dst = find_neighbor(ncoord);

    if (!dst) return 0;

    makepacket(&out_packet, T_BADD);
    if (!put_wx(&p, wcoord.x, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_wy(&p, wcoord.y, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_wz(&p, wcoord.z, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_bx(&p, bcoord.x, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_by(&p, bcoord.y, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_bz(&p, bcoord.z, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_block(&p, *(uint16_t *)&block, &PLength(&out_packet), &PArgc(&out_packet))) return 1;

    return (sendpacket(dst, &out_packet));

}

int c_bset(w_coord wcoord, b_coord bcoord, block_t block) {

    server_socket *dst;

    n_coord ncoord;

    void *p;
    lt_packet out_packet;

    ncoord = wcoord_to_ncoord(wcoord);

    dst = find_neighbor(ncoord);

    if (!dst) return 0;

    makepacket(&out_packet, T_BSET);
    if (!put_wx(&p, wcoord.x, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_wy(&p, wcoord.y, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_wz(&p, wcoord.z, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_bx(&p, bcoord.x, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_by(&p, bcoord.y, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_bz(&p, bcoord.z, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_block(&p, *(uint16_t *)&block, &PLength(&out_packet), &PArgc(&out_packet))) return 1;

    return (sendpacket(dst, &out_packet));

}

int c_brem(w_coord wcoord, b_coord bcoord) {

    server_socket *dst;

    n_coord ncoord;

    void *p;
    lt_packet out_packet;

    ncoord = wcoord_to_ncoord(wcoord);

    dst = find_neighbor(ncoord);

    if (!dst) return 0;

    makepacket(&out_packet, T_BREM);
    if (!put_wx(&p, wcoord.x, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_wy(&p, wcoord.y, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_wz(&p, wcoord.z, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_bx(&p, bcoord.x, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_by(&p, bcoord.y, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    if (!put_bz(&p, bcoord.z, &PLength(&out_packet), &PArgc(&out_packet))) return 1;

    return (sendpacket(dst, &out_packet));

}


int c_pmine(int mining) {

    void *p;
    lt_packet out_packet;

    lattice_player.mining = mining ? 1 : 0;

    makepacket(&out_packet, T_PMINE);
    if (!put_mining(&p, mining ? 1 : 0, &PLength(&out_packet), &PArgc(&out_packet))) return 1;
    return (sendpacket(neighbor_table[1][1][1], &out_packet));

}

int c_schat(char *schat_text) {

    void *p;
    lt_packet out_packet;

    makepacket(&out_packet, T_SCHAT);

    if (schat_text)
        put_chat(&p, schat_text, &PLength(&out_packet), &PArgc(&out_packet));

    return sendpacket(neighbor_table[1][1][1], &out_packet);

}

int c_lusers() {

    lt_packet out_packet;

    makepacket(&out_packet, T_LUSERS);
    return (sendpacket(neighbor_table[1][1][1], &out_packet));

}

