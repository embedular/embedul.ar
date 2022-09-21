// license:GPL-2.0+
// copyright-holders:Jarek Burczynski,Tatsuyuki Satoh

#pragma once

#include <stdint.h>
#include <stdbool.h>


/* --- select emulation chips --- */
#define BUILD_YM3812 (1)
#define BUILD_YM3526 (0)
#define BUILD_Y8950  (0)


/* select output bits size of output : 8 or 16 */
#define OPL_SAMPLE_BITS 16

// sgermino: a way to implement a dynamic number of instances on static memory
// is to use a retro-ciaa MEMPOOL to pass a sufficiently sized buffer to
// Init/Write/Update etc. and a function like "FMOPL_YM3812_Size" to query
// the required buffer size. That might be my preferred implementation, but
// right now, it seems there's no use case for such a system.
#ifndef MAX_OPL_CHIPS
    #define MAX_OPL_CHIPS   1
#endif


#if (OPL_SAMPLE_BITS==16)
typedef int16_t OPLSAMPLE;
#endif
#if (OPL_SAMPLE_BITS==8)
typedef int8_t OPLSAMPLE;
#endif


typedef void (*OPL_TIMERHANDLER)(void *param,int timer,float period);
typedef void (*OPL_IRQHANDLER)(void *param,int irq);
typedef void (*OPL_UPDATEHANDLER)(void *param,int min_interval_us);
typedef void (*OPL_PORTHANDLER_W)(void *param,uint8_t data);
typedef uint8_t (*OPL_PORTHANDLER_R)(void *param);


#if BUILD_YM3812

bool FMOPL_YM3812_Init(uint8_t which, uint32_t clock, uint32_t rate);
void FMOPL_YM3812_Shutdown(uint8_t which);
void FMOPL_YM3812_ResetChip(uint8_t which);
int  FMOPL_YM3812_Write(uint8_t which, uint8_t a, uint8_t v);
int  FMOPL_YM3812_WriteReg(uint8_t which, uint8_t r, uint8_t v);
uint8_t FMOPL_YM3812_Read(uint8_t which, uint8_t a);
int  FMOPL_YM3812_TimerOver(uint8_t which, uint8_t c);
void FMOPL_YM3812_Update(uint8_t which, OPLSAMPLE *buffer, uint32_t length, bool stereo);

void FMOPL_YM3812_SetTimerHandler(uint8_t which, OPL_TIMERHANDLER TimerHandler, void *param);
void FMOPL_YM3812_SetIrqHandler(uint8_t which, OPL_IRQHANDLER IRQHandler, void *param);
void FMOPL_YM3812_SetUpdateHandler(uint8_t which, OPL_UPDATEHANDLER UpdateHandler, void *param);

#endif /* BUILD_YM3812 */


#if BUILD_YM3526

/*
** Initialize YM3526 emulator(s).
**
** 'num' is the number of virtual YM3526's to allocate
** 'clock' is the chip clock in Hz
** 'rate' is sampling rate
*/
void *ym3526_init(device_t *device, uint32_t clock, uint32_t rate);
/* shutdown the YM3526 emulators*/
void ym3526_shutdown(void *chip);
void ym3526_reset_chip(void *chip);
int  ym3526_write(void *chip, int a, int v);
uint8_t ym3526_read(void *chip, int a);
int  ym3526_timer_over(void *chip, int c);
/*
** Generate samples for one of the YM3526's
**
** 'which' is the virtual YM3526 number
** '*buffer' is the output buffer pointer
** 'length' is the number of samples that should be generated
*/
void ym3526_update_one(void *chip, OPLSAMPLE *buffer, int length);

void ym3526_set_timer_handler(void *chip, OPL_TIMERHANDLER TimerHandler, void *param);
void ym3526_set_irq_handler(void *chip, OPL_IRQHANDLER IRQHandler, void *param);
void ym3526_set_update_handler(void *chip, OPL_UPDATEHANDLER UpdateHandler, void *param);

#endif /* BUILD_YM3526 */


#if BUILD_Y8950

/* Y8950 port handlers */
void y8950_set_port_handler(void *chip, OPL_PORTHANDLER_W PortHandler_w, OPL_PORTHANDLER_R PortHandler_r, void *param);
void y8950_set_keyboard_handler(void *chip, OPL_PORTHANDLER_W KeyboardHandler_w, OPL_PORTHANDLER_R KeyboardHandler_r, void *param);
void y8950_set_delta_t_memory(void *chip, void * deltat_mem_ptr, int deltat_mem_size );

void * y8950_init(device_t *device, uint32_t clock, uint32_t rate);
void y8950_shutdown(void *chip);
void y8950_reset_chip(void *chip);
int  y8950_write(void *chip, int a, int v);
uint8_t y8950_read (void *chip, int a);
int  y8950_timer_over(void *chip, int c);
void y8950_update_one(void *chip, OPLSAMPLE *buffer, int length);

void y8950_set_timer_handler(void *chip, OPL_TIMERHANDLER TimerHandler, void *param);
void y8950_set_irq_handler(void *chip, OPL_IRQHANDLER IRQHandler, void *param);
void y8950_set_update_handler(void *chip, OPL_UPDATEHANDLER UpdateHandler, void *param);

#endif /* BUILD_Y8950 */
