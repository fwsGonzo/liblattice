extern int s_ping(struct server_socket *src, int argc, char **argv);
extern int s_p(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_quit(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_pc(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_pr(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_ph(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_chat(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);
extern int s_action(struct server_socket *src, uint32_t *pfrom, int argc, char **argv);

