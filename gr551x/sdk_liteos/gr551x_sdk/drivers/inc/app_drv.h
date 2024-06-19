/*
 * Copyright (c) 2024 GOODIX.
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

#ifndef __APP_DRV_H__
#define __APP_DRV_H__

#include "app_drv_config.h"

#if FLASH_PROTECT_PRIORITY
#define protection_push() platform_interrupt_protection_push()
#define protection_pop()  platform_interrupt_protection_pop()
#else
#define protection_push()
#define protection_pop()
#endif

#endif
