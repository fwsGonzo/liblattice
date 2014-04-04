#ifndef LIBLATTICE_H
#define LIBLATTICE_H

  #include <string.h>
  #include <stdint.h>
  #include <stdio.h>

  #ifdef __linux__
    #include <stdlib.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
  #endif

  #if defined(_WIN32) && !defined(__MINGW32__)

    #ifndef LIBLATTICE_API
      #ifdef LIBLATTICE_EXPORTS
        #define LIBLATTICE_API __declspec(dllexport)
      #else
        #define LIBLATTICE_API __declspec(dllimport)
      #endif
    #endif

    #include "lattice_config.h"
    #include "serversocket.h"
    #include "struct.h"
    #include "globals.h"

    LIBLATTICE_API int lattice_init(int in_sock, void (*callback)(lattice_message *mp));
    LIBLATTICE_API int lattice_select(struct timeval *ptimeout);
    LIBLATTICE_API void lattice_process(void);
    LIBLATTICE_API int lattice_send(lattice_message *msg);
    LIBLATTICE_API void lattice_flush();
    LIBLATTICE_API int lattice_connect(char *ipstr, uint16_t port);

  #else

    #include <sys/time.h>


    #include "lattice_config.h"
    #include "serversocket.h"
    #include "struct.h"
    #include "globals.h"

    extern int lattice_init(int in_sock, void (*callback)(lattice_message *mp));
    extern int lattice_select(struct timeval *ptimeout);
    extern void lattice_process(void);
    extern int lattice_send(lattice_message *msg);
    extern void lattice_flush();
    extern int lattice_connect(char *ipstr, uint16_t port);

  #endif

  #include "client_commands.h"

#endif
