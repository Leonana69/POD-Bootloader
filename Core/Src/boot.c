/* Pin scheme used by the NRF to direct bootloader boot:
 * State           | NRF_TX | NRF_FC |
 * ----------------+--------+--------+
 * Boot Firmware   |  HIGH  |  LOW   |
 * Boot Bootloader |  HIGH  |  HIGH  |
 * Error, boot DFU |  HZ    |  HZ    |
 *
 * NRF_TX and NRF_FC are configured with pull-down resistors
 * to detect the HZ state.
 */

#include "boot.h"
#include "gpio.h"
#include "config.h"
#include "loaderCommands.h"
#include <string.h>

#include "debug.h"

void bootpinInit(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void bootpinDeinit(void) {
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_7);
	__HAL_RCC_GPIOA_CLK_DISABLE();
	__HAL_RCC_GPIOC_CLK_DISABLE();
}

bool bootpinStartFirmware(void) {
	return HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET &&
					HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7) == GPIO_PIN_SET;
}

bool bootpinStartBootloader(void) {
	return HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET &&
					HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7) == GPIO_PIN_SET;
}

bool bootpinNrfReset(void) {
	return HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET &&
					HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7) == GPIO_PIN_RESET;
}

static const uint32_t sector_address[] = {
  [0]  = 0x08000000,
  [1]  = 0x08004000,
  [2]  = 0x08008000,
  [3]  = 0x0800C000,
  [4]  = 0x08010000,
  [5]  = 0x08020000,
  [6]  = 0x08040000,
  [7]  = 0x08060000,
  [8]  = 0x08080000,
  [9]  = 0x080A0000,
  [10] = 0x080C0000,
  [11] = 0x080E0000,
};

bool bootloaderProcess(CrtpPacket *pk) {
	static char buffer[BUFFER_PAGES * PAGE_SIZE];
  if ((pk->datalen > 1) && (pk->header == 0xFF) && (pk->data[0] == 0xFF)) {
    if (pk->data[1] == CMD_GET_INFO) {
      GetInfoReturns_t * info = (GetInfoReturns_t *) &pk->data[2];

      info->pageSize = PAGE_SIZE;
      info->nBuffPages = BUFFER_PAGES;
      info->nFlashPages = flashPages;
      info->flashStart = FLASH_START;
      info->version = PROTOCOL_VERSION;

      pk->datalen = 2 + sizeof(GetInfoReturns_t);

      return true;
    } else if (pk->data[1] == CMD_GET_MAPPING) {
      const uint8_t mapping[] = { 4, 16, 1, 64, 7, 128 };
      GetMappingReturns_t *returns = (GetMappingReturns_t *) &pk->data[2];

      memcpy(returns->mapping, mapping, sizeof(mapping));

      pk->datalen = 2 + sizeof(mapping);

      return true;
    }/*
    else if (pk->data[1] == CMD_SET_ADDRESS)
    {
      SetAddressParameters_t * addressPk;
      addressPk = (SetAddressParameters_t *)&pk->data[2];

      radioSetAddress(addressPk->address);
    }
    else */
    if (pk->data[1] == CMD_LOAD_BUFFER) {
      LoadBufferParameters_t *params = (LoadBufferParameters_t *) &pk->data[2];
      char *data = (char*) &pk->data[2 + sizeof(LoadBufferParameters_t)];

      // Fill the buffer with the given data
      for (int i = 0;
				i < (pk->datalen - (2 + sizeof(LoadBufferParameters_t))) &&
				(i + (params->page * PAGE_SIZE) + params->address) < (BUFFER_PAGES * PAGE_SIZE);
				i++)
        buffer[(i + (params->page * PAGE_SIZE) + params->address)] = data[i];
    } else if (pk->data[1] == CMD_READ_BUFFER) {
      ReadBufferParameters_t *params = (ReadBufferParameters_t *) &pk->data[2];
      char *data = (char*) &pk->data[2+sizeof(ReadBufferParameters_t)];

      // Return the data required
			int i;
      for (i = 0; i < 25 && (i + (params->page * PAGE_SIZE) + params->address) < (BUFFER_PAGES * PAGE_SIZE); i++)
        data[i] = buffer[(i + (params->page * PAGE_SIZE) + params->address)];

      pk->datalen += i;
      return true;
    } else if (pk->data[1] == CMD_READ_FLASH) {
      char *data = (char*) &pk->data[2 + sizeof(ReadFlashParameters_t)];
      ReadFlashParameters_t *params = (ReadFlashParameters_t *) &pk->data[2];
      char *flash= (char*) FLASH_BASE;

      // Return the data required
			int i;
      for (i = 0; i < 25 && (i + (params->page * PAGE_SIZE) + params->address) < (flashPages * PAGE_SIZE); i++)
        data[i] = flash[(i+(params->page*PAGE_SIZE)+params->address)];

      pk->datalen += i;
      return true;
    } else if (pk->data[1] == CMD_WRITE_FLASH) {
      unsigned int error = 0xFF;
      int flashAddress;
      uint32_t *bufferToFlash;
      WriteFlashParameters_t *params = (WriteFlashParameters_t *) &pk->data[2];
      WriteFlashReturns_t *returns = (WriteFlashReturns_t *) &pk->data[2];

      //Test if it is an acceptable write request
      if ((params->flashPage < FLASH_START) ||
					(params->flashPage >= flashPages) ||
          ((params->flashPage + params->nPages) > flashPages) ||
					(params->bufferPage >= BUFFER_PAGES)) {
        //Return a failure answer
        returns->done = 0;
        returns->error = 1;
        pk->datalen = 2 + sizeof(WriteFlashReturns_t);
        return true;
      } else {
        // Else, if everything is OK, flash the page(s)
        HAL_FLASH_Unlock();
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |FLASH_FLAG_WRPERR |
               FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

        __disable_irq();

        // Erase the page(s)
        for (int i = 0; i < params->nPages; i++) {
          for (int j = 0; j < 12; j++) {
            if ((uint32_t)(FLASH_BASE + ((uint32_t)params->flashPage * PAGE_SIZE) + (i * PAGE_SIZE)) == sector_address[j]) {
              FLASH_EraseInitTypeDef EraseInitStruct;
              EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
              EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
              EraseInitStruct.NbSectors = 1;
              // HAL_FLASHEx_Erase does (Sector << FLASH_CR_SNB_Pos)
              // which is (Sector << 3)
              EraseInitStruct.Sector = j;
              uint32_t SectorError = 0;
              // SPL: 
              // FLASH_EraseSector(j << 3, VoltageRange_3);
              // HAL:
              if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
                error = 2;
                goto failure;
              }
            }
          }
        }
				
        // Write the data, long per long
        flashAddress = FLASH_BASE + (params->flashPage * PAGE_SIZE);
        bufferToFlash = (uint32_t *)(&buffer[0] + (params->bufferPage * PAGE_SIZE));
        for (int i = 0; i < ((params->nPages * PAGE_SIZE) / sizeof(uint32_t)); i++, flashAddress += 4) {
          if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flashAddress, bufferToFlash[i]) != HAL_OK) {
            error = 3;
            goto failure;
          }
        }
        // Everything OK! great, send back an OK packet
        returns->done = 1;
        returns->error = 0;
        pk->datalen = 2 + sizeof(WriteFlashReturns_t);
        HAL_FLASH_Lock();
        __enable_irq();
        return true;

        failure:
        HAL_FLASH_Lock();
        __enable_irq();

        // If the write procedure failed, send the error packet
        // TODO: see if it is necessary or wanted to send the reason as well
        returns->done = 0;
        returns->error = error;
        pk->datalen = 2 + sizeof(WriteFlashReturns_t);
        return true;
      }
    }
  }
  return false;
}