extern int lattice_init(int in_sock, void (*callback)(lattice_message *mp));
extern int lattice_select(struct timeval *ptimeout);
extern void lattice_process(void);
extern int lattice_send(lattice_message *msg);
extern void lattice_flush();
extern int lattice_connect(char *ipstr, uint16_t port);
