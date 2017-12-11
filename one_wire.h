/*
 * one_wire.h
 *
 *  Created on: 10.12.2017
 *      Author: rafal
 */

#ifndef ONE_WIRE_ONE_WIRE_H_
#define ONE_WIRE_ONE_WIRE_H_

typedef uint8_t bool;


#define TRUE 1
#define FALSE 0

#define ONE  TRUE
#define ZERO FALSE



#define ONE_WIRE_PORT GPIOD
#define ONE_WIRE_PIN  PD5

#define ONE_WIRE_SET 	(ONE_WIRE_PORT->BSRR = ONE_WIRE_PIN)
#define ONE_WIRE_RESET (ONE_WIRE_PORT->BRR = ONE_WIRE_PIN)

extern volatile uint32_t OneWire_delay_cnt;


typedef struct
{
	uint8_t byte1;
	uint8_t byte2;
	uint8_t byte3;
	uint8_t byte4;
	uint8_t byte5;
	uint8_t byte6;
	uint8_t byte7;
	uint8_t byte8;

} ONEWIRE_ROM;


typedef union
{
	uint8_t One_Wire_ROM[8];
	ONEWIRE_ROM adres;

} ONE_WIRE_DATA;

void OneWire_DelayDecrement (void);
void OneWire_INIT (void);
bool OneWire_RST (void);
void OneWire_putByte (uint8_t byte);
uint8_t OneWire_readByte(void);
uint8_t OneWire_search_ROM (ONE_WIRE_DATA * data);
uint8_t OneWire_read_ROM (ONE_WIRE_DATA * data);

#endif /* ONE_WIRE_ONE_WIRE_H_ */
