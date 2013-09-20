extern int s_ping(struct server_socket *src, int argc, char **argv);
extern int s_p(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_quit(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_pc(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_pr(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_ph(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_chat(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_action(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_s(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_sc(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_bo(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_mo(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_badd(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_bset(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_brem(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_pmine(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_schat(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);

