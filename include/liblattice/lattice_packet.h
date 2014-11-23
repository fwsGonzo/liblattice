/*
**
** $Id $
**
** Streamtella (streamtella) by remorse@paddylee.com; scalable public broadcast solutions
** Copyright (C) 2010 Ralph Covelli
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

#ifndef LATTICE_PACKET_H
#define LATTICE_PACKET_H

  #include "struct.h"

  extern void packet_ntoh(lt_packet_h *h);
  extern void packet_hton(lt_packet_h *h);

  // --------------------

  extern int get_int8(void **src, int8_t *dst, uint16_t *len, uint16_t *argc);
  extern int get_int16(void **src, int16_t *dst, uint16_t *len, uint16_t *argc);
  extern int get_int32(void **src, int32_t *dst, uint16_t *len, uint16_t *argc);
  extern int get_int64(void **src, int64_t *dst, uint16_t *len, uint16_t *argc);

  extern int get_uint8(void **src, uint8_t *dst, uint16_t *len, uint16_t *argc);
  extern int get_uint16(void **src, uint16_t *dst, uint16_t *len, uint16_t *argc);
  extern int get_uint32(void **src, uint32_t *dst, uint16_t *len, uint16_t *argc);
  extern int get_uint64(void **src, uint64_t *dst, uint16_t *len, uint16_t *argc);

  extern int get_float(void **src, float *dst, uint16_t *len, uint16_t *argc);
  extern int get_double(void **src, double *dst, uint16_t *len, uint16_t *argc);
  extern int get_string(void **src, char **dst, uint16_t *len, uint16_t *argc);

  extern int get_sector(void **src, block_t *dst, uint16_t *len, uint16_t *argc);
  extern int get_flatsector(void **src, flatdata_t *dst, uint16_t *len, uint16_t *argc);

  // --------------------

  extern int put_int8(void **dst, int8_t src, uint16_t *len, uint16_t *argc);
  extern int put_int16(void **dst, int16_t src, uint16_t *len, uint16_t *argc);
  extern int put_int32(void **dst, int32_t src, uint16_t *len, uint16_t *argc);
  extern int put_int64(void **dst, int64_t src, uint16_t *len, uint16_t *argc);

  extern int put_uint8(void **dst, uint8_t src, uint16_t *len, uint16_t *argc);
  extern int put_uint16(void **dst, uint16_t src, uint16_t *len, uint16_t *argc);
  extern int put_uint32(void **dst, uint32_t src, uint16_t *len, uint16_t *argc);
  extern int put_uint64(void **dst, uint64_t src, uint16_t *len, uint16_t *argc);

  extern int put_float(void **dst, float src, uint16_t *len, uint16_t *argc);
  extern int put_double(void **dst, double src, uint16_t *len, uint16_t *argc);
  extern int put_string(void **dst, const char *src, uint16_t *len, uint16_t *argc);

  extern int put_sector(void **dst, block_t *src, uint16_t *len, uint16_t *argc);
  extern int put_flatsector(void **dst, flatdata_t *src, uint16_t *len, uint16_t *argc);

#endif
