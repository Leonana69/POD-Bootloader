#ifndef __BOOTPIN_H__
#define __BOOTPIN_H__

#include <stdbool.h>
#include "crtp.h"

void bootpinInit(void);
void bootpinDeinit(void);
bool bootpinStartFirmware(void);
bool bootpinStartBootloader(void);
bool bootpinNrfReset(void);
void boot(void);
bool bootloaderProcess(CrtpPacket *pk);
#endif //__BOOTPIN_H__
