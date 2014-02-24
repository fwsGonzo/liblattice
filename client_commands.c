#include <stdlib.h>

#ifdef __linux__
        #include <sys/socket.h>
        #include <netinet/in.h>
        #include <arpa/inet.h>
#else
#include <windows.h>
#include <winsock2.h>
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

int c_p(w_coord wcoord, b_coord bcoord) {

    server_socket *s;
    w_coord oldwcoord;
    int x;
    int y;
    int z;
    n_coord newcenter;

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
                    sendto_one(s, "P %d %d %d %d %d %d\n", wcoord.x, wcoord.y, wcoord.z, bcoord.x, bcoord.y, bcoord.z);
                } else {
                    // need to ENDP if we are walking away
                    if (user_is_within_outer_border(oldwcoord, s->coord)) {
                        sendto_one(s, "ENDP\n");
                    }
                }
            }
        }
    } else {
        // we need to recenter to the neighboring server

        recenter_neighbors(newcenter);

    }

    return 0;

}


int c_quit(char *reason) {

    if (reason)
        return (sendto_one(neighbor_table[1][1][1], "QUIT :%s\n", reason));
    else
        return (sendto_one(neighbor_table[1][1][1], "QUIT\n"));

}

int c_p_empty(void) {

    return (sendto_one(neighbor_table[1][1][1], "P\n"));

}

int c_pc(uint32_t color) {

    lattice_player.usercolor = color;

    return (sendto_one(neighbor_table[1][1][1], "PC %d\n", color));

}

int c_pr(head_rot rot) {

    lattice_player.hrot = rot;

    return (sendto_one(neighbor_table[1][1][1], "PR %d %d\n", rot.xrot, rot.yrot));

}

int c_ph(hand_hold hand) {

    lattice_player.hhold = hand;

    return (sendto_one(neighbor_table[1][1][1], "PH %d %d\n", hand.item_id, hand.item_type));

}

int c_chat(char *string) {

    if (string)
        return (sendto_one(neighbor_table[1][1][1], "CHAT :%s\n", string));
    else
        return (sendto_one(neighbor_table[1][1][1], "CHAT :\n"));

}

int c_action(char *string) {

    if (string)
        return (sendto_one(neighbor_table[1][1][1], "ACTION :%s\n", string));
    else
        return (sendto_one(neighbor_table[1][1][1], "ACTION :\n"));

}

int c_s(int32_t mid, int32_t sid) {

    return (sendto_one(neighbor_table[1][1][1], "S %d %d\n", mid, sid));

}

int c_sc(int32_t csid) {

    return (sendto_one(neighbor_table[1][1][1], "SC %d\n", csid));

}

int c_bo(w_coord wcoord, b_coord bcoord, int32_t id) {

    return (sendto_one(neighbor_table[1][1][1], "BO %d %d %d %d %d %d $d\n", wcoord.x, wcoord.y, wcoord.z, bcoord.x, bcoord.y, bcoord.z, id));

}

int c_mo(w_coord wcoord, b_coord bcoord, int32_t id, int32_t count) {

    return (sendto_one(neighbor_table[1][1][1], "MO %d %d %d %d %d %d $d %d\n", wcoord.x, wcoord.y, wcoord.z, bcoord.x, bcoord.y, bcoord.z, id, count));

}

int c_badd(w_coord wcoord, b_coord bcoord, block_t block) {

    server_socket *p;

    n_coord ncoord;

    ncoord = wcoord_to_ncoord(wcoord);

    p = find_neighbor(ncoord);

    if (!p) return 0;

    return (sendto_one(p, "BADD %d %d %d %d %d %d %d %d\n", wcoord.x, wcoord.y, wcoord.z, bcoord.x, bcoord.y, bcoord.z, block.id, block.bf));

}

int c_bset(w_coord wcoord, b_coord bcoord, block_t block) {

    server_socket *p;

    n_coord ncoord;

    ncoord = wcoord_to_ncoord(wcoord);

    p = find_neighbor(ncoord);

    if (!p) return 0;

    return (sendto_one(p, "BSET %d %d %d %d %d %d %d %d\n", wcoord.x, wcoord.y, wcoord.z, bcoord.x, bcoord.y, bcoord.z, block.id, block.bf));

}

int c_brem(w_coord wcoord, b_coord bcoord) {

    server_socket *p;

    n_coord ncoord;

    ncoord = wcoord_to_ncoord(wcoord);

    p = find_neighbor(ncoord);

    if (!p) return 0;

    return (sendto_one(p, "BREM %d %d %d %d %d %d\n", wcoord.x, wcoord.y, wcoord.z, bcoord.x, bcoord.y, bcoord.z));

}


int c_pmine(int mining) {

    lattice_player.mining = mining ? 1 : 0;

    return (sendto_one(neighbor_table[1][1][1], "PMINE %d\n", mining ? 1 : 0));

}

int c_schat(char *string) {

    if (string)
        return (sendto_one(neighbor_table[1][1][1], "SCHAT :%s\n", string));
    else
        return (sendto_one(neighbor_table[1][1][1], "SCHAT :\n"));

}

int c_lusers() {

    return (sendto_one(neighbor_table[1][1][1], "LUSERS\n"));

}

