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

#ifndef MACROS_H
#define MACROS_H

  #define my_min(x, y) ((x) < (y)) ? (x) : (y)
  #define my_max(x, y) ((x) > (y)) ? (x) : (y)

  #define sendq_popall(x) sendq_pop((x), (x->sendq_length))

  #define NOSLIDEOVER 0
  #define SLIDEOVER   1

#endif
