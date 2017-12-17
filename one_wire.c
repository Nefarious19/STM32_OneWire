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

typedef struct
{
	uint8_t last_discrepancy;
	uint8_t last_family_discrepancy;
	uint8_t last_device_flag;

}ONE_WIRE_SEARCH_STS;

volatile uint32_t OneWire_delay_cnt;

ONE_WIRE_DATA rom;
ONE_WIRE_SEARCH_STS OW_stats;



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

	static uint8_t function_call_number = 0;

	uint8_t tmp = 0;
	uint8_t i = 0;
	uint8_t last_zero = 0;
	uint8_t bit_value = 0;
	uint8_t bit_position = 0;
	uint8_t return_value = 0;

	if(function_call_number > NUMBER_OF_SENSORS)
	{
		return ON_WIRE_LAST_SENSOR_FOUND;
	}
	else
	{
		function_call_number++;
	}

	if(!OneWire_RST()) return_value = ONE_WIRE_SENSOR_NOT_FOUND;
	else
	{
		OneWire_putByte(SEARCH_ROM);

		for(i = 0; i < 64; i++)
		{
			bit_position = i;
			tmp = 0;

			if(OneWire_Read_bit()) tmp |= (1<<1);
			if(OneWire_Read_bit()) tmp |= (1<<0);

			switch (tmp)
			{
				case 0x00:

					if(bit_position == OW_stats.last_discrepancy) bit_value = 1;
					else if (bit_position > OW_stats.last_discrepancy) bit_value = 0;
					else
					{
						bit_value = (1<<(bit_position%8)) & data->One_Wire_ROM[bit_position/8];
					}

					if(bit_value == 0)
					{
						last_zero = bit_position;
						if(last_zero < 9) OW_stats.last_family_discrepancy = last_zero;
					}

					break;

				case 0x01:
					bit_value = 0;
					break;

				case 0x02:
					bit_value = 1;
					break;

				case 0x03:
					OW_stats = (ONE_WIRE_SEARCH_STS){ 0, 0, 0};
					return_value = ON_WIRE_SEARCH_ERROR;
					break;
			}
			data->One_Wire_ROM[bit_position/8] |= (bit_value<<(bit_position%8));
			OneWire_Send_bit(bit_value);
		}
	}

	OW_stats.last_discrepancy = last_zero;

	if( ( data->adres.CRC_VAL > 0) && ( data->adres.CRC_VAL < 255) )
	{
		return_value = ONE_WIRE_SENSORS_FOUND;
	}

	return return_value;
}

uint8_t OneWire_read_ROM (ONE_WIRE_DATA * data)
{

	uint8_t i = 0;

	if(!OneWire_RST()) return 0;
	OneWire_putByte(READ_ROM);
	for(i = 0; i < 8; i++)
	{
		data->One_Wire_ROM[i] = OneWire_readByte();
	}

	return 1;
}
