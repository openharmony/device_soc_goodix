/**
 ****************************************************************************************
 *
 * @file utility.c
 *
 * @brief utility Implementation.
 *
 ****************************************************************************************
 * @attention
  #####Copyright (c) 2019 GOODIX
  All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of GOODIX nor the names of its contributors may be used
    to endorse or promote products derived from this software without
    specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************************
 */

/*
* INCLUDE FILES
****************************************************************************************
*/
#include "utility.h"

/*
 * DEFINES
 *****************************************************************************************
 */

#define ITEM_0           0
#define ITEM_1           1
#define ITEM_2           2
#define ITEM_3           3
#define ITEM_4           4
#define ITEM_5           5
#define ITEM_6           6
#define ITEM_7           7

#define BIT_8            8
#define BIT_16           16
#define BIT_24           24
#define BIT_32           32
#define BIT_40           40
#define BIT_48           48
#define BIT_56           56

#define OFFSET_0         0
#define OFFSET_1         1
#define OFFSET_2         2
#define OFFSET_3         3
#define OFFSET_4         4

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void htole16(uint8_t *p_buf, uint16_t x)
{
    uint8_t *u8ptr;
    u8ptr = p_buf;
    u8ptr[ITEM_0] = (uint8_t) x;
    u8ptr[ITEM_1] = (uint8_t)(x >> BIT_8);
}

void htole32(uint8_t *p_buf, uint32_t x)
{
    uint8_t *u8ptr;
    u8ptr = p_buf;
    u8ptr[ITEM_0] = (uint8_t) x;
    u8ptr[ITEM_1] = (uint8_t)(x >> BIT_8);
    u8ptr[ITEM_2] = (uint8_t)(x >> BIT_16);
    u8ptr[ITEM_3] = (uint8_t)(x >> BIT_24);
}

void htole64(uint8_t *p_buf, uint64_t x)
{
    uint8_t *u8ptr;
    u8ptr = p_buf;
    u8ptr[ITEM_0] = (uint8_t) x;
    u8ptr[ITEM_1] = (uint8_t)(x >> BIT_8);
    u8ptr[ITEM_2] = (uint8_t)(x >> BIT_16);
    u8ptr[ITEM_3] = (uint8_t)(x >> BIT_24);
    u8ptr[ITEM_4] = (uint8_t)(x >> BIT_32);
    u8ptr[ITEM_5] = (uint8_t)(x >> BIT_40);
    u8ptr[ITEM_6] = (uint8_t)(x >> BIT_48);
    u8ptr[ITEM_7] = (uint8_t)(x >> BIT_56);
}

uint16_t le16toh(const uint8_t *p_buf)
{
    const uint8_t *u8ptr;
    uint16_t x;
    u8ptr = p_buf;
    x = u8ptr[ITEM_0];
    x |= (uint16_t) u8ptr[ITEM_1] << BIT_8;
    return x;
}

uint32_t le32toh(const uint8_t *p_buf)
{
    const uint8_t *u8ptr;
    uint32_t x;
    u8ptr = p_buf;
    x = u8ptr[ITEM_0];
    x |= (uint32_t) u8ptr[ITEM_1] << BIT_8;
    x |= (uint32_t) u8ptr[ITEM_2] << BIT_16;
    x |= (uint32_t) u8ptr[ITEM_3] << BIT_24;
    return x;
}

uint64_t le64toh(const uint8_t *p_buf)
{
    const uint8_t *u8ptr;
    uint64_t x;
    u8ptr = p_buf;
    x = u8ptr[ITEM_0];
    x |= (uint64_t) u8ptr[ITEM_1] << BIT_8;
    x |= (uint64_t) u8ptr[ITEM_2] << BIT_16;
    x |= (uint64_t) u8ptr[ITEM_3] << BIT_24;
    x |= (uint64_t) u8ptr[ITEM_4] << BIT_32;
    x |= (uint64_t) u8ptr[ITEM_5] << BIT_40;
    x |= (uint64_t) u8ptr[ITEM_6] << BIT_48;
    x |= (uint64_t) u8ptr[ITEM_7] << BIT_56;
    return x;
}

void htobe16(uint8_t *p_buf, uint16_t x)
{
    uint8_t *u8ptr;
    u8ptr = p_buf;
    u8ptr[ITEM_0] = (uint8_t)(x >> BIT_8);
    u8ptr[ITEM_1] = (uint8_t) x;
}

void htobe32(uint8_t *p_buf, uint32_t x)
{
    uint8_t *u8ptr;
    u8ptr = p_buf;
    u8ptr[ITEM_0] = (uint8_t)(x >> BIT_24);
    u8ptr[ITEM_1] = (uint8_t)(x >> BIT_16);
    u8ptr[ITEM_2] = (uint8_t)(x >> BIT_8);
    u8ptr[ITEM_3] = (uint8_t) x;
}

void htobe64(uint8_t *p_buf, uint64_t x)
{
    uint8_t *u8ptr;
    u8ptr = p_buf;
    u8ptr[ITEM_0] = (uint8_t)(x >> BIT_56);
    u8ptr[ITEM_1] = (uint8_t)(x >> BIT_48);
    u8ptr[ITEM_2] = (uint8_t)(x >> BIT_40);
    u8ptr[ITEM_3] = (uint8_t)(x >> BIT_32);
    u8ptr[ITEM_4] = (uint8_t)(x >> BIT_24);
    u8ptr[ITEM_5] = (uint8_t)(x >> BIT_16);
    u8ptr[ITEM_6] = (uint8_t)(x >> BIT_8);
    u8ptr[ITEM_7] = (uint8_t) x;
}

uint16_t be16toh(const uint8_t *p_buf)
{
    const uint8_t *u8ptr;
    uint16_t x;
    u8ptr = p_buf;
    x = (uint16_t) u8ptr[ITEM_0] << BIT_8;
    x |= u8ptr[ITEM_1];
    return x;
}

uint32_t be32toh(const uint8_t *p_buf)
{
    const uint8_t *u8ptr;
    uint32_t x;
    u8ptr = p_buf;
    x = (uint32_t) u8ptr[ITEM_0] << BIT_24;
    x |= (uint32_t) u8ptr[ITEM_1] << BIT_16;
    x |= (uint32_t) u8ptr[ITEM_2] << BIT_8;
    x |= u8ptr[ITEM_3];
    return x;
}

uint64_t be64toh(const uint8_t *p_buf)
{
    const uint8_t *u8ptr;
    uint64_t x;
    u8ptr = p_buf;
    x = (uint64_t) u8ptr[ITEM_0] << BIT_56;
    x |= (uint64_t) u8ptr[ITEM_1] << BIT_48;
    x |= (uint64_t) u8ptr[ITEM_2] << BIT_40;
    x |= (uint64_t) u8ptr[ITEM_3] << BIT_32;
    x |= (uint64_t) u8ptr[ITEM_4] << BIT_24;
    x |= (uint64_t) u8ptr[ITEM_5] << BIT_16;
    x |= (uint64_t) u8ptr[ITEM_6] << BIT_8;
    x |= u8ptr[ITEM_7];
    return x;
}

uint8_t get_u8_inc(const uint8_t **pp_buf)
{
    const uint8_t *u8ptr;
    uint8_t x;
    u8ptr = *pp_buf;
    x = u8ptr[ITEM_0];
    *pp_buf += OFFSET_1;
    return x;
}

uint16_t get_u16_inc(const uint8_t **pp_buf)
{
    const uint8_t *u8ptr;
    uint16_t x;
    u8ptr = *pp_buf;
    x = u8ptr[ITEM_0];
    x |= (uint16_t) u8ptr[ITEM_1] << BIT_8;
    *pp_buf += OFFSET_2;
    return x;
}

uint32_t get_u32_inc(const uint8_t **pp_buf)
{
    const uint8_t *u8ptr;
    uint32_t x;
    u8ptr = *pp_buf;
    x = u8ptr[ITEM_0];
    x |= (uint32_t) u8ptr[ITEM_1] << BIT_8;
    x |= (uint32_t) u8ptr[ITEM_2] << BIT_16;
    x |= (uint32_t) u8ptr[ITEM_3] << BIT_24;
    *pp_buf += OFFSET_4;
    return x;
}

void put_u8_inc(uint8_t **pp_buf, uint8_t x)
{
    uint8_t *u8ptr;
    u8ptr = *pp_buf;
    u8ptr[ITEM_0] = x;
    *pp_buf += OFFSET_1;
}

void put_u16_inc(uint8_t **pp_buf, uint16_t x)
{
    uint8_t *u8ptr;
    u8ptr = *pp_buf;
    u8ptr[ITEM_0] = (uint8_t) x;
    u8ptr[ITEM_1] = (uint8_t)(x >> BIT_8);
    *pp_buf += OFFSET_2;
}

void put_u32_inc(uint8_t **pp_buf, uint32_t x)
{
    uint8_t *u8ptr;
    u8ptr = *pp_buf;
    u8ptr[ITEM_0] = (uint8_t) x;
    u8ptr[ITEM_1] = (uint8_t)(x >> BIT_8);
    u8ptr[ITEM_2] = (uint8_t)(x >> BIT_16);
    u8ptr[ITEM_3] = (uint8_t)(x >> BIT_24);
    *pp_buf += OFFSET_4;
}

