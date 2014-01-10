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

#ifndef SOCKET_H
#define SOCKET_H

typedef int socket_error_t;

extern socket_error_t sock_getError();
extern int sock_ignoreError(socket_error_t error);
extern char* sock_errorString(socket_error_t error);

#endif
