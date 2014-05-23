/*
**
** $Id $
**
** The Lattice (lattice) by remorse@paddylee.com; scalable MMO solutions
** Copyright (C) 2010-2013 Ralph Covelli
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
*/

#ifndef SEND_H
#define SEND_H

  #include "lattice_config.h"
  #include "serversocket.h"
  #include "struct.h"

  extern int flush_write(server_socket *s, int closing);
  extern void flushall_write(void);
  extern int sendq_add(server_socket *s, void *data, int length);
  extern int sendq_pop(server_socket *s, int length);
  extern int sendq_flush(server_socket *s);
  extern int writeto(server_socket *s, const void *buf, size_t count);

  extern int sendto_one(server_socket *entry, char *format, ...);
  extern int sendto_allservers(char *format, ...);
  extern int sendto_allservers_butone(server_socket *butme, char *format, ...);

  extern int sendpacket(server_socket *entry, lt_packet *packet);
  extern int sendpacketto_allservers(lt_packet *packet);
  extern int sendpacketto_allservers_butone(server_socket *butme, lt_packet *packet);

#endif
