#ifndef _BOARD_SMBUS_H
#define _BOARD_SMBUS_H

#include <ec/smbus.h>

void smbus_init(void);
int smbus_read(uint8_t address, uint8_t command, uint16_t * data);
int smbus_write(uint8_t address, uint8_t command, uint16_t data);
int smbus_read_bytes(uint8_t address, uint8_t command, char data[], uint8_t bytes);
int smbus_write_bytes(uint8_t address, uint8_t command, char data[], uint8_t bytes);

#endif // _BOARD_SMBUS_H
