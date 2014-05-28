#ifndef SERVER_COMMANDS_H
#define SERVER_COMMANDS_H

  extern int s_ping(struct server_socket *src, lt_packet *packet);
  extern int s_pong(struct server_socket *src, lt_packet *packet);
  extern int s_iamserver(struct server_socket *src, lt_packet *packet);
  extern int s_p(struct server_socket *src, lt_packet *packet);
  extern int s_quit(struct server_socket *src, lt_packet *packet);
  extern int s_pc(struct server_socket *src, lt_packet *packet);
  extern int s_pr(struct server_socket *src, lt_packet *packet);
  extern int s_ph(struct server_socket *src, lt_packet *packet);
  extern int s_chat(struct server_socket *src, lt_packet *packet);
  extern int s_action(struct server_socket *src, lt_packet *packet);
  extern int s_s(struct server_socket *src, lt_packet *packet);
  extern int s_sc(struct server_socket *src, lt_packet *packet);
  extern int s_bo(struct server_socket *src, lt_packet *packet);
  extern int s_mo(struct server_socket *src, lt_packet *packet);
  extern int s_badd(struct server_socket *src, lt_packet *packet);
  extern int s_bset(struct server_socket *src, lt_packet *packet);
  extern int s_brem(struct server_socket *src, lt_packet *packet);
  extern int s_pmine(struct server_socket *src, lt_packet *packet);
  extern int s_schat(struct server_socket *src, lt_packet *packet);
  extern int s_log(struct server_socket *src, lt_packet *packet);
  extern int s_satstep(struct server_socket *src, lt_packet *packet);
  extern int s_sat(struct server_socket *src, lt_packet *packet);
  extern int s_fade(struct server_socket *src, lt_packet *packet);
  extern int s_user(struct server_socket *src, lt_packet *packet);
  extern int s_server(struct server_socket *src, lt_packet *packet);
  extern int s_delserver(struct server_socket *src, lt_packet *packet);
  extern int s_moveto(struct server_socket *src, lt_packet *packet);
  extern int s_movefrom(struct server_socket *src, lt_packet *packet);
  extern int s_trackerfailure(struct server_socket *src, lt_packet *packet);
  extern int s_closing(struct server_socket *src, lt_packet *packet);

#endif
