/*
 * Copyright (c) 2021 GOODIX.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "gr551x_spi_flash.h"
#include "app_log.h"
#include "gr55xx_dfu.h"
#include <stdint.h>
#include <stdbool.h>

#define FLASH_OP_PAGE_SIZE   (0x1000)
#define FLASH_SIZE_MAX       (0x000E1000)
#define IMG_INFO_DFU_ADDR     (0x01002000UL)

static uint8_t   is_flash_init = 0;
static uint32_t  flash_op_save_offset  = 0;
static uint32_t  flash_op_erase_offset = 0;

typedef struct
{
    uint16_t    pattern;
    uint16_t    version;
    boot_info_t boot_info;
    uint8_t     comments[12];
} fw_img_info_t;

bool HILINK_OtaAdapterFlashInit(void)
{
    APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---Entry!!! ", __FUNCTION__);

    flash_op_save_offset = 0;
    flash_op_erase_offset = 0;

    if (is_flash_init)
    {
        return true;
    }

    flash_init_t  flash_init;

    flash_init.spi_type = FLASH_QSPI_ID1;
    flash_init.flash_io.spi_cs.gpio           = APP_IO_TYPE_NORMAL;
    flash_init.flash_io.spi_cs.pin            = APP_IO_PIN_15;
    flash_init.flash_io.spi_cs.mux            = APP_IO_MUX_2;
    flash_init.flash_io.spi_clk.gpio          = APP_IO_TYPE_NORMAL;
    flash_init.flash_io.spi_clk.pin           = APP_IO_PIN_9;
    flash_init.flash_io.spi_clk.mux           = APP_IO_MUX_2;
    flash_init.flash_io.spi_io0.qspi_io0.gpio = APP_IO_TYPE_NORMAL;
    flash_init.flash_io.spi_io0.qspi_io0.pin  = APP_IO_PIN_8;
    flash_init.flash_io.spi_io0.qspi_io0.mux  = APP_IO_MUX_2;
    flash_init.flash_io.spi_io1.qspi_io1.gpio = APP_IO_TYPE_NORMAL;
    flash_init.flash_io.spi_io1.qspi_io1.pin  = APP_IO_PIN_14;
    flash_init.flash_io.spi_io1.qspi_io1.mux  = APP_IO_MUX_2;
    flash_init.flash_io.qspi_io2.gpio         = APP_IO_TYPE_NORMAL;
    flash_init.flash_io.qspi_io2.pin          = APP_IO_PIN_13;
    flash_init.flash_io.qspi_io2.mux          = APP_IO_MUX_2;
    flash_init.flash_io.qspi_io3.gpio         = APP_IO_TYPE_NORMAL;
    flash_init.flash_io.qspi_io3.pin          = APP_IO_PIN_12;
    flash_init.flash_io.qspi_io3.mux          = APP_IO_MUX_2;

    if (!spi_flash_init(&flash_init))
    {
        APP_LOG_ERROR(">>>>>>>>>>>>>>>>>>>>>>>>>%s fail !!! ", __FUNCTION__);
        return false;
    }

    if (!spi_flash_sector_erase(flash_op_erase_offset, FLASH_OP_PAGE_SIZE))
    {
        APP_LOG_ERROR(">>>>>>>>>>>>>>>>>>>>>>>>>>Erase Bank1 address: 0x%08x fail !!! ", flash_op_erase_offset);
        return false;
    }
    APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>Erase Bank1 address: 0x%08x ", flash_op_erase_offset);
    flash_op_erase_offset += FLASH_OP_PAGE_SIZE;

    is_flash_init = 1;
    return true;
}

unsigned int HILINK_OtaAdapterGetUpdateIndex(void)
{
    APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    return 0;
}


int HILINK_OtaAdapterFlashErase(unsigned int size)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---size:%d Entry!!! ", __FUNCTION__, size);
    if(spi_flash_sector_erase(0, size))
    {
        return 0;
    }

    APP_LOG_ERROR(">>>>>>>>>>>>>>>>>>>>>>>>>>%s fail !!! ", __FUNCTION__);
    return -1;
}

int HILINK_OtaAdapterFlashWrite(const unsigned char *buf, unsigned int buflen)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- buflen %d  !!! ", __FUNCTION__, buflen);

    if ((flash_op_save_offset + buflen) > flash_op_erase_offset)
    {
        if (!spi_flash_sector_erase(flash_op_erase_offset, FLASH_OP_PAGE_SIZE))
        {
            APP_LOG_ERROR(">>>>>>>>>>>>>>>>>>>>>>>>>>Erase Bank1 address: 0x%08x fail !!! ", flash_op_erase_offset);
            return false;
        }
        APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>Erase Bank1 address: 0x%08x ", flash_op_erase_offset);
        flash_op_erase_offset += FLASH_OP_PAGE_SIZE;
    }

    APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>Write Bank1 address: 0x%08x len: 0x%08x", flash_op_save_offset, buflen);
    if (buflen != spi_flash_write(flash_op_save_offset, (uint8_t *)buf, buflen))
    {
        APP_LOG_ERROR(">>>>>>>>>>>>>>>>>>>>>>>>>>Write Bank1 address: 0x%08x fail !!! ", flash_op_save_offset);
        return -1;
    }
    flash_op_save_offset += buflen;
    
    return 0;
}

int HILINK_OtaAdapterFlashRead(unsigned int offset, unsigned char *buf, unsigned int buflen)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---buflen:%d Entry!!! ", __FUNCTION__, buflen);
    if (buflen == spi_flash_read(offset, buf, buflen))
    {
        return 0;
    }

    APP_LOG_ERROR(">>>>>>>>>>>>>>>>>>>>>>>>>>%s error: read size is not buflen !!! ", __FUNCTION__);
    return -1;
}

bool HILINK_OtaAdapterFlashFinish(void)
{
    APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---Entry!!! ", __FUNCTION__);
    return true;
}

unsigned int HILINK_OtaAdapterFlashMaxSize(void)
{
    APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---0x%08x Entry!!! ", __FUNCTION__, FLASH_SIZE_MAX);
    return FLASH_SIZE_MAX;
}

void HILINK_OtaAdapterRestart(int flag)
{
    APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    (void)flag;
    fw_img_info_t img_info;

    spi_flash_read(flash_op_save_offset - 48, (uint8_t *)&img_info, sizeof(fw_img_info_t));
    hal_flash_write(IMG_INFO_DFU_ADDR, (uint8_t *)&img_info, sizeof(fw_img_info_t));
    
    hal_nvic_system_reset();
    return;
}

int HilinkGetMcuVersion(char *version,unsigned int inlen, unsigned int *outlen)
{
    return 0;
}

int HilinkGetRebootFlag(void)
{
    return 1;
}

int HilinkNotifyOtaData(const unsigned char *data, unsigned int len, unsigned int offset)
{
    return 0;
}

int HilinkOtaStartProcess(int type)
{
    return 0;
}
int HilinkOtaEndProcess(int status)
{
    return 0;
}


int HilinkNotifyOtaStatus(int flag, unsigned int len, unsigned int type)
{
    return 0;
}
