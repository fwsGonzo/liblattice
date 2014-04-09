#ifndef CLIENT_COMMANDS_H
#define CLIENT_COMMANDS_H


  #if defined(_WIN32) && !defined(__MINGW32__)

    #ifndef LIBLATTICE_API
      #ifdef LIBLATTICE_EXPORTS
        #define LIBLATTICE_API __declspec(dllexport)
      #else
        #define LIBLATTICE_API __declspec(dllimport)
      #endif
    #endif

  #else

    #define LIBLATTICE_API extern

  #endif


  LIBLATTICE_API int c_p(w_coord wcoord, b_coord bcoord);
  LIBLATTICE_API int c_p_empty(void);
  LIBLATTICE_API int c_quit(char *reason);
  LIBLATTICE_API int c_pc(uint32_t color);
  LIBLATTICE_API int c_pr(head_rot rot);
  LIBLATTICE_API int c_ph(hand_hold hand);
  LIBLATTICE_API int c_chat(char *chat_text);
  LIBLATTICE_API int c_action(char *action_text);
  LIBLATTICE_API int c_s(int32_t mid, int32_t sid);
  LIBLATTICE_API int c_sc(int32_t csid);
  LIBLATTICE_API int c_bo(w_coord wcoord, b_coord bcoord, int32_t id);
  LIBLATTICE_API int c_mo(w_coord wcoord, b_coord bcoord, int32_t id, int32_t count);
  LIBLATTICE_API int c_badd(w_coord wcoord, b_coord bcoord, block_t block);
  LIBLATTICE_API int c_bset(w_coord wcoord, b_coord bcoord, block_t block);
  LIBLATTICE_API int c_brem(w_coord wcoord, b_coord bcoord);
  LIBLATTICE_API int c_pmine(int mining);
  LIBLATTICE_API int c_schat(char *schat_text);
  LIBLATTICE_API int c_lusers(void);

#endif
