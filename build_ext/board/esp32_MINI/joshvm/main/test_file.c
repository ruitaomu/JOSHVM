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


#include "joshvm_esp32_rec_engine.h"
#include "joshvm_esp32_media.h"
#include "joshvm_esp32_record.h"
#include "joshvm_esp32_ring_buff.h"
#include "joshvm_esp32_player.h"
#include "joshvm.h"

#define test_url "http://mirror-advertising.oss-cn-beijing.aliyuncs.com/20191104/c9817b80-aa09-4237-bbe6-7a61d038823c.mp3"
#define MP3_URI "/userdata/ding.mp3"
#define MP3_URI2 "/sdcard/16000.mp3"
#define TAG "TEST_FILE"

//------------------test--start-----------------
joshvm_media_t *handle_media_rec_test = NULL;
joshvm_media_t *handle_media_player_test = NULL;
joshvm_media_t *handle_track_test = NULL;
joshvm_media_t *handle_recorder_test = NULL;
void test_esp32_media(void);
static int callback_temp = 1;

#define MP3_URI_TEST "file://sdcard/48000.wav"

static void media_player_callback_test(void*handle,int para)
{
	callback_temp = 0;
	ESP_LOGI(TAG,"media_player_callback");
}


//------------------test--end-----------------


static void *handle_media_rec= NULL;
static void *handle_media_player = NULL;
//static void *handle_track = NULL;
//static void *handle_audio_rec = NULL;
static void *handle_vad_rec = NULL;

extern int esp32_playback_voice(int index);

static void player_callback(void* para, int para2)
{
	printf("finish play  index = %d\n",para2);
	ESP_LOGE("TEST>>>>>>>>","player start heap_caps_get_free_size = %d",heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT));
}

static void vad_callback(int index)
{
	printf("***vad*** callback  %s\n",(index == 0 ? "start":"stop"));

	#if 0
	if(index){
	/*	printf("\n");
		heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
		printf("\n");
		heap_caps_print_heap_info(MALLOC_CAP_8BIT);
		printf("\n");
		heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
		printf("\n");
		heap_caps_print_heap_info(MALLOC_CAP_SPIRAM);
		printf("\n");*/
		joshvm_esp32_vad_stop();
		//joshvm_esp32_media_close(handle_vad_rec);
		vTaskDelay(3000);
		joshvm_esp32_media_set_source(handle_media_player,MP3_URI2);
		joshvm_esp32_media_start(handle_media_player,player_callback);
	}
	*/
	#endif
}


static void wake_callback(int index)
{
	printf("wakeup callback  index = %d\n",index);
	//esp32_playback_voice(4);
	//joshvm_esp32_media_create(4,&handle_vad_rec);
	joshvm_esp32_vad_start(vad_callback);	
}

void fun_test()
{	
	joshvm_esp32_wakeup_enable(wake_callback);
	joshvm_esp32_media_create(4,&handle_vad_rec);
	//joshvm_esp32_vad_start(vad_callback);
		
	joshvm_esp32_media_create(0,&handle_media_player);
	joshvm_esp32_media_set_source(handle_media_player,MP3_URI2);
	joshvm_esp32_media_start(handle_media_player,player_callback);

	vTaskDelay(5000);
	//joshvm_esp32_media_stop(handle_media_player);

	//extern joshvm_err_t joshvm_esp32_i2s_deinit(void);
	//joshvm_esp32_i2s_deinit();

	
	while(1){


	

		vTaskDelay(1000);
	}	
}


void player_test()
{
	joshvm_esp32_media_create(0,&handle_media_player_test);

	while(1){
		//joshvm_esp32_media_set_source(handle_media_player_test,"/userdata/ding.mp3");
		//joshvm_esp32_media_start(handle_media_player_test,media_player_callback_test);
		//while(callback_temp);


		//joshvm_esp32_media_set_source(handle_media_player_test,"/userdata/ding.mp3");
		joshvm_esp32_media_start(handle_media_player_test,media_player_callback_test);
		//while(callback_temp);
		
		
		vTaskDelay(3000 / portTICK_PERIOD_MS);
		//joshvm_esp32_media_stop(handle_media_player_test);
		//vTaskDelay(1000 / portTICK_PERIOD_MS);
		joshvm_esp32_media_stop(handle_media_player_test);
	}


}

void test_esp32_media(void)
{

//---wakeup test
	//joshvm_esp32_wakeup_enable(media_player_callback_test);

	//vTaskDelay(20000 / portTICK_PERIOD_MS); 

	
	//joshvm_esp32_wakeup_enable(media_player_callback_test);



	


	//joshvm_esp32_media_create(0,&handle_media_player_test);
	//joshvm_esp32_media_create(1,&handle_media_rec_test);
	joshvm_esp32_media_create(2,&handle_track_test);
	joshvm_esp32_media_create(3,&handle_recorder_test);
	//joshvm_esp32_vad_start(test_vad_callback);


	//audio_track  audio_recorder

	joshvm_esp32_media_start(handle_recorder_test,media_player_callback_test);
	vTaskDelay(10000 / portTICK_PERIOD_MS); 
	joshvm_esp32_media_stop(handle_recorder_test);

	//joshvm_esp32_vad_stop();

	//joshvm_esp32_wakeup_disable();

	//joshvm_esp32_vad_stop();
//	printf("1valie_size = %d\n",handle_recorder_test->j_union.audioRecorder.rec_rb->valid_size);
//	printf("2valie_size = %d\n",handle_track_test->j_union.audioTrack.track_rb->valid_size);

	joshvm_esp32_media_start(handle_track_test,media_player_callback_test);
//	printf("3valie_size = %d\n",handle_recorder_test->j_union.audioRecorder.rec_rb->valid_size);
//	printf("4valie_size = %d\n",handle_track_test->j_union.audioTrack.track_rb->valid_size);
	
	joshvm_esp32_media_pause(handle_track_test);


	handle_track_test->j_union.audioTrack.track_rb = handle_recorder_test->j_union.audioRecorder.rec_rb;
//	printf("5valie_size = %d\n",handle_recorder_test->j_union.audioRecorder.rec_rb->valid_size);
//	printf("6valie_size = %d\n",handle_track_test->j_union.audioTrack.track_rb->valid_size);


	joshvm_esp32_media_start(handle_track_test,media_player_callback_test);

	vTaskDelay(5000 / portTICK_PERIOD_MS); 

	joshvm_esp32_media_pause(handle_track_test);
	vTaskDelay(5000 / portTICK_PERIOD_MS); 
	joshvm_esp32_media_start(handle_track_test,media_player_callback_test);

	vTaskDelay(5000 / portTICK_PERIOD_MS); 
	printf("track  %d\n",audio_element_get_state(handle_track_test->j_union.audioTrack.audiotrack_t.i2s));
	joshvm_esp32_media_stop(handle_track_test);



/*
 
*/

/*
	joshvm_esp32_media_set_output_file(handle_media_player_test,"/sdcard/16000.wav");
	joshvm_esp32_media_set_audio_sample_rate(handle_media_player_test,16000);
	joshvm_esp32_media_set_channel_config(handle_media_player_test,1);
	joshvm_esp32_media_prepare(handle_media_player_test,NULL);
	joshvm_esp32_media_start(handle_media_player_test,media_player_callback_test);

	vTaskDelay(10000 / portTICK_PERIOD_MS); 

	joshvm_esp32_media_stop(handle_media_player_test);
	
	joshvm_esp32_media_release(handle_media_player_test);
*/
}




//--test-------------------

void test_callback(int index)
{
	printf("wakeup   callback  index = %d\n",index);
	
}

void test_vad_callback(int index)
{
	printf("vad   callback  ---------------- index = %d\n",index);
	
}

void test_rec_engine(void)
{

	joshvm_esp32_wakeup_enable(test_callback);

	joshvm_esp32_vad_start(test_vad_callback);


}


