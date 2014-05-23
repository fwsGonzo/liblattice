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

  #define get_uid get_uint32
  #define put_uid put_uint32

  #define get_model get_uint16
  #define put_model put_uint16

  #define get_color get_uint32
  #define put_color put_uint32

  #define get_nickname get_string
  #define put_nickname put_string

  #define get_chat get_string
  #define put_chat put_string

  #define get_action get_string
  #define put_action put_string

  #define get_quitreason get_string
  #define put_quitreason put_string

  #define get_burstdist get_uint16
  #define put_burstdist put_uint16

  #define get_block get_uint16
  #define put_block put_uint16

  #define get_wx get_int32
  #define put_wx put_int32
  #define get_wy get_int32
  #define put_wy put_int32
  #define get_wz get_int32
  #define put_wz put_int32

  #define get_bx get_int32
  #define put_bx put_int32
  #define get_by get_int32
  #define put_by put_int32
  #define get_bz get_int32
  #define put_bz put_int32

  #define get_nx get_int32
  #define put_nx put_int32
  #define get_ny get_int32
  #define put_ny put_int32
  #define get_nz get_int32
  #define put_nz put_int32

  #define get_xrot get_int16
  #define get_yrot get_int16
  #define put_xrot put_int16
  #define put_yrot put_int16

  #define get_item_id get_uint16
  #define get_item_type get_uint16
  #define put_item_id put_uint16
  #define put_item_type put_uint16

  #define get_mining get_uint16
  #define put_mining put_uint16

  #define get_usercolor get_uint32
  #define put_usercolor put_uint32

  #define get_satstep get_uint32
  #define put_satstep put_uint32

  #define get_sat get_double
  #define put_sat put_double

  #define get_mid get_int32
  #define put_mid put_int32

  #define get_sid get_int32
  #define put_sid put_int32

  #define get_csid get_int32
  #define put_csid put_int32

  #define get_boid get_int32
  #define put_boid put_int32

  #define get_moid get_int32
  #define put_moid put_int32

  #define get_mocount get_int32
  #define put_mocount put_int32


#endif
