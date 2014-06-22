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
  #else
    #include <windows.h>
  #endif

  #if defined(_WIN32) && !defined(__MINGW32__)

    #ifndef LIBLATTICE_API
      #ifdef LIBLATTICE_EXPORTS
        #pragma comment(lib, "ws2_32.lib")
        #define LIBLATTICE_API __declspec(dllexport)
      #else
        #define LIBLATTICE_API __declspec(dllimport)
      #endif
    #endif

  #else

    #include <sys/time.h>

    #define LIBLATTICE_API extern

  #endif

  #include "lattice_config.h"
  #include "serversocket.h"
  #include "struct.h"
  #include "globals.h"

  LIBLATTICE_API  int lattice_init(int in_sock, void (*callback)(lattice_message *mp));
  LIBLATTICE_API  int lattice_select(struct timeval *ptimeout);
  LIBLATTICE_API void lattice_process(void);
  LIBLATTICE_API  int lattice_send(lattice_message *msg);
  LIBLATTICE_API void lattice_flush(void);
  LIBLATTICE_API  int lattice_connect(const char *ipstr, uint16_t port);
  LIBLATTICE_API void lattice_disconnect(void);
  LIBLATTICE_API  int lattice_setplayer(lattice_player_t *player);
  LIBLATTICE_API  int lattice_getplayer(lattice_player_t *player);

  LIBLATTICE_API  int authserver_login(const char *username, const char *password, const char *hostname, uint16_t port, uint16_t burstdist);
  LIBLATTICE_API  const char* authserver_errorstring(int error);

  #include "client_commands.h"

#endif
