/**
 *****************************************************************************************
 *
 * @file app_lfs.c
 *
 * @brief App Little File System.
 *
 *****************************************************************************************
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
 *****************************************************************************************
 */
#include "app_lfs.h"
#include "custom_config.h"
#include "app_log.h"


 /*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
int app_lfs_block_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int app_lfs_block_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int app_lfs_block_erase(const struct lfs_config *c, lfs_block_t block);
int app_lfs_device_sync(const struct lfs_config *c);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static uint32_t s_app_lfs_start_addr;
static lfs_t    s_app_lfs_instance;


static const struct lfs_config s_app_lfs_cfg = 
{
    .context        = NULL,
    .read           = app_lfs_block_read,
    .prog           = app_lfs_block_prog,
    .erase          = app_lfs_block_erase,
    .sync           = app_lfs_device_sync,
    .read_size      = APP_LFS_READ_SIZE,
    .prog_size      = APP_LFS_PROG_SIZE,
    .block_size     = APP_LFS_BLOCK_SIZE,
    .block_count    = APP_LFS_BLOCK_COUNT,
    .block_cycles   = APP_LFS_BLOCK_CYCLES,
    .cache_size     = APP_LFS_CACHE_SIZE,
    .lookahead_size = APP_LFS_LOOKAHEAD_SIZE,
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
static void s_lfs_start_addr_get(void)
{
    uint32_t flash_id;
    uint32_t flash_size;
    uint32_t nvds_start_addr;

    hal_flash_get_info(&flash_id, &flash_size);

    nvds_start_addr = APP_LFS_FLASH_BASE + flash_size - hal_flash_sector_size() * NVDS_NUM_SECTOR;
    s_app_lfs_start_addr  = nvds_start_addr - APP_LFS_BLOCK_COUNT * APP_LFS_BLOCK_SIZE;
    APP_LOG_INFO("LFS Flash start addr=0x%x, All size=%dKB", s_app_lfs_start_addr, APP_LFS_BLOCK_COUNT * APP_LFS_BLOCK_SIZE / 1024);
}

int app_lfs_block_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    uint32_t addr;

    if (!s_app_lfs_start_addr)
    {
        s_lfs_start_addr_get();
    }

    addr = s_app_lfs_start_addr + block * c->block_size + off;

    return size == hal_flash_read(addr, (uint8_t*)buffer, size) ? LFS_ERR_OK : LFS_ERR_IO;
}

int app_lfs_block_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    uint32_t addr;

    if (!s_app_lfs_start_addr)
    {
        s_lfs_start_addr_get();
    }

    addr = s_app_lfs_start_addr + block * c->block_size + off;

    return size == hal_flash_write(addr, (uint8_t*)buffer, size) ? LFS_ERR_OK : LFS_ERR_IO;
}

int app_lfs_block_erase(const struct lfs_config *c, lfs_block_t block)
{
    uint32_t addr;

    if (!s_app_lfs_start_addr)
    {
        s_lfs_start_addr_get();
    }

    addr = s_app_lfs_start_addr + block * c->block_size;

    hal_flash_erase (addr, c->block_size);
    return hal_flash_erase (addr, c->block_size) ? LFS_ERR_OK : LFS_ERR_IO;
}

int app_lfs_device_sync(const struct lfs_config *c)
{
    return LFS_ERR_OK;
}


/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
int app_lfs_init(void)
{
    int error_code = APP_LFS_ERR_OK;

    error_code = lfs_mount(&s_app_lfs_instance, &s_app_lfs_cfg);
    if (LFS_ERR_OK != error_code)
    {
        error_code = APP_LFS_ERR_OK;
        error_code |= lfs_format (&s_app_lfs_instance, &s_app_lfs_cfg);
        error_code |= lfs_mount(&s_app_lfs_instance, &s_app_lfs_cfg);
    }

    return error_code;
}

int app_lfs_deinit(void)
{
    return lfs_unmount(&s_app_lfs_instance);
}

int app_lfs_format(void)
{
    return lfs_format (&s_app_lfs_instance, &s_app_lfs_cfg);
}

int app_lfs_mkdir(const char *dir_path_str)
{
    int error_code;

    error_code = lfs_mkdir(&s_app_lfs_instance, dir_path_str);

    if (APP_LFS_ERR_EXIST == error_code)
    {
        error_code = APP_LFS_ERR_OK;
    }
    return error_code;
}

int app_lfs_dir_open(app_lfs_dir_id_t *p_dir_id, const char *dir_path_str)
{
    return lfs_dir_open(&s_app_lfs_instance, p_dir_id, dir_path_str);
}

int app_lfs_dir_close(app_lfs_dir_id_t *p_dir_id)
{
    return lfs_dir_close(&s_app_lfs_instance, p_dir_id);
}

int app_lfs_dir_rewind(app_lfs_dir_id_t *p_dir_id)
{
    return lfs_dir_rewind(&s_app_lfs_instance, p_dir_id);
}
int app_lfs_dir_read(app_lfs_dir_id_t *p_dir_id, app_lfs_info_t *p_lfs_info)
{
    return lfs_dir_read(&s_app_lfs_instance, p_dir_id, p_lfs_info);
}

int app_lfs_file_open(app_lfs_file_id_t *p_file_id, const char *file_path_str, int flags)
{
    return lfs_file_open(&s_app_lfs_instance,p_file_id, file_path_str, flags);   
}

uint32_t app_lfs_file_read(app_lfs_file_id_t *p_file_id, uint8_t *p_rd_buff, uint32_t size)
{
    int ret_code;

    if (NULL == p_file_id || NULL == p_rd_buff || 0 == size)
    {
        return 0;
    }

    ret_code = lfs_file_read(&s_app_lfs_instance, p_file_id, p_rd_buff, size);
    if (ret_code < LFS_ERR_OK)
    {
        return 0;
    }

    return ret_code;
}

uint32_t app_lfs_file_write(app_lfs_file_id_t *p_file_id, uint8_t *p_rd_buff, uint32_t size)
{
    int ret_code;

    if (NULL == p_file_id || NULL == p_rd_buff || 0 == size)
    {
        return 0;
    }

    ret_code = lfs_file_write(&s_app_lfs_instance, p_file_id, p_rd_buff, size);
    if (ret_code < LFS_ERR_OK)
    {
        return 0;
    }

    if ( LFS_ERR_OK == lfs_file_sync(&s_app_lfs_instance, p_file_id))
    {
        return ret_code;
    }

    return 0;
}

int app_lfs_file_sync(app_lfs_file_id_t *p_file_id)
{
    return lfs_file_sync(&s_app_lfs_instance, p_file_id);
}

int app_lfs_file_seek(app_lfs_file_id_t *p_file_id, int offset, int whence)
{
    return lfs_file_seek(&s_app_lfs_instance, p_file_id, offset, whence);
}

int app_lfs_file_stat(const char *path, app_lfs_info_t *p_lfs_info)
{
    return lfs_stat(&s_app_lfs_instance, path, p_lfs_info);
}

int app_lfs_file_size(app_lfs_file_id_t *p_file_id)
{
    return lfs_file_size(&s_app_lfs_instance, p_file_id);
}

int app_lfs_file_tell(app_lfs_file_id_t *p_file_id)
{
    return lfs_file_tell(&s_app_lfs_instance, p_file_id);
}

int app_lfs_file_close(app_lfs_file_id_t *p_file_id)
{
    return lfs_file_close(&s_app_lfs_instance, p_file_id);
}

int app_lfs_rename(const char *oldpath, const char *newpath)
{
    return lfs_rename(&s_app_lfs_instance, oldpath, newpath);
}

int app_lfs_file_remove(const char *file_path_str)
{
    return lfs_remove(&s_app_lfs_instance, file_path_str);
}

void app_lfs_file_traverse(app_lfs_dir_id_t *p_dir_id, const char *dir_path_str, app_lfs_traverse_cb_t traverse_cb)
{
    int              ret_err;
    struct lfs_info  lfs_info;
    app_lfs_dir_id_t dir_id;
    char             name[APP_LFS_PATH_MAX] = {0};

    if (NULL == dir_path_str || NULL == traverse_cb)
    {
        return;
    }

    if (0 != app_lfs_dir_open(p_dir_id, dir_path_str))
    {
        return;
    }

    while (1)
    {
        ret_err =  lfs_dir_read(&s_app_lfs_instance, p_dir_id, &lfs_info);

        if (ret_err < LFS_ERR_OK)
        {
            app_lfs_dir_close(p_dir_id);

            return;
        }
        else if (LFS_ERR_OK == ret_err)
        {
            app_lfs_dir_close(p_dir_id);

            return;
        }

        if (LFS_TYPE_DIR == lfs_info.type)
        {
            if (0 != memcmp(".", lfs_info.name, 1) && 0 != memcmp("..", lfs_info.name, 2))
            {
                uint8_t copy_len = strlen(dir_path_str);

                memcpy(name, dir_path_str, copy_len);
                name[copy_len] = '/';
                memcpy(&name[copy_len + 1], lfs_info.name, strlen(lfs_info.name));
                app_lfs_file_traverse(&dir_id, name, traverse_cb);
            }
        }
        else if (LFS_TYPE_REG == lfs_info.type)
        {
            traverse_cb(lfs_info.name, lfs_info.size);
        }
    }
}

