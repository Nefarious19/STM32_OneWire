/*
 * one_wire.c
 *
 *  Created on: 10.12.2017
 *      Author: rafal

 */

#include "stm32f1xx_hal.h"
#include "one_wire.h"
#include "OneWire_cmd.h"
#include "STM32_PINS.h"

volatile uint32_t OneWire_delay_cnt;

ONE_WIRE_DATA rom;


void OneWire_DelayDecrement (void)
{
	if(OneWire_delay_cnt) OneWire_delay_cnt--;
}

void OneWire_Delay_us (uint32_t delay_us)
{
	OneWire_delay_cnt = delay_us/5;
	while(OneWire_delay_cnt);
}


void OneWire_Delay_ms(uint32_t delay_ms)
{
	while(delay_ms--)
	{
		OneWire_Delay_us(1000);
	}
}


static void OneWire_Send_bit (bool state)
{
	if(state)
	{
		ONE_WIRE_RESET;
		OneWire_Delay_us(5);
		ONE_WIRE_SET;
		OneWire_Delay_us(65);
	}
	else
	{
		ONE_WIRE_RESET;
		OneWire_Delay_us(65);
		ONE_WIRE_SET;
		OneWire_Delay_us(5);
	}
}

static inline bool OneWire_Read_bit (void)
{
	bool state = 0;
	ONE_WIRE_RESET;
	OneWire_Delay_us(5);
	ONE_WIRE_SET;
	OneWire_Delay_us(10);
	state = ( bool )(GPIOD->IDR & PD5);
	OneWire_Delay_us(40);
	return state;
}

void OneWire_putByte (uint8_t byte)
{
	for(uint8_t i = 0; i < 8; i++)
	{
		OneWire_Send_bit((bool)(byte&0x01));
		byte >>= 1;
	}
}

uint8_t OneWire_readByte(void)
{
	uint8_t data = 0;
	for(uint8_t i = 0; i < 8; i++)
	{
		if(OneWire_Read_bit()) data |= (1<<i);
	}

	return data;
}

void OneWire_INIT (void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;
	GPIOD->CRL = (GPIOD->CRL & 0xFF0FFFFF) | 0x00700000;
}

bool OneWire_RST (void)
{
	bool sensor_presence = 0;
	ONE_WIRE_RESET;
	OneWire_Delay_us(480);
	ONE_WIRE_SET;
	OneWire_Delay_us(5);
	for(uint8_t i = 0; i < 95; i++)
	{
		if(!(GPIOD->IDR & PD5)) sensor_presence = 1;
		OneWire_Delay_us(5);
	}

	return sensor_presence;
}

uint8_t OneWire_search_ROM (ONE_WIRE_DATA * data)
{
	uint8_t tmp = 0;
	uint8_t i = 0;
	static uint8_t more_sensors = 0;
	static uint8_t last_bit = 0;

	if(!OneWire_RST()) return 0;
	OneWire_putByte(SEARCH_ROM);

	more_sensors = 0;

	for(i = 0; i < 64; i++)
	{
		tmp |= OneWire_Read_bit();
		tmp <<= 1;
		tmp |= OneWire_Read_bit();

		switch (tmp)
		{
			case 0x00:
				more_sensors = 1;
				data->One_Wire_ROM[i/8] |= (1<<((i&8)));
				OneWire_Send_bit(last_bit);
				last_bit = ~last_bit;
			break;
			case 0x01:
				data->One_Wire_ROM[i/8] |= (1<<((i&8)));
				OneWire_Send_bit(0);
			break;
			case 0x02:
				data->One_Wire_ROM[i/8] |= (1<<((i&8)));
				OneWire_Send_bit(1);
			break;
		}

		tmp = 0;
	}
	return more_sensors;
}

uint8_t OneWire_read_ROM (ONE_WIRE_DATA * data)
{

	uint8_t i = 0;

	if(!OneWire_RST()) return 0;
	OneWire_putByte(READ_ROM);
	OneWire_Delay_us(1000);
	for(i = 0; i < 8; i++)
	{
		data->One_Wire_ROM[i] = OneWire_readByte();
	}

	return 1;
}
