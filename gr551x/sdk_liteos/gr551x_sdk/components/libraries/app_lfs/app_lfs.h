/**
 *****************************************************************************************
 *
 * @file app_lfs.h
 *
 * @brief APP Little File System Portable API
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
#ifndef __APP_LFS_H__
#define __APP_LFS_H__

#include "hal_flash.h"
#include "gr55xx_hal.h"
#include "gr55xx_sys.h"
#include "lfs.h"

/**
 * @defgroup APP_LFS_MAROC Defines
 * @{
 */
#ifndef APP_LFS_READ_SIZE
#define APP_LFS_READ_SIZE           16
#endif

#ifndef APP_LFS_PROG_SIZE
#define APP_LFS_PROG_SIZE           16
#endif

#ifndef APP_LFS_CACHE_SIZE
#define APP_LFS_CACHE_SIZE          16
#endif

#ifndef APP_LFS_BLOCK_SIZE
#define APP_LFS_BLOCK_SIZE          EXFLASH_SIZE_SECTOR_BYTES
#endif

#ifndef APP_LFS_BLOCK_CYCLES
#define APP_LFS_BLOCK_CYCLES        500
#endif

#ifndef APP_LFS_BLOCK_COUNT
#define APP_LFS_BLOCK_COUNT         10
#endif

#ifndef APP_LFS_LOOKAHEAD_SIZE
#define APP_LFS_LOOKAHEAD_SIZE      32
#endif

#ifndef APP_LFS_FLASH_BASE
#define APP_LFS_FLASH_BASE           EXFLASH_START_ADDR
#endif

#define APP_LFS_PATH_MAX             (LFS_NAME_MAX + 1)
/** @} */


/**
 * @defgroup APP_LFS_ENUM Enumerations
 * @{
 */
/**@brief App LFS Error Code. */
enum
{
    APP_LFS_ERR_OK          = 0,    /**< No error. */
    APP_LFS_ERR_IO          = -5,   /**< Error during device operation. */
    APP_LFS_ERR_CORRUPT     = -84,  /**< Corrupted. */
    APP_LFS_ERR_NOENT       = -2,   /**< No directory entry. */
    APP_LFS_ERR_EXIST       = -17,  /**< Entry already exists. */
    APP_LFS_ERR_NOTDIR      = -20,  /**< Entry is not a dir. */
    APP_LFS_ERR_ISDIR       = -21,  /**< Entry is a dir. */
    APP_LFS_ERR_NOTEMPTY    = -39,  /**< Dir is not empty. */
    APP_LFS_ERR_BADF        = -9,   /**< Bad file number. */
    APP_LFS_ERR_FBIG        = -27,  /**< File too large. */
    APP_LFS_ERR_INVAL       = -22,  /**< Invalid parameter. */
    APP_LFS_ERR_NOSPC       = -28,  /**< No space left on device. */
    APP_LFS_ERR_NOMEM       = -12,  /**< No more memory available. */
    APP_LFS_ERR_NOATTR      = -61,  /**< No data/attr available. */
    APP_LFS_ERR_NAMETOOLONG = -36,  /**< File name too long. */
};
/** @} */

/**
 * @defgroup APP_LFS_TYPEDEF Typedefs
 * @{
 */
/**@brief The app lfs file id. */
typedef lfs_file_t app_lfs_file_id_t;

/**@brief The app lfs directory id. */
typedef lfs_dir_t  app_lfs_dir_id_t;

/**@brief The app lfs file info. */
typedef struct lfs_info  app_lfs_info_t;

/**@brief The app lfs file info. */
typedef void (*app_lfs_traverse_cb_t)(char *name, uint32_t size);
/** @} */

/**
 * @defgroup APP_LFS_FUNCTION Functions
 * @{
 */
/**
 *****************************************************************************************
 * @brief Initialize APP Little File System instance.
 *
 * @return Result of initialization.
 *****************************************************************************************
 */
int app_lfs_init(void);

/**
 *****************************************************************************************
 * @brief Deinitialize APP Little File System instance.
 *
 * @return Result of deinitialization.
 *****************************************************************************************
 */
int app_lfs_deinit(void);

/**
 *****************************************************************************************
 * @brief Format APP Little File System instance.
 *
 * @return Result of format file system.
 *****************************************************************************************
 */
int app_lfs_format(void);

/**
 *****************************************************************************************
 * @brief Make a directory.
 *
 * @param[in] dir_path_str: Directory path string.
 *
 * @return Result of make.
 *****************************************************************************************
 */
int app_lfs_mkdir(const char *dir_path_str);

/**
 *****************************************************************************************
 * @brief Open a directory.
 *
 * @param[in,out] p_dir_id:     Pointer to directory id.
 * @param[in]     dir_path_str: File path string.
 *
 * @return Result of open.
 *****************************************************************************************
 */
int app_lfs_dir_open(app_lfs_dir_id_t *p_dir_id, const char *dir_path_str);

/**
 *****************************************************************************************
 * @brief Close a directory.
 *
 * @param[in] p_dir_id:     Pointer to directory id.
 *
 * @return Result of close.
 *****************************************************************************************
 */
int app_lfs_dir_close(app_lfs_dir_id_t *p_dir_id);

/**
 *****************************************************************************************
 * @brief Read a directory.
 *
 * @param[in,out] p_dir_id:   Pointer to directory id.
 * @param[in]     p_lfs_info: Pointer to read info.
 *
 * @return Result of read.
 *****************************************************************************************
 */
int app_lfs_dir_read(app_lfs_dir_id_t *p_dir_id, app_lfs_info_t *p_lfs_info);

/**
 *****************************************************************************************
 * @brief Open a lfs file (if no exist, creat).
 *
 * @param[in,out] p_file_id:     Pointer to file id.
 * @param[in]     file_path_str: File path string.
 * @param[in]     flags:  File flags.
 *
 * @return Result of open.
 *****************************************************************************************
 */
int app_lfs_file_open(app_lfs_file_id_t *p_file_id, const char *file_path_str, int flags);

/**
 *****************************************************************************************
 * @brief Read a lfs file.
 *
 * @param[in]     p_file_id: Pointer to file id.
 * @param[in,out] p_rd_buff: Pointer to read buffer.
 * @param[in]     size:      Size of read buffer.
 *
 * @return Read bytes.
 *****************************************************************************************
 */
uint32_t app_lfs_file_read(app_lfs_file_id_t *p_file_id, uint8_t *p_rd_buff, uint32_t size);

/**
 *****************************************************************************************
 * @brief Write or update a lfs file.
 *
 * @param[in] p_file_id: Pointer to file id.
 * @param[in] p_rd_buff: Pointer to write buffer.
 * @param[in] size:      Size of write buffer.
 *
 * @return Write bytes.
 *****************************************************************************************
 */
uint32_t app_lfs_file_write(app_lfs_file_id_t *p_file_id, uint8_t *p_rd_buff, uint32_t size);

/**
 *****************************************************************************************
 * @brief File sync.
 *
 * @param[in] p_file_id: Pointer to file id. 
 *
 * @return Result of sync.
 *****************************************************************************************
 */
int app_lfs_file_sync(app_lfs_file_id_t *p_file_id);

/**
 *****************************************************************************************
 * @brief Change file location.
 *
 * @param[in] p_file_id: Pointer to file id.
 * @param[in] off: File offset.
 * @param[in] whence: Position flag.
 *
 * @return Result of seek.
 *****************************************************************************************
 */
int app_lfs_file_seek(app_lfs_file_id_t *p_file_id, int offset, int whence);

/**
 *****************************************************************************************
 * @brief Find information about files or directories.
 *
 * @param[in] path: File path.
 * @param[in] p_lfs_info: File information. 
 *
 * @return Result of stat.
 *****************************************************************************************
 */
int app_lfs_file_stat(const char *path, app_lfs_info_t *p_lfs_info);

/**
 *****************************************************************************************
 * @brief Get file size.
 *
 * @param[in] p_file_id: Pointer to file id.
 *
 * @return File size.
 *****************************************************************************************
 */
 int app_lfs_file_size(app_lfs_file_id_t *p_file_id);

/**
 *****************************************************************************************
 * @brief Return the location of the file.
 *
 * @param[in] p_file_id: Pointer to file id.
 *
 * @return File pointer position.
 *****************************************************************************************
 */
int app_lfs_file_tell(app_lfs_file_id_t *p_file_id);

/**
 *****************************************************************************************
 * @brief Close a lfs file.
 *
 * @param[in] p_file_id: Pointer to file id.
 *
 * @return Result of close.
 *****************************************************************************************
 */
int app_lfs_file_close(app_lfs_file_id_t *p_file_id);

/**
 *****************************************************************************************
 * @brief Remove a lfs file.
 *
 * @param[in] file_path_str: File path string.
 *
 * @return Result of remove.
 *****************************************************************************************
 */
int app_lfs_file_remove(const char *file_path_str);

/**
 *****************************************************************************************
 * @brief Rename or move a file or directory
 *
 * @note If removing a directory, the directory must be empty.
 *
 * @param[in] oldpath: Old file or directory path string.
 * @param[in] newpath: New file or directory path string.
 *
 * @return Result of rename.
 *****************************************************************************************
 */
int app_lfs_rename(const char *oldpath, const char *newpath);

/**
 *****************************************************************************************
 * @brief Traverse through all file in one directory
 *
 * @param[in] p_dir_id:     Pointer to directory id.
 * @param[in] dir_path_str: Directory path string.
 * @param[in] traverse_cb:  Callback for traverse.
 *****************************************************************************************
 */
void app_lfs_file_traverse(app_lfs_dir_id_t *p_dir_id, const char *dir_path_str, app_lfs_traverse_cb_t traverse_cb);

/** @} */
#endif


