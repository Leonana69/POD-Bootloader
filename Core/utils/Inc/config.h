/**
 *    ||        ____  _ __  ______
 * +------+    / __ )(_) /_/  ____/_________ ____  ____
 * | 0xBC |   / __ /  / __/ /    / ___/ __ `/_  / / _  \
 * +------+  / /_/ / / /_/ /___ / /  / /_/ / / /_/   __/
 *  ||  ||  /_____/_/\__/\____//_/   \__,_/ /___/ \___/
 *
 * CrazyLoader firmware
 *
 * Copyright (C) 2011-2013 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * config.h - CrazyLoader config include file
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#define BUFFER_PAGES    10
#define flashPages      1024
#define FLASH_START     16

#define PAGE_SIZE       1024

// #ifndef FLASH_BASE
//     #define FLASH_BASE      0x08000000
// #endif

// FIRMWARE_START 0x08004000
#define FIRMWARE_START (FLASH_BASE + (FLASH_START * PAGE_SIZE))

// same for stm32 series
#define SYSTEM_BASE     0x1FFF0000

#define uartMain        huart6
#define uartDebug       huart3

#define NRF_FC_PIN_PORT GPIOA
#define NRF_FC_PIN      GPIO_PIN_4
#define NRF_TX_PIN_PORT GPIOC
#define NRF_TX_PIN      GPIO_PIN_7

#endif /* __CONFIG_H__ */
