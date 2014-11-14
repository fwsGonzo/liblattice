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

#include <string.h>
#include <stdint.h>

#ifdef __linux__
#include <netinet/in.h>
#else
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#endif

#include "lattice_config.h"
#include "serversocket.h"
#include "struct.h"

#include "portable_endian.h"

void packet_ntoh(lt_packet_h *h) {

    if(!h) return;
    h->marker = le32toh(h->marker);
    h->fromuid = le32toh(h->fromuid);
    h->flags = le16toh(h->flags);
    h->payload_argc = le16toh(h->payload_argc);
    h->payload_type = le16toh(h->payload_type);
    h->payload_length = le16toh(h->payload_length);
    return;

}

void packet_hton(lt_packet_h *h) {

    if(!h) return;
    h->marker = htole32(h->marker);
    h->fromuid = htole32(h->fromuid);
    h->flags = htole16(h->flags);
    h->payload_argc = htole16(h->payload_argc);
    h->payload_type = htole16(h->payload_type);
    h->payload_length = htole16(h->payload_length);
    return;

}

double lattice_htond(double d) {
    int64_t i64;
    int64_t o64;
    i64 = (*(int64_t *)(&d));
    o64 = htole64(i64);
    return *((double *)(&o64));
}

double lattice_ntohd(double d) {
    int64_t i64;
    int64_t o64;
    i64 = (*(int64_t *)(&d));
    o64 = le64toh(i64);
    return *((double *)(&o64));
}

float lattice_htonf(float f) {
    int32_t i32;
    int32_t o32;
    i32 = (*(int32_t *)(&f));
    o32 = htole32(i32);
    return *((float *)(&o32));
}

float lattice_ntohf(float f) {
    int32_t i32;
    int32_t o32;
    i32 = (*(int32_t *)(&f));
    o32 = le32toh(i32);
    return *((float *)(&o32));
}

// src points to a movable pointer to the payload of the packet
// dst points to the var we are loading up
// len and argc decays as we read through the payload

// 0 on fail

int get_int8(void **src, int8_t *dst, uint16_t *len, uint16_t *argc) {

    if(!src || !*src || !dst || !len || !argc) return(0);
    if(*len < sizeof(int8_t)) return(0);
    if(!*argc) return(0);
    *dst = **(int8_t **)src;
    (*(int8_t **)src)++;
    *len -= sizeof(int8_t);
    (*argc)--;
    return(1);

}

int get_int16(void **src, int16_t *dst, uint16_t *len, uint16_t *argc) {

    if(!src || !*src || !dst || !len || !argc) return(0);
    if(*len < sizeof(int16_t)) return(0);
    if(!*argc) return(0);
    *dst = le16toh(**(int16_t **)src);
    (*(int16_t **)src)++;
    *len -= sizeof(int16_t);
    (*argc)--;
    return(1);

}

int get_int32(void **src, int32_t *dst, uint16_t *len, uint16_t *argc) {

    if(!src || !*src || !dst || !len || !argc) return(0);
    if(*len < sizeof(int32_t)) return(0);
    if(!*argc) return(0);
    *dst = le32toh(**(int32_t **)src);
    (*(int32_t **)src)++;
    *len -= sizeof(int32_t);
    (*argc)--;
    return(1);

}

int get_int64(void **src, int64_t *dst, uint16_t *len, uint16_t *argc) {

    if(!src || !*src || !dst || !len || !argc) return(0);
    if(*len < sizeof(int64_t)) return(0);
    if(!*argc) return(0);
    *dst = le64toh(**(int64_t **)src);
    (*(int64_t **)src)++;
    *len -= sizeof(int64_t);
    (*argc)--;
    return(1);

}

int get_uint8(void **src, uint8_t *dst, uint16_t *len, uint16_t *argc) {

    if(!src || !*src || !dst || !len || !argc) return(0);
    if(*len < sizeof(uint8_t)) return(0);
    if(!*argc) return(0);
    *dst = **(uint8_t **)src;
    (*(uint8_t **)src)++;
    *len -= sizeof(uint8_t);
    (*argc)--;
    return(1);

}

int get_uint16(void **src, uint16_t *dst, uint16_t *len, uint16_t *argc) {

    if(!src || !*src || !dst || !len || !argc) return(0);
    if(*len < sizeof(uint16_t)) return(0);
    if(!*argc) return(0);
    *dst = le16toh(**(uint16_t **)src);
    (*(uint16_t **)src)++;
    *len -= sizeof(uint16_t);
    (*argc)--;
    return(1);

}

int get_uint32(void **src, uint32_t *dst, uint16_t *len, uint16_t *argc) {

    if(!src || !*src || !dst || !len || !argc) return(0);
    if(*len < sizeof(uint32_t)) return(0);
    if(!*argc) return(0);
    *dst = le32toh(**(uint32_t **)src);
    (*(uint32_t **)src)++;
    *len -= sizeof(uint32_t);
    (*argc)--;
    return(1);

}

int get_uint64(void **src, uint64_t *dst, uint16_t *len, uint16_t *argc) {

    if(!src || !*src || !dst || !len || !argc) return(0);
    if(*len < sizeof(uint64_t)) return(0);
    if(!*argc) return(0);
    *dst = le32toh(**(uint64_t **)src);
    (*(uint64_t **)src)++;
    *len -= sizeof(uint64_t);
    (*argc)--;
    return(1);

}

int get_float(void **src, float *dst, uint16_t *len, uint16_t *argc) {

    if(!src || !*src || !dst || !len || !argc) return(0);
    if(*len < sizeof(float)) return(0);
    if(!*argc) return(0);
    *dst = lattice_ntohf(**(float **)src);
    (*(float **)src)++;
    *len -= sizeof(float);
    (*argc)--;
    return(1);

}

int get_double(void **src, double *dst, uint16_t *len, uint16_t *argc) {

    if(!src || !*src || !dst || !len || !argc) return(0);
    if(*len < sizeof(double)) return(0);
    if(!*argc) return(0);
    *dst = lattice_ntohd(**(double **)src);
    (*(double **)src)++;
    *len -= sizeof(double);
    (*argc)--;
    return(1);

}

int get_string(void **src, char **dst, uint16_t *len, uint16_t *argc) {

    int i;
    char *p;

    if(!src || !*src || !dst || !len || !argc) return(0);
    if (*len < 1) return(0);
    if(!*argc) return(0);
    p = memchr(*(char **)src, '\0', *len);
    if (!p) return(0);
    i = (p - (*(char **)src)) + 1;
    *(char **)dst = *(char **)src;
    *(char **)src += i;
    *len -= i;
    (*argc)--;
    return(1);

}

int get_sector(void **src, block_t **dst, uint16_t *len, uint16_t *argc) {

#if BYTE_ORDER != LITTLE_ENDIAN
    int i;
#endif

    if(!src || !*src || !dst || !len || !argc) return(0);
    if(*len < (sizeof(block_t) * BLOCKSDB_BLOCKS_COUNT)) return(0);
    if((*argc) < BLOCKSDB_BLOCKS_COUNT) return(0);

    memcpy(*dst, *src, (sizeof(block_t) * BLOCKSDB_BLOCKS_COUNT));

#if BYTE_ORDER != LITTLE_ENDIAN
    for(i = 0; i < BLOCKSDB_BLOCKS_COUNT; i++)
        (*dst)[i] = le16toh((*dst)[i]);
#endif

    (*(block_t **)src) += BLOCKSDB_BLOCKS_COUNT;

    *len -= (sizeof(block_t) * BLOCKSDB_BLOCKS_COUNT);
    *argc -= BLOCKSDB_BLOCKS_COUNT;
    return(1);

}


// dst points to a movable pointer to the payload of the packet
// src is the var we are pulling from
// len and argc increments as we read through the payload

// 0 on fail

int put_int8(void **dst, int8_t src, uint16_t *len, uint16_t *argc) {

    if(!dst || !*dst || !len || !argc) return(0);
    if((*len + sizeof(int8_t)) > PAYLOAD_MTU) return(0);
    ** (int8_t **)dst = src;
    (* (int8_t **)dst)++;
    *len += sizeof(int8_t);
    (*argc)++;
    return(1);

}

int put_int16(void **dst, int16_t src, uint16_t *len, uint16_t *argc) {

    if(!dst || !*dst || !len || !argc) return(0);
    if((*len + sizeof(int16_t)) > PAYLOAD_MTU) return(0);
    ** (int16_t **)dst = htole16(src);
    (* (int16_t **)dst)++;
    *len += sizeof(int16_t);
    (*argc)++;
    return(1);

}

int put_int32(void **dst, int32_t src, uint16_t *len, uint16_t *argc) {

    if(!dst || !*dst || !len || !argc) return(0);
    if((*len + sizeof(int32_t)) > PAYLOAD_MTU) return(0);
    ** (int32_t **)dst = htole32(src);
    (* (int32_t **)dst)++;
    *len += sizeof(int32_t);
    (*argc)++;
    return(1);

}

int put_int64(void **dst, int64_t src, uint16_t *len, uint16_t *argc) {

    if(!dst || !*dst || !len || !argc) return(0);
    if((*len + sizeof(int64_t)) > PAYLOAD_MTU) return(0);
    ** (int64_t **)dst = htole64(src);
    (* (int64_t **)dst)++;
    *len += sizeof(int64_t);
    (*argc)++;
    return(1);

}

int put_uint8(void **dst, uint8_t src, uint16_t *len, uint16_t *argc) {

    if(!dst || !*dst || !len || !argc) return(0);
    if((*len + sizeof(uint8_t)) > PAYLOAD_MTU) return(0);
    ** (uint8_t **)dst = src;
    (* (uint8_t **)dst)++;
    *len += sizeof(uint8_t);
    (*argc)++;
    return(1);

}

int put_uint16(void **dst, uint16_t src, uint16_t *len, uint16_t *argc) {

    if(!dst || !*dst || !len || !argc) return(0);
    if((*len + sizeof(uint16_t)) > PAYLOAD_MTU) return(0);
    ** (uint16_t **)dst = htole16(src);
    (* (uint16_t **)dst)++;
    *len += sizeof(uint16_t);
    (*argc)++;
    return(1);

}

int put_uint32(void **dst, uint32_t src, uint16_t *len, uint16_t *argc) {

    if(!dst || !*dst || !len || !argc) return(0);
    if((*len + sizeof(uint32_t)) > PAYLOAD_MTU) return(0);
    ** (uint32_t **)dst = htole32(src);
    (* (uint32_t **)dst)++;
    *len += sizeof(uint32_t);
    (*argc)++;
    return(1);

}

int put_uint64(void **dst, uint64_t src, uint16_t *len, uint16_t *argc) {

    if(!dst || !*dst || !len || !argc) return(0);
    if((*len + sizeof(uint64_t)) > PAYLOAD_MTU) return(0);
    ** (uint64_t **)dst = htole64(src);
    (* (uint64_t **)dst)++;
    *len += sizeof(uint64_t);
    (*argc)++;
    return(1);

}

int put_float(void **dst, float src, uint16_t *len, uint16_t *argc) {

    if(!dst || !*dst || !len || !argc) return(0);
    if((*len + sizeof(float)) > PAYLOAD_MTU) return(0);
    ** (float **)dst = lattice_htonf(src);
    (* (float **)dst)++;
    *len += sizeof(float);
    (*argc)++;
    return(1);

}

int put_double(void **dst, double src, uint16_t *len, uint16_t *argc) {

    if(!dst || !*dst || !len || !argc) return(0);
    if((*len + sizeof(double)) > PAYLOAD_MTU) return(0);
    ** (double **)dst = lattice_htond(src);
    (* (double **)dst)++;
    *len += sizeof(double);
    (*argc)++;
    return(1);

}

int put_string(void **dst, const char *src, uint16_t *len, uint16_t *argc) {

    int i;
    char *p;

    if(!dst || !*dst || !src || !len || !argc) return(0);
    if(*len >= PAYLOAD_MTU) return(0);
    p = memchr(src, '\0', PAYLOAD_MTU - *len);
    if (!p) return(0);
    i = (p - src) + 1;
    if ((*len + i) > PAYLOAD_MTU) return(0);
    memcpy(*(char **)dst, src, i);
    * (char **)dst += i;
    *len += i;
    (*argc)++;
    return(1);

}

int put_sector(void **dst, block_t *src, uint16_t *len, uint16_t *argc) {

#if BYTE_ORDER != LITTLE_ENDIAN
    int i;
#endif

    if(!dst || !*dst || !src || !len || !argc) return(0);
    if((*len + (sizeof(block_t) * BLOCKSDB_BLOCKS_COUNT)) > PAYLOAD_MTU) return(0);

    memcpy(*dst, src, (sizeof(block_t) * BLOCKSDB_BLOCKS_COUNT));

#if BYTE_ORDER != LITTLE_ENDIAN
    for(i = 0; i < BLOCKSDB_BLOCKS_COUNT; i++)
        (*dst)[i] = htole16((*dst)[i]);
#endif

    (*(block_t **)dst) += BLOCKSDB_BLOCKS_COUNT;

    *len += (sizeof(block_t) * BLOCKSDB_BLOCKS_COUNT);
    *argc += BLOCKSDB_BLOCKS_COUNT;
    return(1);

}
