/*

  VLSI Solution generic microcontroller example player / recorder definitions.
  v1.00.

  See VS10xx AppNote: Playback and Recording for details.

  v1.00 2012-11-23 HH  First release

*/
#ifndef PLAYER_RECORDER_H
#define PLAYER_RECORDER_H

#include "vs10xx_uc.h"

#define perform_hardware_reset_VS1053() GPIO_WriteBit(GPIOD, GPIO_Pin_8, 0)
#define stop_hardware_reset_VS1053() 	GPIO_WriteBit(GPIOD, GPIO_Pin_8, 1)
#define select_VS1053_SCI()				GPIO_WriteBit(GPIOD, GPIO_Pin_11, 0)
#define deselect_VS1053_SCI()			GPIO_WriteBit(GPIOD, GPIO_Pin_11, 1)
#define select_VS1053_SDI()				GPIO_WriteBit(GPIOD, GPIO_Pin_10, 0)
#define deselect_VS1053_SDI()			GPIO_WriteBit(GPIOD, GPIO_Pin_10, 1)

void GPIOVS1053_Init();
int VSTestInitHardware(void);
int VSTestInitSoftware(void);
int VSTestHandleFile(char *fileName, int record);

void WriteSci(u_int8 addr, u_int16 data);
u_int16 ReadSci(u_int8 addr);
int WriteSdi(const u_int8 *data, u_int8 bytes);
void SaveUIState(void);
void RestoreUIState(void);
int GetUICommand(void);

#endif
