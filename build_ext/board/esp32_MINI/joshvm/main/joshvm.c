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
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "sdkconfig.h"
#include "audio_mem.h"
#include "recorder_engine.h"
#include "esp_audio.h"
#include "esp_log.h"
#include "board.h"
#include "app_wifi_service.h"
#include "sd_card_init.h"
#include "joshvm_esp32_media.h"
#include "joshvm_esp32_rec_engine.h"
#include "joshvm_esp32_player.h"
#include "joshvm.h"
#include "joshvm_esp32_timer.h"

//---variate
static const char *TAG              = "JOSHVM_Audio";
UBaseType_t pvCreatedTask_vadtask;
extern esp_audio_handle_t           player;
SemaphoreHandle_t xSemaphore_MegaBoard_init = NULL;
SemaphoreHandle_t s_mutex_recorder = NULL;
SemaphoreHandle_t s_mutex_player = NULL;


//---define
//---fun
extern void javanotify_simplespeech_event(int, int);
extern void JavaTask();
extern int esp32_record_voicefile(unsigned char* filename, int time);
extern int esp32_playback_voice(int index);
extern void esp32_device_control(int);
extern int esp32_read_voice_buffer(unsigned char*, int);
extern int esp32_playback_voice_url(const char *url);
extern void esp32_stop_playback(void);
extern void esp32_stop_record(void);
extern void JavaNativeTest();


int esp32_read_voice_buffer(unsigned char* buffer,	int length)
{
	return 0;
}

void joshvm_app_init(void)
{
    //esp_log_level_set("*", ESP_LOG_INFO);
    ESP_LOGI(TAG, "ADF version is %s", ADF_VER);

	xSemaphore_MegaBoard_init = xSemaphoreCreateMutex(); 
	s_mutex_recorder = xSemaphoreCreateMutex();  
	s_mutex_player = xSemaphoreCreateMutex(); 
#ifndef	CONFIG_JOSH_EVB_MEGA_ESP32_V1_0_BOARD
	esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);
    audio_board_sdcard_init(set);
#else    
    app_sdcard_init();
#endif
	app_wifi_service();
	joshvm_vad_timer();
	ESP_LOGW(TAG,"Before javatask,free heap size = %d",heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT));
	/*	
	printf("Main task Executing on core : %d\n",xPortGetCoreID());	
	vTaskGetRunTimeStats(buff);
	printf("JOSHVM_Audio,Run Time Stats:\nTask Name   Time	  Percent\n%s\n", buf);	
	vTaskList(buff);
	printf("JOSHVM_Audio,Task List:\nTask Name	 Status   Prio	  HWM	 Task Number\n%s\n", buf);
	*/
	vTaskDelay(500 / portTICK_PERIOD_MS);

	while (1) {		
		JavaTask(); 
		
		for (int i = 10; i >= 0; i--) {
	        printf("Restarting in %d seconds...\n", i);
	        vTaskDelay(10000 / portTICK_PERIOD_MS);
	    }
	    printf("Restarting now.\n");
	    fflush(stdout);
	    esp_restart();	  
	}
}

/**
 * JOSH VM interface
 **/
int esp32_playback_voice(int i) {
	//NOT implemented
	return 0;
}

int esp32_record_voicefile(unsigned char* filename, int time) {
	//NOT implemented
	return 0;
}

int esp32_playback_voice_url(const char *url)
{
	return 0;
}

void esp32_device_control(int command) {
}

void esp32_stop_playback() {
}

void esp32_stop_record() {
}
