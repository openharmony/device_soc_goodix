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

#include "errno.h"
#include "fcntl.h"
#include "unistd.h"
#include "hal_file.h"

int open(const char *path, int oflag, ...)
{
    return HalFileOpen(path, oflag, 0);
}

int close(int fd)
{
    return HalFileClose(fd);
}

ssize_t read(int fd, void *buf, size_t nbyte)
{
    return HalFileRead(fd, buf, nbyte);
}

ssize_t write(int fd, const void *buf, size_t nbyte)
{
    return HalFileWrite(fd, buf, nbyte);
}

int unlink(const char *path)
{
    return HalFileDelete(path);
}

off_t lseek(int fd, off_t offset, int whence)
{
    return HalFileSeek(fd, offset, whence);
}

