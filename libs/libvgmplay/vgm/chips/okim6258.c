/**********************************************************************************************
 *
 *   OKI MSM6258 ADPCM
 *
 *   TODO:
 *   3-bit ADPCM support
 *   Recording?
 *
 **********************************************************************************************/


//#include "emu.h"
#include <stddef.h>	// for NULL
#include "mamedef.h"
#ifdef _DEBUG
#include <stdio.h>
#endif
//#include "streams.h"
#include <math.h>
#include "okim6258.h"

#define COMMAND_STOP		(1 << 0)
#define COMMAND_PLAY		(1 << 1)
#define	COMMAND_RECORD		(1 << 2)

#define STATUS_PLAYING		(1 << 1)
#define STATUS_RECORDING	(1 << 2)

static const int dividers[4] = { 1024, 768, 512, 512 };

#define QUEUE_SIZE	(1 << 1)
#define QUEUE_MASK	(QUEUE_SIZE - 1)
typedef struct _okim6258_state okim6258_state;
struct _okim6258_state
{
	UINT8  status;
    //TODO:  MODIZER changes start / YOYOFR
    UINT8 Muted;
    //TODO:  MODIZER changes end / YOYOFR

	UINT32 master_clock;	/* master clock frequency */
	UINT32 divider;			/* master clock divider */
	UINT8 adpcm_type;		/* 3/4 bit ADPCM select */
	UINT8 data_in;			/* ADPCM data-in register */
	UINT8 nibble_shift;		/* nibble select */
	//sound_stream *stream;	/* which stream are we playing on? */

	UINT8 output_bits;
	INT32 output_mask;

	// Valley Bell: Added a small queue to prevent race conditions.
	UINT8 data_buf[8];
	UINT8 data_in_last;
	UINT8 data_buf_pos;
	// Data Empty Values:
	//	00 - data written, but not read yet
	//	01 - read data, waiting for next write
	//	02 - tried to read, but had no data
	UINT8 data_empty;
	// Valley Bell: Added pan
	UINT8 pan;
	INT32 last_smpl;

	INT32 signal;
	INT32 step;
	
	UINT8 clock_buffer[0x04];
	UINT32 initial_clock;
	UINT8 initial_div;
	
	SRATE_CALLBACK SmpRateFunc;
	void* SmpRateData;
};

/* step size index shift table */
static const int index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

/* lookup table for the precomputed difference */
static int diff_lookup[49*16];

/* tables computed? */
static int tables_computed = 0;

#define MAX_CHIPS	0x02
static okim6258_state OKIM6258Data[MAX_CHIPS];
static UINT8 Iternal10Bit = 0x00;

/*INLINE okim6258_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == OKIM6258);
	return (okim6258_state *)downcast<legacy_device_base *>(device)->token();
}*/

/**********************************************************************************************

     compute_tables -- compute the difference tables

***********************************************************************************************/

static void compute_tables(void)
{
	/* nibble to bit map */
	static const int nbl2bit[16][4] =
	{
		{ 1, 0, 0, 0}, { 1, 0, 0, 1}, { 1, 0, 1, 0}, { 1, 0, 1, 1},
		{ 1, 1, 0, 0}, { 1, 1, 0, 1}, { 1, 1, 1, 0}, { 1, 1, 1, 1},
		{-1, 0, 0, 0}, {-1, 0, 0, 1}, {-1, 0, 1, 0}, {-1, 0, 1, 1},
		{-1, 1, 0, 0}, {-1, 1, 0, 1}, {-1, 1, 1, 0}, {-1, 1, 1, 1}
	};

	int step, nib;

	if (tables_computed)
		return;
	
	/* loop over all possible steps */
	for (step = 0; step <= 48; step++)
	{
		/* compute the step value */
		int stepval = floor(16.0 * pow(11.0 / 10.0, (double)step));

		/* loop over all nibbles and compute the difference */
		for (nib = 0; nib < 16; nib++)
		{
			diff_lookup[step*16 + nib] = nbl2bit[nib][0] *
				(stepval   * nbl2bit[nib][1] +
				 stepval/2 * nbl2bit[nib][2] +
				 stepval/4 * nbl2bit[nib][3] +
				 stepval/8);
		}
	}

	tables_computed = 1;
}


static INT16 clock_adpcm(okim6258_state *chip, UINT8 nibble)
{
	INT32 max = chip->output_mask - 1;
	INT32 min = -chip->output_mask;

	// original MAME algorithm (causes a DC offset over time)
	//chip->signal += diff_lookup[chip->step * 16 + (nibble & 15)];

	// awesome algorithm ported from XM6 - it works PERFECTLY
	int sample = diff_lookup[chip->step * 16 + (nibble & 15)];
	chip->signal = ((sample << 8) + (chip->signal * 245)) >> 8;

	/* clamp to the maximum */
	if (chip->signal > max)
		chip->signal = max;
	else if (chip->signal < min)
		chip->signal = min;

	/* adjust the step size and clamp */
	chip->step += index_shift[nibble & 7];
	if (chip->step > 48)
		chip->step = 48;
	else if (chip->step < 0)
		chip->step = 0;

	/* return the signal scaled up to 32767 */
	return chip->signal << 4;
}

//TODO:  MODIZER changes start / YOYOFR
#include "../../../../src/ModizerVoicesData.h"

void okim6258_set_mute_mask(UINT8 ChipID, UINT32 MuteMask)
{
    okim6258_state *chip = &OKIM6258Data[ChipID];
    UINT8 CurChn;
    
    chip->Muted = MuteMask & 0x01;
    
    return;
}
//TODO:  MODIZER changes end / YOYOFR

/**********************************************************************************************

     okim6258_update -- update the sound chip so that it is in sync with CPU execution

***********************************************************************************************/

//static STREAM_UPDATE( okim6258_update )
void okim6258_update(UINT8 ChipID, stream_sample_t **outputs, int samples)
{
	//okim6258_state *chip = (okim6258_state *)param;
	okim6258_state *chip = &OKIM6258Data[ChipID];
	//stream_sample_t *buffer = outputs[0];
	stream_sample_t *bufL = outputs[0];
	stream_sample_t *bufR = outputs[1];
    
    //TODO:  MODIZER changes start / YOYOFR
    //search first voice linked to current chip
    int m_voice_ofs=-1;
    int m_total_channels=1;
    for (int ii=0;ii<=SOUND_MAXVOICES_BUFFER_FX-m_total_channels;ii++) {
        if (((m_voice_ChipID[ii]&0x7F)==(m_voice_current_system&0x7F))&&(((m_voice_ChipID[ii]>>8)&0xFF)==m_voice_current_systemSub)) {
            m_voice_ofs=ii;
            break;
        }
    }
    int smplIncr=44100*1024/m_voice_current_samplerate+1;
    //printf("okim clock: %d\n",smplFreq);
    //TODO:  MODIZER changes end / YOYOFR
    

	//memset(outputs[0], 0, samples * sizeof(*outputs[0]));

	if (chip->status & STATUS_PLAYING)
	{
		int nibble_shift = chip->nibble_shift;

		while (samples)
		{
			/* Compute the new amplitude and update the current step */
			//int nibble = (chip->data_in >> nibble_shift) & 0xf;
			int nibble;
			INT16 sample;
			
			if (! nibble_shift)
			{
				// 1st nibble - get data
				if (! chip->data_empty)
				{
					chip->data_in = chip->data_buf[chip->data_buf_pos >> 4];
					chip->data_buf_pos += 0x10;
					chip->data_buf_pos &= 0x7F;
					if ((chip->data_buf_pos >> 4) == (chip->data_buf_pos & 0x0F))
						chip->data_empty ++;
				}
				else
				{
					//chip->data_in = chip->data_in_last;
					if (chip->data_empty < 0x80)
						chip->data_empty ++;
				}
			}
			nibble = (chip->data_in >> nibble_shift) & 0xf;

			/* Output to the buffer */
			//INT16 sample = clock_adpcm(chip, nibble);
			if (chip->data_empty < 0x02)
			{
				sample = clock_adpcm(chip, nibble);
				chip->last_smpl = sample;
			}
			else
			{
				// Valley Bell: data_empty behaviour (loosely) ported from XM6
				if (chip->data_empty >= 0x02 + 0x01)
				{
					chip->data_empty -= 0x01;
					/*if (chip->signal < 0)
						chip->signal ++;
					else if (chip->signal > 0)
						chip->signal --;*/
					chip->signal = chip->signal * 15 / 16;
					chip->last_smpl = chip->signal << 4;
				}
				sample = chip->last_smpl;
			}

			nibble_shift ^= 4;

			//*buffer++ = sample;
            if (chip->Muted) {
                *bufL++=0;
                *bufR++=0;
            } else {
                *bufL++ = (chip->pan & 0x02) ? 0x00 : sample;
                *bufR++ = (chip->pan & 0x01) ? 0x00 : sample;
            }
			samples--;
            
            //TODO:  MODIZER changes start / YOYOFR
            if (m_voice_ofs>=0) {
                int ofs_start=m_voice_current_ptr[m_voice_ofs+0];
                int ofs_end=(m_voice_current_ptr[m_voice_ofs+0]+smplIncr);
                
                if ((ofs_end>>10)>(ofs_start>>10))
                for (;;) {
                    
                    if ((!(chip->Muted))&&((chip->pan&3)!=3)) m_voice_buff[m_voice_ofs+0][(ofs_start>>10)&(SOUND_BUFFER_SIZE_SAMPLE-1)]=LIMIT8((sample>>8));
                    ofs_start+=1024;
                    if (ofs_start>=ofs_end) break;
                }
                while ((ofs_end>>10)>SOUND_BUFFER_SIZE_SAMPLE) ofs_end-=(SOUND_BUFFER_SIZE_SAMPLE<<10);
                m_voice_current_ptr[m_voice_ofs+0]=ofs_end;
            }
            //TODO:  MODIZER changes start / YOYOFR
		}

		/* Update the parameters */
		chip->nibble_shift = nibble_shift;
	}
	else
	{
		/* Fill with 0 */
		while (samples--)
		{
			//*buffer++ = 0;
			*bufL++ = 0;
			*bufR++ = 0;
            
            //TODO:  MODIZER changes start / YOYOFR
            if (m_voice_ofs>=0) {
                int ofs_end=(m_voice_current_ptr[m_voice_ofs+0]+smplIncr);
                
                while ((ofs_end>>10)>SOUND_BUFFER_SIZE_SAMPLE) ofs_end-=(SOUND_BUFFER_SIZE_SAMPLE<<10);
                m_voice_current_ptr[m_voice_ofs+0]=ofs_end;
            }
            //TODO:  MODIZER changes start / YOYOFR
		}
	}
}



/**********************************************************************************************

     state save support for MAME

***********************************************************************************************/

/*static void okim6258_state_save_register(okim6258_state *info, running_device *device)
{
	state_save_register_device_item(device, 0, info->status);
	state_save_register_device_item(device, 0, info->master_clock);
	state_save_register_device_item(device, 0, info->divider);
	state_save_register_device_item(device, 0, info->data_in);
	state_save_register_device_item(device, 0, info->nibble_shift);
	state_save_register_device_item(device, 0, info->signal);
	state_save_register_device_item(device, 0, info->step);
}*/


/**********************************************************************************************

     OKIM6258_start -- start emulation of an OKIM6258-compatible chip

***********************************************************************************************/

static int get_vclk(okim6258_state* info)
{
	int clk_rnd;
	
	clk_rnd = info->master_clock;
	clk_rnd += info->divider / 2;	 // for better rounding - should help some of the streams
	return clk_rnd / info->divider;
}

//static DEVICE_START( okim6258 )
int device_start_okim6258(UINT8 ChipID, int clock, int divider, int adpcm_type, int output_12bits)
{
	//const okim6258_interface *intf = (const okim6258_interface *)device->baseconfig().static_config();
	//okim6258_state *info = get_safe_token(device);
	okim6258_state *info;

	if (ChipID >= MAX_CHIPS)
		return 0;
	
	info = &OKIM6258Data[ChipID];
	
	compute_tables();

	//info->master_clock = device->clock();
	info->initial_clock = clock;
	info->initial_div = divider;
	info->master_clock = clock;
	info->adpcm_type = /*intf->*/adpcm_type;
	info->clock_buffer[0x00] = (clock & 0x000000FF) >>  0;
	info->clock_buffer[0x01] = (clock & 0x0000FF00) >>  8;
	info->clock_buffer[0x02] = (clock & 0x00FF0000) >> 16;
	info->clock_buffer[0x03] = (clock & 0xFF000000) >> 24;
	info->SmpRateFunc = NULL;

	/* D/A precision is 10-bits but 12-bit data can be output serially to an external DAC */
	info->output_bits = /*intf->*/output_12bits ? 12 : 10;
	if (Iternal10Bit)
		info->output_mask = (1 << (info->output_bits - 1));
	else
		info->output_mask = (1 << (12 - 1));
	info->divider = dividers[/*intf->*/divider];

	//info->stream = stream_create(device, 0, 1, device->clock()/info->divider, info, okim6258_update);

	info->signal = -2;
	info->step = 0;

	//okim6258_state_save_register(info, device);
	
	return get_vclk(info);
}


/**********************************************************************************************

     OKIM6258_stop -- stop emulation of an OKIM6258-compatible chip

***********************************************************************************************/

void device_stop_okim6258(UINT8 ChipID)
{
	okim6258_state *info = &OKIM6258Data[ChipID];
	
	return;
}

//static DEVICE_RESET( okim6258 )
void device_reset_okim6258(UINT8 ChipID)
{
	//okim6258_state *info = get_safe_token(device);
	okim6258_state *info = &OKIM6258Data[ChipID];
    
    //TODO:  MODIZER changes start / YOYOFR
    info->Muted=0;
    //TODO:  MODIZER changes end / YOYOFR

	//stream_update(info->stream);
	
	info->master_clock = info->initial_clock;
	info->clock_buffer[0x00] = (info->initial_clock & 0x000000FF) >>  0;
	info->clock_buffer[0x01] = (info->initial_clock & 0x0000FF00) >>  8;
	info->clock_buffer[0x02] = (info->initial_clock & 0x00FF0000) >> 16;
	info->clock_buffer[0x03] = (info->initial_clock & 0xFF000000) >> 24;
	info->divider = dividers[info->initial_div];
	if (info->SmpRateFunc != NULL)
		info->SmpRateFunc(info->SmpRateData, get_vclk(info));
	
	
	info->signal = -2;
	info->step = 0;
	info->status = 0;

	// Valley Bell: Added reset of the Data In register.
	info->data_in = 0x00;
	info->data_buf[0] = info->data_buf[1] = 0x00;
	info->data_buf_pos = 0x00;
	info->data_empty = 0xFF;
	info->pan = 0x00;
}


/**********************************************************************************************

     okim6258_set_divider -- set the master clock divider

***********************************************************************************************/

//void okim6258_set_divider(running_device *device, int val)
void okim6258_set_divider(UINT8 ChipID, int val)
{
	//okim6258_state *info = get_safe_token(device);
	okim6258_state *info = &OKIM6258Data[ChipID];
	int divider = dividers[val];

	info->divider = dividers[val];
	//stream_set_sample_rate(info->stream, info->master_clock / divider);
	if (info->SmpRateFunc != NULL)
		info->SmpRateFunc(info->SmpRateData, get_vclk(info));
}


/**********************************************************************************************

     okim6258_set_clock -- set the master clock

***********************************************************************************************/

//void okim6258_set_clock(running_device *device, int val)
void okim6258_set_clock(UINT8 ChipID, int val)
{
	//okim6258_state *info = get_safe_token(device);
	okim6258_state *info = &OKIM6258Data[ChipID];

	if (val)
	{
		info->master_clock = val;
	}
	else
	{
		info->master_clock =	(info->clock_buffer[0x00] <<  0) |
								(info->clock_buffer[0x01] <<  8) |
								(info->clock_buffer[0x02] << 16) |
								(info->clock_buffer[0x03] << 24);
	}
	//stream_set_sample_rate(info->stream, info->master_clock / info->divider);
	if (info->SmpRateFunc != NULL)
		info->SmpRateFunc(info->SmpRateData, get_vclk(info));
}


/**********************************************************************************************

     okim6258_get_vclk -- get the VCLK/sampling frequency

***********************************************************************************************/

//int okim6258_get_vclk(running_device *device)
int okim6258_get_vclk(UINT8 ChipID)
{
	//okim6258_state *info = get_safe_token(device);
	okim6258_state *info = &OKIM6258Data[ChipID];

	return get_vclk(info);
}


/**********************************************************************************************

     okim6258_status_r -- read the status port of an OKIM6258-compatible chip

***********************************************************************************************/

//READ8_DEVICE_HANDLER( okim6258_status_r )
/*UINT8 okim6258_status_r(UINT8 ChipID, offs_t offset)
{
	//okim6258_state *info = get_safe_token(device);
	okim6258_state *info = &OKIM6258Data[ChipID];

	//stream_update(info->stream);

	return (info->status & STATUS_PLAYING) ? 0x00 : 0x80;
}*/


/**********************************************************************************************

     okim6258_data_w -- write to the control port of an OKIM6258-compatible chip

***********************************************************************************************/
//WRITE8_DEVICE_HANDLER( okim6258_data_w )
static void okim6258_data_w(UINT8 ChipID, /*offs_t offset, */UINT8 data)
{
	//okim6258_state *info = get_safe_token(device);
	okim6258_state *info = &OKIM6258Data[ChipID];

	/* update the stream */
	//stream_update(info->stream);

	//info->data_in = data;
	//info->nibble_shift = 0;
	
	if (info->data_empty >= 0x02)
		info->data_buf_pos = 0x00;
	info->data_in_last = data;
	info->data_buf[info->data_buf_pos & 0x0F] = data;
	info->data_buf_pos += 0x01;
	info->data_buf_pos &= 0xF7;
	if ((info->data_buf_pos >> 4) == (info->data_buf_pos & 0x0F))
	{
		logerror("Warning: FIFO full!\n");
		info->data_buf_pos = (info->data_buf_pos & 0xF0) | ((info->data_buf_pos-1) & 0x07);
	}
	info->data_empty = 0x00;
}


/**********************************************************************************************

     okim6258_ctrl_w -- write to the control port of an OKIM6258-compatible chip

***********************************************************************************************/

//WRITE8_DEVICE_HANDLER( okim6258_ctrl_w )
static void okim6258_ctrl_w(UINT8 ChipID, /*offs_t offset, */UINT8 data)
{
	//okim6258_state *info = get_safe_token(device);
	okim6258_state *info = &OKIM6258Data[ChipID];

	//stream_update(info->stream);

	if (data & COMMAND_STOP)
	{
		info->status &= ~(STATUS_PLAYING | STATUS_RECORDING);
		return;
	}

	if (data & COMMAND_PLAY)
	{
		if (!(info->status & STATUS_PLAYING))
		{
			info->status |= STATUS_PLAYING;

			/* Also reset the ADPCM parameters */
			info->signal = -2;	// Note: XM6 lets this fade to 0 when nothing is going on
			info->step = 0;
			info->nibble_shift = 0;
			
			info->data_buf[0x00] = data;
			info->data_buf_pos = 0x01;	// write pos 01, read pos 00
			info->data_empty = 0x00;
		}
		info->step = 0;	// this line was verified with the source of XM6
		info->nibble_shift = 0;
	}
	else
	{
		info->status &= ~STATUS_PLAYING;
	}

	if (data & COMMAND_RECORD)
	{
		logerror("M6258: Record enabled\n");
		info->status |= STATUS_RECORDING;
	}
	else
	{
		info->status &= ~STATUS_RECORDING;
	}
}

static void okim6258_set_clock_byte(UINT8 ChipID, UINT8 Byte, UINT8 val)
{
	okim6258_state *info = &OKIM6258Data[ChipID];
	
	info->clock_buffer[Byte] = val;
	
	return;
}

static void okim6258_pan_w(UINT8 ChipID, UINT8 data)
{
	okim6258_state *info = &OKIM6258Data[ChipID];

	info->pan = data;
	
	return;
}


void okim6258_write(UINT8 ChipID, UINT8 Port, UINT8 Data)
{
	switch(Port)
	{
	case 0x00:
		okim6258_ctrl_w(ChipID, /*0x00, */Data);
		break;
	case 0x01:
		okim6258_data_w(ChipID, /*0x00, */Data);
		break;
	case 0x02:
		okim6258_pan_w(ChipID, Data);
		break;
	case 0x08:
	case 0x09:
	case 0x0A:
		okim6258_set_clock_byte(ChipID, Port & 0x03, Data);
		break;
	case 0x0B:
		okim6258_set_clock_byte(ChipID, Port & 0x03, Data);
		okim6258_set_clock(ChipID, 0);
		break;
	case 0x0C:
		okim6258_set_divider(ChipID, Data);
		break;
	}
	
	return;
}


void okim6258_set_options(UINT16 Options)
{
	Iternal10Bit = (Options >> 0) & 0x01;
	
	return;
}

void okim6258_set_srchg_cb(UINT8 ChipID, SRATE_CALLBACK CallbackFunc, void* DataPtr)
{
	okim6258_state *info = &OKIM6258Data[ChipID];
	
	// set Sample Rate Change Callback routine
	info->SmpRateFunc = CallbackFunc;
	info->SmpRateData = DataPtr;
	
	return;
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

/*DEVICE_GET_INFO( okim6258 )
{
	switch (state)
	{
		// --- the following bits of info are returned as 64-bit signed integers --- //
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(okim6258_state);			break;

		// --- the following bits of info are returned as pointers to data or functions --- //
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(okim6258);		break;
		case DEVINFO_FCT_STOP:							// nothing //								break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(okim6258);		break;

		// --- the following bits of info are returned as NULL-terminated strings --- //
		case DEVINFO_STR_NAME:							strcpy(info->s, "OKI6258");					break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "OKI ADPCM");				break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(OKIM6258, okim6258);*/
