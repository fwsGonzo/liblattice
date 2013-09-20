extern int c_p(w_coord wcoord, b_coord bcoord);
extern int c_quit(char *reason);
extern int c_pc(uint32_t color);
extern int c_pr(head_rot rot);
extern int c_ph(hand_hold hand);
extern int c_chat(char *string);
extern int c_action(char *string);
extern int c_s(int32_t mid, int32_t sid);
extern int c_sc(int32_t csid);
extern int c_bo(w_coord wcoord, b_coord bcoord, int32_t id);
extern int c_mo(w_coord wcoord, b_coord bcoord, int32_t id, int32_t count);
extern int c_badd(w_coord wcoord, b_coord bcoord, block_t block);
extern int c_bset(w_coord wcoord, b_coord bcoord, block_t block);
extern int c_brem(w_coord wcoord, b_coord bcoord);
extern int c_pmine(int mining);
extern int c_schat(char *string);
extern int c_lusers();

