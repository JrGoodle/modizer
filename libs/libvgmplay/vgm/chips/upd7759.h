#pragma once

//#include "devlegcy.h"

/* There are two modes for the uPD7759, selected through the !MD pin.
   This is the mode select input.  High is stand alone, low is slave.
   We're making the assumption that nobody switches modes through
   software. */

#define UPD7759_STANDARD_CLOCK		640000

typedef struct _upd7759_interface upd7759_interface;
struct _upd7759_interface
{
	//void (*drqcallback)(running_device *device, int param);	/* drq callback (per chip, slave mode only) */
	void (*drqcallback)(int param);	/* drq callback (per chip, slave mode only) */
};

//TODO:  MODIZER changes start / YOYOFR
void upd7759_set_mute_mask(UINT8 ChipID, UINT32 MuteMask);
//TODO:  MODIZER changes end / YOYOFR


void upd7759_update(UINT8 ChipID, stream_sample_t **outputs, int samples);
void device_reset_upd7759(UINT8 ChipID);
int device_start_upd7759(UINT8 ChipID, int clock);
void device_stop_upd7759(UINT8 ChipID);

//void upd7759_set_bank_base(running_device *device, offs_t base);

//void upd7759_reset_w(running_device *device, UINT8 data);
//void upd7759_start_w(running_device *device, UINT8 data);
//int upd7759_busy_r(running_device *device);
//WRITE8_DEVICE_HANDLER( upd7759_port_w );

void upd7759_set_bank_base(UINT8 ChipID, offs_t base);

void upd7759_reset_w(UINT8 ChipID, UINT8 data);
void upd7759_start_w(UINT8 ChipID, UINT8 data);
int upd7759_busy_r(UINT8 ChipID);
void upd7759_port_w(UINT8 ChipID, offs_t offset, UINT8 data);

void upd7759_write(UINT8 ChipID, UINT8 Port, UINT8 Data);
void upd7759_write_rom(UINT8 ChipID, offs_t ROMSize, offs_t DataStart, offs_t DataLength,
					   const UINT8* ROMData);

//DECLARE_LEGACY_SOUND_DEVICE(UPD7759, upd7759);
