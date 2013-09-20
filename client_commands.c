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


int c_p(w_coord wcoord, b_coord bcoord) {

    return (sendto_one(neighbor_table[1][1][1], "P %d %d %d %d %d %d\n", wcoord.x, wcoord.y, wcoord.z, bcoord.x, bcoord.y, bcoord.z));

}


int c_quit(char *reason) {

    if (reason)
        return (sendto_one(neighbor_table[1][1][1], "QUIT :%s\n", reason));
    else
        return (sendto_one(neighbor_table[1][1][1], "QUIT\n"));

}

int c_pc(uint32_t color) {

    return (sendto_one(neighbor_table[1][1][1], "PC %d\n", color));

}

int c_pr(head_rot rot) {

    return (sendto_one(neighbor_table[1][1][1], "PR %d %d\n", rot.xrot, rot.yrot));

}

int c_ph(hand_hold hand) {

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

    return (sendto_one(neighbor_table[1][1][1], "BADD %d %d %d %d %d %d %d %d\n", wcoord.x, wcoord.y, wcoord.z, bcoord.x, bcoord.y, bcoord.z, block.id, block.bf));

}

int c_bset(w_coord wcoord, b_coord bcoord, block_t block) {

    return (sendto_one(neighbor_table[1][1][1], "BSET %d %d %d %d %d %d %d %d\n", wcoord.x, wcoord.y, wcoord.z, bcoord.x, bcoord.y, bcoord.z, block.id, block.bf));

}

int c_brem(w_coord wcoord, b_coord bcoord) {

    return (sendto_one(neighbor_table[1][1][1], "BREM %d %d %d %d %d %d\n", wcoord.x, wcoord.y, wcoord.z, bcoord.x, bcoord.y, bcoord.z));

}


int c_pmine(int mining) {

    return (sendto_one(neighbor_table[1][1][1], "PMINE %d\n", mining ? 1 : 0));

}
