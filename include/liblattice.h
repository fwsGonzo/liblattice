extern int lattice_init(int in_sock, void (*callback)());
extern int lattice_select(struct timeval *ptimeout);
extern void lattice_process(void);

