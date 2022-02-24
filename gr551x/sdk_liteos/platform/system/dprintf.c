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

#include <stdarg.h>
#include <stdio.h>
#include "securec.h"

#define BUFSIZE  256

int printf(char const  *fmt, ...)
{
    char buf[BUFSIZE] = { 0 };
    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf_s(buf, sizeof(buf), BUFSIZE - 1, fmt, ap);
    if ( len < 0 ) {
        return len;
    }

    va_end(ap);
    if (len > 0) {
        char const *s = buf;
        while (*s) {
            _putchar(*s++);
        }
    }
    return len;
}

/*
int sprintf(char *buf, const char *fmt, ...)
{
    va_list args;
    int val;

    va_start(args, fmt);
    val = vsprintf_s(buf, fmt, args);
    va_end(args);

    return val;
}
*/
