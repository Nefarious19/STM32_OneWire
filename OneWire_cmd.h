/*
 * OneWire_cmd.h
 *
 *  Created on: 11.12.2017
 *      Author: rafal
 */

#ifndef ONE_WIRE_ONEWIRE_CMD_H_
#define ONE_WIRE_ONEWIRE_CMD_H_

 //ROM  COMMANDS
#define	 SEARCH_ROM   	0xF0
#define	 READ_ROM   	0x33
#define	 MATCH_ROM		0x55
#define	 SKIP_ROM		0xCC
#define	 ALARM_SEARCH	0xEC

//FUNCTION_COMMANDS
#define	CONVERT_T		0x44
#define	WRITE_SCRATCH	0x4E
#define	READ_SCRATCH	0xBE
#define	COPY_SCRATCH	0x48
#define	RECALL_E2		0xB8
#define	READ_PWR_SPL	0xB4


#endif /* ONE_WIRE_ONEWIRE_CMD_H_ */
