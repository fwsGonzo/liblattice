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

extern int ncoord_is_equal(n_coord a, n_coord b);
extern n_coord wcoord_to_ncoord(w_coord wcoord);

extern int serv_can_connect_to_me(n_coord coord);
extern int serv_can_connect_to_serv(n_coord coord_a, n_coord coord_b);
extern int serv_in_range_of_serv(n_coord center, n_coord side);

extern void init_neighbor_table(void);
extern server_socket *find_neighbor(n_coord coord);
extern int add_neighbor(n_coord coord, server_socket *s);
extern int del_neighbor(n_coord coord);

extern server_socket *connect_server(n_coord coord, struct in_addr ip, port_t port);

