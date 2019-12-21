/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef _JOSHVM_APP_H_
#define _JOSHVM_APP_H_
#include "recorder_engine.h"

typedef enum 
{	
	QUEUE_WAKEUP_START,
	QUEUE_VAD_START,
	QUEUE_VAD_STOP,
	QUEUE_WAKEUP_END,
}rec_ecb_event_type_t;

#define JOSHVM_CYCLEBUF_BUFFER_SIZE (REC_ONE_BLOCK_SIZE * 60)
#define VAD_TASK_PRI					4
#define ESP_AUDIO_STATE_TASK_PRI		2
#define JOSHVM_AUDIO_STATE_TASK_PRI		2
#define	JOSHVM_AUDIO_TRACK_TASK_PRI		5
#define	JOSHVM_AUDIO_RECORDER_TASK_PRI	3

void joshvm_app_init(void);


/**
 * @brief 
 *
 * @note 
 *
 * @param 
 *
 * @return
 *     - >0 , number of bytes in returned buffer
 *     - =0, no data
 *     - -1, Reach the end of voice data (no bytes in buffer and VAD_STOP has been detected)
 *     - other, Read error
 */
int esp32_read_voice_buffer(unsigned char* buffer,  int length);



#endif
