#include <stdlib.h>

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
        mess.args = &submess;
        submess.wcoord = lattice_player.wpos;
        submess.bcoord = lattice_player.bpos;
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

        // get rid of the servers that are leaving us

        for (x=0;x<3;x++)
        for (y=0;y<3;y++)
        for (z=0;z<3;z++) {
            if ((s=neighbor_table[x][y][z])) {
                if (!serv_in_range_of_serv(newcenter, s->coord)) {
                    // we are falling off the edge of the neighbor table
                    sendto_one(s, "SLIDEOVER\n");
                    neighbor_table[x][y][z] = NULL;
                }
            }
        }

        // neighbor table matrix translation

        if (serv_in_range_of_serv(lattice_player.centeredon, newcenter)) {

            // X plane

            if (newcenter.x > lattice_player.centeredon.x) {
                for (x=1;x<=2;x++) for (y=0;y<=2;y++) for (z=0;z<=2;z++) {
                    neighbor_table[x-1][y][z] = neighbor_table[x][y][z];
                    neighbor_table[x][y][z] = NULL;
                }
            }
            if (newcenter.x < lattice_player.centeredon.x) {
                for (x=1;x>=0;x--) for (y=2;y>=0;y--) for (z=2;z>=0;z--) {
                    neighbor_table[x+1][y][z] = neighbor_table[x][y][z];
                    neighbor_table[x][y][z] = NULL;
                }

            }

            // Y plane

            if (newcenter.y > lattice_player.centeredon.y) {
                for (x=0;x<=2;x++) for (y=1;y<=2;y++) for (z=0;z<=2;z++) {
                    neighbor_table[x][y-1][z] = neighbor_table[x][y][z];
                    neighbor_table[x][y][z] = NULL;
                }
            }
            if (newcenter.y < lattice_player.centeredon.y) {
                for (x=2;x>=0;x--) for (y=1;y>=0;y--) for (z=2;z>=0;z--) {
                    neighbor_table[x][y+1][z] = neighbor_table[x][y][z];
                    neighbor_table[x][y][z] = NULL;
                }

            }

            // Z plane

            if (newcenter.z > lattice_player.centeredon.z) {
                for (x=0;x<=2;x++) for (y=0;y<=2;y++) for (z=1;z<=2;z++) {
                    neighbor_table[x][y][z-1] = neighbor_table[x][y][z];
                    neighbor_table[x][y][z] = NULL;
                }
            }
            if (newcenter.z < lattice_player.centeredon.z) {
                for (x=2;x>=0;x--) for (y=2;y>=0;y--) for (z=1;z>=0;z--) {
                    neighbor_table[x][y][z+1] = neighbor_table[x][y][z];
                    neighbor_table[x][y][z] = NULL;
                }

            }

        }

        // send out CENTERED AND SIDED MOVEs

        for (x=0;x<3;x++)
        for (y=0;y<3;y++)
        for (z=0;z<3;z++) {
            if ((s=neighbor_table[x][y][z])) {
                if (s == neighbor_table[1][1][1]) {
                    // this is a new center
                    sendto_one(s,
                               //                     wx wy wz bx by bz  HEAD  HAND
                               "CENTEREDMOVE %u %u %u %u %u %u %d %d %d %d %d %d %d %d %u\n",
                               lattice_player.centeredon.x,
                               lattice_player.centeredon.y,
                               lattice_player.centeredon.z,
                               lattice_player.wpos.x,
                               lattice_player.wpos.y,
                               lattice_player.wpos.z,
                               lattice_player.bpos.x,
                               lattice_player.bpos.y,
                               lattice_player.bpos.z,
                               lattice_player.hrot.xrot,
                               lattice_player.hrot.yrot,
                               lattice_player.hhold.item_id,
                               lattice_player.hhold.item_type,
                               lattice_player.mining,
                               lattice_player.usercolor);
                } else {
                    // this is a moved side
                    if (user_is_within_outer_border(lattice_player.wpos, s->coord)) {
                        sendto_one(s,
                                   //                  wx wy wz bx by bz  HEAD  HAND
                                   "SIDEDMOVE %u %u %u %u %u %u %d %d %d %d %d %d %d %d %u\n",
                                   newcenter.x,
                                   newcenter.y,
                                   newcenter.z,
                                   lattice_player.wpos.x,
                                   lattice_player.wpos.y,
                                   lattice_player.wpos.z,
                                   lattice_player.bpos.x,
                                   lattice_player.bpos.y,
                                   lattice_player.bpos.z,
                                   lattice_player.hrot.xrot,
                                   lattice_player.hrot.yrot,
                                   lattice_player.hhold.item_id,
                                   lattice_player.hhold.item_type,
                                   lattice_player.mining,
                                   lattice_player.usercolor);

                    } else {
                        sendto_one(s,
                                   "SIDEDMOVE %u %u %u\n",
                                   newcenter.x,
                                   newcenter.y,
                                   newcenter.z);
                    } // if (user_is_within_outer_border(lattice_player.wpos, s->coord))
                } // if (s == neighbor_table[1][1][1])
            } // if ((s=neighbor_table[x][y][z]))
        } // for ()

        // update lattice_player

        lattice_player.centeredon.x = neighbor_table[1][1][1]->coord.x;
        lattice_player.centeredon.y = neighbor_table[1][1][1]->coord.y;
        lattice_player.centeredon.z = neighbor_table[1][1][1]->coord.z;
        lattice_player.my_min_wcoord.x = lattice_player.centeredon.x << 8;
        lattice_player.my_min_wcoord.y = lattice_player.centeredon.y << 8;
        lattice_player.my_min_wcoord.z = lattice_player.centeredon.z << 8;
        lattice_player.my_max_wcoord.x = (lattice_player.centeredon.x << 8) | 0x000000FF;
        lattice_player.my_max_wcoord.y = (lattice_player.centeredon.y << 8) | 0x000000FF;
        lattice_player.my_max_wcoord.z = (lattice_player.centeredon.z << 8) | 0x000000FF;

    }

    return 0;

}


int c_quit(char *reason) {

    if (reason)
        return (sendto_one(neighbor_table[1][1][1], "QUIT :%s\n", reason));
    else
        return (sendto_one(neighbor_table[1][1][1], "QUIT\n"));

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

