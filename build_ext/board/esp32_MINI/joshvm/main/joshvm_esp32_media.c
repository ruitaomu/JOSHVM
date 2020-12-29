#include "audio_pipeline.h"
#include "esp_audio.h"
#include "audio_mem.h"
#include "audio_common.h"
#include "audio_pipeline.h"
#include "board.h"
#include "joshvm_esp32_ring_buff.h"
#include "joshvm_esp32_media.h"
#include "joshvm_esp32_record.h"
#include "joshvm_esp32_player.h"
#include "joshvm.h"
#include "esp_log.h"
#include "string.h"
#include "board.h"

//---define
#define TAG  "JOSHVM_ESP32_MEDIA"
//5*48*2*1000    48K*2CHA*16/8*2.5S = 16K*1CHA*16/8*15S 
#define A_RECORD_RB_SIZE (48*10000)
#define A_TRACK_RB_SIZE (48*10000)
#define A_VAD_RB_SIZE (48*10000)	//16K*1CHA*16/8*15S 
//---variable
static int8_t audio_status = 0;
typedef struct{
	int format;
	char* url;
	int sample_rate;
	int channel;
	int bit_rate;
}j_meida_rec_default_cfg_t;
static j_meida_rec_default_cfg_t j_meida_rec_default_cfg = {joshvm_meida_format_wav,"/sdcard/default.wav",16000,1,16}; 
//static j_meida_rec_default_cfg_t j_meida_rec_info = {joshvm_meida_format_wav,"/sdcard/default.wav",16000,1,16}; 



static struct{
	int sample_rate;
	int channel;
	int bit_rate;
}j_audio_track_default_cfg = {16000,1,16};

static struct{
	int sample_rate;
	int channel;
	int bit_rate;
}j_audio_rec_default_cfg = {16000,1,16};


joshvm_media_t *joshvm_media_vad = NULL;
static TaskHandle_t audio_recorder_handler = NULL;
TaskHandle_t audio_track_handler = NULL;
static uint8_t m_player_obj_created_status = 0;
static uint8_t m_rec_obj_created_status = 0;
static uint8_t a_track_obj_created_status = 0;
static uint8_t a_rec_obj_created_status = 0;
static uint8_t a_vad_obj_created_status = 0;
uint8_t wakeup_obj_created_status = 0;
static ring_buffer_t audio_recorder_rb;
static ring_buffer_t audio_track_rb;
static ring_buffer_t audio_vad_rb;
static uint8_t run_one_time = 0;
audio_board_handle_t MegaBoard_handle = NULL;
extern SemaphoreHandle_t xSemaphore_MegaBoard_init;
extern SemaphoreHandle_t s_mutex_recorder;
extern SemaphoreHandle_t s_mutex_player;
extern int j_audioBoard_volume;
//extern EventGroupHandle_t j_EventGroup_player;
extern audio_element_handle_t create_i2s_stream(int sample_rates, int bits, int channels, audio_stream_type_t type);


void joshvm_esp32_media_callback(joshvm_media_t * handle,joshvm_err_t errcode)
{
	ESP_LOGI(TAG,"joshvm_esp32_media_callback");

	//audio_status = JOSHVM_MEDIA_RESERVE;
	handle->j_union.mediaPlayer.callback(handle,errcode);
}

joshvm_err_t joshvm_mep32_board_init()
{
	if(xSemaphoreTake( xSemaphore_MegaBoard_init, ( TickType_t ) 0 ) == pdTRUE){		
		ESP_LOGI(TAG,"Init I2s");
		audio_element_handle_t josh_i2s_stream_reader = create_i2s_stream(48000,16,1,AUDIO_STREAM_READER);
		audio_element_handle_t josh_i2s_stream_writer = create_i2s_stream(48000,16,1,AUDIO_STREAM_WRITER);
		
//		if(joshvm_esp32_i2s_create() == JOSHVM_FAIL){
//			return JOSHVM_FAIL;
//		}
		ESP_LOGI(TAG,"Init Board");
		MegaBoard_handle = audio_board_init();
		if((MegaBoard_handle->audio_hal == NULL) || (MegaBoard_handle->adc_hal == NULL)){
			goto err;			
		}
	
		int ret = audio_hal_ctrl_codec(MegaBoard_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);
		if(ret != ESP_OK){
			goto err;
		}
		audio_hal_set_volume(MegaBoard_handle->audio_hal,j_audioBoard_volume);
		audio_element_deinit(josh_i2s_stream_reader);
		audio_element_deinit(josh_i2s_stream_writer);
		return JOSHVM_OK;
		err:
			ESP_LOGE(TAG,"Init Board Failed!");
			audio_element_deinit(josh_i2s_stream_reader);
			audio_element_deinit(josh_i2s_stream_writer);
			xSemaphoreGive( xSemaphore_MegaBoard_init );
			//joshvm_esp32_i2s_deinit();
			if(MegaBoard_handle->audio_hal != NULL){
				audio_hal_deinit(MegaBoard_handle->audio_hal);	
				MegaBoard_handle->audio_hal = NULL;
			}
			if(MegaBoard_handle->adc_hal != NULL){
				audio_hal_deinit(MegaBoard_handle->adc_hal);
				MegaBoard_handle->adc_hal = NULL;
			}
			if(MegaBoard_handle != NULL){
				audio_free(MegaBoard_handle);
				MegaBoard_handle = NULL;
			}		
		return JOSHVM_FAIL;
	}else{
		if((MegaBoard_handle == NULL) || (MegaBoard_handle->audio_hal == NULL) || (MegaBoard_handle->adc_hal == NULL)){
			vTaskDelay(2000 / portTICK_PERIOD_MS);//waitting for another task create i2s and init baord,when two task call this fun at the sametime
			if((MegaBoard_handle == NULL) || (MegaBoard_handle->audio_hal == NULL) || (MegaBoard_handle->adc_hal == NULL))return JOSHVM_FAIL;
		}
		ESP_LOGI(TAG,"Audio_Board had already Init");
	}
	return JOSHVM_OK;
}

int joshvm_esp32_media_create(int type, void** handle)
{
	ESP_LOGW(TAG,"Create object,free heap size = %d",heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT));
	if(run_one_time == 0){
		run_one_time = 1;		
		//printf("---<<<MEGA_ESP32 Firmware Version Alpha_v1.5001>>>---\r\n");	
		printf("-------------------------- JOSH OPEN SMART HARDWARE --------------------------\n");
		printf("|                                                                            |\n");
		printf("|                    MEGA_ESP32 Firmware Version v1.0.2.7                    |\n");
		printf("|                         For ESP32-LyraT-Mini-v1.1                          |\n");
		printf("|                         Compile data:Jan. 3 2020                           |\n");
		printf("------------------------------------------------------------------------------\n");		
	}

	if(joshvm_mep32_board_init() != JOSHVM_OK){
		return JOSHVM_FAIL;
	}

	int ret = JOSHVM_OK;
	joshvm_media_t* joshvm_media = NULL;	
	if(type != AUDIO_VAD_REC){	
		switch(type){
			case MEDIA_PLAYER: 
				if(xSemaphoreTake( s_mutex_player, ( TickType_t ) 0 ) != pdTRUE){
					ESP_LOGE(TAG,"player(such as meidaPlyaer/audioTrack) obj can only run one");
					return JOSHVM_FAIL;
				}
		
				if(m_player_obj_created_status != OBJ_CREATED){
					m_player_obj_created_status = OBJ_CREATED;					
					joshvm_media = (joshvm_media_t*)audio_calloc(1, sizeof(joshvm_media_t));
					joshvm_media->media_type = type;
					joshvm_media->evt_que = xQueueCreate(4, sizeof(esp_audio_state_t));
					if((joshvm_media->j_union.mediaPlayer.evt_group_stop = xEventGroupCreate()) == NULL){
						ESP_LOGE(TAG,"player evt_group_stop create failed!");						
						return JOSHVM_FAIL;
					} 
					*handle = joshvm_media;			
					ret = JOSHVM_OK;
				}else{
					ESP_LOGW(TAG,"MediaPlayer has arleady created!Can't create again!");
					ret =  JOSHVM_FAIL;
				}
				break;
			case MEDIA_RECORDER: 	
				if(xSemaphoreTake( s_mutex_recorder, ( TickType_t ) 0 ) != pdTRUE){
					ESP_LOGE(TAG,"recorder(such as meidaRecorder/audioRecorder/wakeup&vad) obj can only run one");
					return JOSHVM_FAIL;
				}
			
				if(m_rec_obj_created_status != OBJ_CREATED){
					m_rec_obj_created_status = OBJ_CREATED;					
					joshvm_media = (joshvm_media_t*)audio_calloc(1, sizeof(joshvm_media_t));
					joshvm_media->media_type = type;
					joshvm_media->evt_que = xQueueCreate(4, sizeof(esp_audio_state_t));
					
					joshvm_media->j_union.mediaRecorder.format = j_meida_rec_default_cfg.format;
					joshvm_media->j_union.mediaRecorder.url = j_meida_rec_default_cfg.url;
					joshvm_media->j_union.mediaRecorder.sample_rate = j_meida_rec_default_cfg.sample_rate;
					joshvm_media->j_union.mediaRecorder.channel = j_meida_rec_default_cfg.channel;
					joshvm_media->j_union.mediaRecorder.bit_rate = j_meida_rec_default_cfg.bit_rate;	
				
//					if(joshvm_meida_recorder_init(joshvm_media) != JOSHVM_OK){
//						joshvm_esp32_media_close(joshvm_media);
//						return JOSHVM_FAIL;
//					}
					ESP_LOGI(TAG,"MediaRecorder created!");					
					*handle = joshvm_media;
				}else{
					ESP_LOGW(TAG,"MediaRecorder has arleady created!Can't create again!");
					ret =  JOSHVM_FAIL;
				}
				break;
			case AUDIO_TRACK:
				if(xSemaphoreTake( s_mutex_player, ( TickType_t ) 0 ) != pdTRUE){
					ESP_LOGE(TAG,"player(such as meidaPlyaer/audioTrack) obj can only run one");
					return JOSHVM_FAIL;
				}
				
				if(a_track_obj_created_status != OBJ_CREATED){
					a_track_obj_created_status = OBJ_CREATED;					
					joshvm_media = (joshvm_media_t*)audio_calloc(1, sizeof(joshvm_media_t));
					joshvm_media->media_type = type;
					joshvm_media->evt_que = xQueueCreate(4, sizeof(esp_audio_state_t));
					
					ring_buffer_init(&audio_track_rb,A_TRACK_RB_SIZE);
					joshvm_media->j_union.audioTrack.status = AUDIO_UNKNOW;
					joshvm_media->j_union.audioTrack.sample_rate = j_audio_track_default_cfg.sample_rate;
					joshvm_media->j_union.audioTrack.channel = j_audio_track_default_cfg.channel;
					joshvm_media->j_union.audioTrack.bit_rate = j_audio_track_default_cfg.bit_rate;

					joshvm_media->j_union.audioTrack.track_rb = &audio_track_rb;
					ESP_LOGI(TAG,"AudioTrack created!");					
					*handle = joshvm_media;
					ret = JOSHVM_OK;
				}else{
					ESP_LOGW(TAG,"AudioTrack has arleady created!Can't create again!");
					ret =  JOSHVM_FAIL;
				}
				break;
			case AUDIO_RECORDER:	
				if(xSemaphoreTake( s_mutex_recorder, ( TickType_t ) 0 ) != pdTRUE){
					ESP_LOGE(TAG,"recorder(such as meidaRecorder/audioRecorder/wakeup&vad) obj can only run one");
					return JOSHVM_FAIL;
				}
				
				if(a_rec_obj_created_status != OBJ_CREATED){
					a_rec_obj_created_status = OBJ_CREATED;					
					joshvm_media = (joshvm_media_t*)audio_calloc(1, sizeof(joshvm_media_t));
					joshvm_media->media_type = type;
					joshvm_media->evt_que = xQueueCreate(4, sizeof(esp_audio_state_t));
					
					ring_buffer_init(&audio_recorder_rb,A_RECORD_RB_SIZE);
					joshvm_media->j_union.audioRecorder.sample_rate = j_audio_rec_default_cfg.sample_rate;
					joshvm_media->j_union.audioRecorder.channel = j_audio_rec_default_cfg.channel;
					joshvm_media->j_union.audioRecorder.bit_rate = j_audio_rec_default_cfg.bit_rate;
					joshvm_media->j_union.audioRecorder.rec_rb = &audio_recorder_rb;
					ESP_LOGI(TAG,"AudioRecorder created!");					
					*handle = joshvm_media;
					ret = JOSHVM_OK;
				}else{
					ESP_LOGW(TAG,"AudioRecorder has arleady created!Can't create again!");
					ret =  JOSHVM_FAIL;
				}
				break;
			default :
				return JOSHVM_INVALID_ARGUMENT;
				break;
		}			
		return ret;
	}else if(type == AUDIO_VAD_REC){		
		if(a_vad_obj_created_status != OBJ_CREATED){			
			a_vad_obj_created_status = OBJ_CREATED;
			joshvm_media_vad = (joshvm_media_t*)audio_calloc(1, sizeof(joshvm_media_t));
			joshvm_media_vad->media_type = type;
			joshvm_media_vad->evt_que = xQueueCreate(4, sizeof(esp_audio_state_t));

			ring_buffer_init(&audio_vad_rb,A_VAD_RB_SIZE);
			joshvm_media_vad->j_union.vad.rec_rb = &audio_vad_rb;				
			ESP_LOGI(TAG,"VAD AudioRecorder created!");					
			*handle = joshvm_media_vad;
			ret = JOSHVM_OK;			
		}else{
			ESP_LOGW(TAG,"VAD has arleady created!Can't create again!");
			ret =  JOSHVM_FAIL;
		}
		return ret;
	}
	return JOSHVM_INVALID_ARGUMENT;
}


int joshvm_esp32_media_close(joshvm_media_t* handle)
{
	ESP_LOGI(TAG,"joshvm_esp32_media_close");
	if(handle == NULL){
		return JOSHVM_OK;
	}	
	
	switch(handle->media_type)
	{
		case MEDIA_PLAYER:
			m_player_obj_created_status = OBJ_CREATED_NOT;	
			xSemaphoreGive(s_mutex_player);
			joshvm_esp32_media_stop(handle);
			if(handle->j_union.mediaPlayer.evt_group_stop != NULL){
				vEventGroupDelete(handle->j_union.mediaPlayer.evt_group_stop);
				handle->j_union.mediaPlayer.evt_group_stop = NULL;
			}
			ESP_LOGI(TAG,"MediaPlayer closed!");			
			break;
		case MEDIA_RECORDER:
			m_rec_obj_created_status = OBJ_CREATED_NOT;
			joshvm_esp32_media_stop(handle);
			xSemaphoreGive(s_mutex_recorder);
			ESP_LOGI(TAG,"MediaRecorder closed!");
			break;
		case AUDIO_TRACK:
			a_track_obj_created_status = OBJ_CREATED_NOT;
			ring_buffer_deinit(handle->j_union.audioTrack.track_rb);
			joshvm_esp32_media_stop(handle);
			xSemaphoreGive(s_mutex_player);
			break;
		case AUDIO_RECORDER:
			a_rec_obj_created_status = OBJ_CREATED_NOT;
			ring_buffer_deinit(handle->j_union.audioRecorder.rec_rb);
			joshvm_esp32_media_stop(handle);
			xSemaphoreGive(s_mutex_recorder);
			break;
		case AUDIO_VAD_REC:			
			a_vad_obj_created_status = OBJ_CREATED_NOT;
			ring_buffer_deinit(handle->j_union.vad.rec_rb);
			
			break;
		default:
			break;
	}

	if(handle->evt_que){
		vQueueDelete(handle->evt_que);
		handle->evt_que = NULL;
	}

	if(handle){
		audio_free(handle);		
		handle = NULL;
	}
	
	if((m_player_obj_created_status == OBJ_CREATED_NOT)\
		&& (m_rec_obj_created_status == OBJ_CREATED_NOT)\
		&& (a_track_obj_created_status == OBJ_CREATED_NOT)\
		&& (a_rec_obj_created_status == OBJ_CREATED_NOT)\
		&& (a_vad_obj_created_status == OBJ_CREATED_NOT)\
		&& (wakeup_obj_created_status == OBJ_CREATED_NOT)){
		audio_hal_ctrl_codec(MegaBoard_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_STOP);
		if(MegaBoard_handle->audio_hal != NULL){
			audio_hal_deinit(MegaBoard_handle->audio_hal);	
			MegaBoard_handle->audio_hal = NULL;
		}
		if(MegaBoard_handle->adc_hal != NULL){
			audio_hal_deinit(MegaBoard_handle->adc_hal);
			MegaBoard_handle->adc_hal = NULL;
		}
		if(MegaBoard_handle != NULL){
			audio_free(MegaBoard_handle);
			MegaBoard_handle = NULL;
		}		
		xSemaphoreGive( xSemaphore_MegaBoard_init );
		//joshvm_esp32_i2s_deinit();
	}
	return JOSHVM_OK;
}

//void joshvm_media_recorder_setinfo(  joshvm_media_t *handle)
//{
//	handle->j_union.mediaRecorder.bit_rate 		= j_meida_rec_info.bit_rate;
//	handle->j_union.mediaRecorder.channel 		= j_meida_rec_info.channel;
//	handle->j_union.mediaRecorder.format 		= j_meida_rec_info.format;
//	handle->j_union.mediaRecorder.sample_rate 	= j_meida_rec_info.sample_rate;
//	handle->j_union.mediaRecorder.url 			= j_meida_rec_info.url;
//}

int joshvm_esp32_media_prepare(joshvm_media_t* handle, void(*callback)(void*, int))
{
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}

	ESP_LOGI(TAG,"joshvm_esp32_media_prepare");
	int ret = JOSHVM_OK;
	switch(handle->media_type){
		case MEDIA_PLAYER:
			if(joshvm_audio_wrapper_init(handle) != JOSHVM_OK){
				ESP_LOGE(TAG,"mediaPlayer prepare failed");
				joshvm_esp32_media_stop(handle);						
				return JOSHVM_FAIL;
			}		
			handle->j_union.mediaPlayer.obj_release_flag = OBJ_release_need;
			break;
		case MEDIA_RECORDER:	
			if(joshvm_meida_recorder_init(handle) != JOSHVM_OK){
				joshvm_esp32_media_stop(handle);
				//joshvm_esp32_media_close(handle);
				ESP_LOGE(TAG,"mediaRecorder prepare failed");
				return JOSHVM_FAIL;
			}
			handle->j_union.mediaRecorder.obj_release_flag = OBJ_release_need;
			//joshvm_media_recorder_setinfo(handle);
			if(joshvm_meida_recorder_cfg(handle) != ESP_OK){
				return JOSHVM_FAIL;
			}
			break;
		default : 
			ret = JOSHVM_NOT_SUPPORTED;
			break;
	}
	return ret;
}

int joshvm_esp32_media_start(joshvm_media_t* handle, void(*callback)(void*, int))
{
	ESP_LOGI(TAG,"joshvm_esp32_media_start");
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}
	QueueHandle_t que = handle->evt_que;
	uint16_t que_val = 0;
	int ret = JOSHVM_OK;
		switch(handle->media_type){
			case MEDIA_PLAYER:	
				//play
				if((handle->j_union.mediaPlayer.status == AUDIO_UNKNOW) || (handle->j_union.mediaPlayer.status == AUDIO_STOP)){
					handle->j_union.mediaPlayer.callback = callback;
					if(joshvm_audio_play_handler(handle->j_union.mediaPlayer.url) != ESP_OK){
						return JOSHVM_FAIL;
					}
					handle->j_union.mediaPlayer.status = AUDIO_START;
					ESP_LOGW(TAG,"player,free heap size = %d",heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT));
					break;
				}
				//pause->play
				else if(handle->j_union.mediaPlayer.status == AUDIO_PAUSE){				
					if(joshvm_audio_resume_handler(handle->j_union.mediaPlayer.url) != ESP_OK) return JOSHVM_FAIL;	
					handle->j_union.mediaPlayer.status = AUDIO_START;
					ret = JOSHVM_OK;
					break;
				}
				break;
			case MEDIA_RECORDER:
				if(audio_pipeline_run(handle->j_union.mediaRecorder.recorder_t.pipeline) != ESP_OK){
					return JOSHVM_FAIL;
				}
				break;
			case AUDIO_TRACK:
				//play
				if((handle->j_union.audioTrack.status == AUDIO_UNKNOW) || (handle->j_union.audioTrack.status == AUDIO_STOP) || (handle->j_union.audioTrack.status == AUDIO_FINISH)){
					handle->j_union.audioTrack.status = AUDIO_START;
					handle->j_union.audioTrack.rb_callback_flag = NO_NEED_CB;				
					if(joshvm_audio_track_init(handle) != JOSHVM_OK){
						handle->j_union.audioTrack.status = AUDIO_STOP;					
						//joshvm_esp32_media_close(handle);
						joshvm_esp32_media_stop(handle);
						return JOSHVM_FAIL;
					}
					handle->j_union.audioTrack.obj_release_flag = OBJ_release_need;
					que_val = QUE_TRACK_START;
					xQueueSend(que, &que_val, (portTickType)0);
					xTaskCreate(joshvm_audio_track_task,"joshvm_audio_track_task",2*1024,(void*)handle,JOSHVM_AUDIO_TRACK_TASK_PRI,&audio_track_handler);
					ESP_LOGI(TAG,"AudioTrack start!");
					break;
				}
				//pause->play
				else if(handle->j_union.audioTrack.status == AUDIO_PAUSE){
					printf("start -- resume\n");
					que_val = QUE_TRACK_START;
					xQueueSend(que, &que_val, (portTickType)0);
					handle->j_union.audioTrack.status = AUDIO_START;
					ret = JOSHVM_OK;
					break;

				}
				break;
			case AUDIO_RECORDER:
				handle->j_union.audioRecorder.status = AUDIO_START;
				handle->j_union.audioRecorder.rb_callback_flag = NO_NEED_CB;				
				if(joshvm_audio_recorder_init(handle) != JOSHVM_OK){
					handle->j_union.audioRecorder.status = AUDIO_STOP;
					//joshvm_esp32_media_close(handle);
					joshvm_esp32_media_stop(handle);
					return JOSHVM_FAIL;
				}
				handle->j_union.audioRecorder.obj_release_flag = OBJ_release_need;
				xTaskCreate(joshvm_audio_recorder_task, "joshvm_audio_recorder_task", 2 * 1024, (void*)handle, JOSHVM_AUDIO_RECORDER_TASK_PRI, &audio_recorder_handler);
				ESP_LOGI(TAG,"AudioRecorder start!");
				break;
			default :
				ret = JOSHVM_NOT_SUPPORTED;
				break;
		}
	return ret;
}

int joshvm_esp32_media_pause(joshvm_media_t* handle)
{
	ESP_LOGI(TAG,"joshvm_esp32_media_pause");
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}
	
	QueueHandle_t que = handle->evt_que;
	uint16_t que_val = 0;
	int ret;	
	switch(handle->media_type){
		case MEDIA_PLAYER:			
			if(joshvm_audio_pause() != ESP_OK){
				return JOSHVM_FAIL;
			}
			handle->j_union.mediaPlayer.status = AUDIO_PAUSE;
			ret = JOSHVM_OK;
			break;
		case MEDIA_RECORDER:
			ret = JOSHVM_NOT_SUPPORTED;
			break;
		case AUDIO_TRACK:
			que_val = QUE_TRACK_PAUSE;
			xQueueSend(que, &que_val, (portTickType)0);
			handle->j_union.audioTrack.status = AUDIO_PAUSE;
			ret = JOSHVM_OK;			
			break;
		case AUDIO_RECORDER:
			ret = JOSHVM_NOT_SUPPORTED;
			break;
		default :
			ret = JOSHVM_NOT_SUPPORTED;
			break;
	}
	return ret;
}

int joshvm_esp32_media_stop(joshvm_media_t* handle)
{
	ESP_LOGI(TAG,"joshvm_esp32_media_stop");
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}
	QueueHandle_t que = handle->evt_que;
	uint16_t que_val = 0;
	int ret = JOSHVM_FAIL;
	switch(handle->media_type){
		case MEDIA_PLAYER:	
			if(handle->j_union.mediaPlayer.obj_release_flag == OBJ_release_need){
				handle->j_union.mediaPlayer.obj_release_flag = OBJ_release_no;
				if((ret = joshvm_audio_stop_handler(handle)) != ESP_OK){
					return JOSHVM_FAIL;
				}	
				joshvm_audio_player_destroy();		
				ESP_LOGI(TAG,"MediaPlayer stop!");
			}
			handle->j_union.mediaPlayer.status = AUDIO_STOP;
			ret = JOSHVM_OK;			
			break;
		case MEDIA_RECORDER:
			if(handle->j_union.mediaRecorder.obj_release_flag == OBJ_release_need){
				handle->j_union.mediaRecorder.obj_release_flag = OBJ_release_no;
				if(audio_pipeline_terminate(handle->j_union.mediaRecorder.recorder_t.pipeline) != ESP_OK) return JOSHVM_FAIL;
				joshvm_media_recorder_release(handle);
				ESP_LOGI(TAG,"MediaRecorder stop!");
			}
			ret = JOSHVM_OK;
			break;
		case AUDIO_TRACK:	
			if(handle->j_union.audioTrack.obj_release_flag == OBJ_release_need){
				handle->j_union.audioTrack.obj_release_flag = OBJ_release_no;
				//handle->j_union.audioTrack.status = AUDIO_STOP;
				que_val = QUE_TRACK_STOP;
				xQueueSend(que, &que_val, (portTickType)0);			
				ESP_LOGI(TAG,"AudioTrack stop!");
			}
			handle->j_union.audioTrack.status = AUDIO_STOP;
			ret = JOSHVM_OK;
			break;
		case AUDIO_RECORDER:
			if(handle->j_union.audioRecorder.obj_release_flag == OBJ_release_need){
				handle->j_union.audioRecorder.obj_release_flag = OBJ_release_no;
				handle->j_union.audioRecorder.status = AUDIO_STOP;
				que_val = QUE_RECORD_STOP;
				xQueueSend(que, &que_val, (portTickType)0);
				ESP_LOGI(TAG,"AudioRedorder stop1! %p  %p",handle,handle->j_union.audioRecorder.audiorecorder_t.pipeline);
				if(audio_pipeline_terminate(handle->j_union.audioRecorder.audiorecorder_t.pipeline) != ESP_OK)	return JOSHVM_FAIL;
				ESP_LOGI(TAG,"AudioRedorder stop2!");
				joshvm_audio_rcorder_release(handle);
				ESP_LOGI(TAG,"AudioRedorder stop3!");
			}			
			ret = JOSHVM_OK;
			break;
		default :
			ret = JOSHVM_NOT_SUPPORTED;
			break;
	}
	return ret;
}


int joshvm_esp32_media_reset(joshvm_media_t* handle)
{
	ESP_LOGI(TAG,"joshvm_esp32_media_reset");
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}
	int ret;
	switch(handle->media_type){
		case MEDIA_PLAYER:			

			ret = JOSHVM_OK;
			break;
		case MEDIA_RECORDER:
			handle->j_union.mediaRecorder.format = j_meida_rec_default_cfg.format;
			handle->j_union.mediaRecorder.url = j_meida_rec_default_cfg.url;
			handle->j_union.mediaRecorder.sample_rate = j_meida_rec_default_cfg.sample_rate;
			handle->j_union.mediaRecorder.channel = j_meida_rec_default_cfg.channel;
			handle->j_union.mediaRecorder.bit_rate = j_meida_rec_default_cfg.bit_rate;	
			ret = JOSHVM_OK;
			break;
		default :
			ret = JOSHVM_NOT_SUPPORTED;
			break;
	}
	return ret;
}

int joshvm_esp32_media_get_state(joshvm_media_t* handle, int* state)
{
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}

	int ret;
	switch(handle->media_type){
		case MEDIA_PLAYER:
			ret = joshvm_audio_get_state();
			switch(ret){
				case AUDIO_STATUS_RUNNING:
					*state = JOSHVM_MEDIA_PLAYING;//
					ret = JOSHVM_OK;
					break;
				case AUDIO_STATUS_PAUSED:
					*state = JOSHVM_MEDIA_PAUSED;
					ret = JOSHVM_OK;
					break;
				case AUDIO_STATUS_STOPPED:
					*state = JOSHVM_MEDIA_STOPPED;
					ret = JOSHVM_OK;
					break;
				default:
					ret = JOSHVM_FAIL;
					break;
			}
			return ret;
			break;
		case MEDIA_RECORDER:
			joshvm_media_get_state(handle,state);
			ret = JOSHVM_OK;
			break;
		case AUDIO_TRACK:
			joshvm_media_get_state(handle,state);
			ret = JOSHVM_OK;
			break;
		case AUDIO_RECORDER:
			joshvm_media_get_state(handle,state);
			ret = JOSHVM_OK;
			break;
		default :
			ret = JOSHVM_NOT_SUPPORTED;
			break;
	}
	return ret;
}

int joshvm_esp32_media_read(joshvm_media_t* handle, unsigned char* buffer, int size, int* bytesRead, void(*callback)(void*, int))
{
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}

	int ret = JOSHVM_OK;
	switch(handle->media_type){
		case AUDIO_RECORDER:
			handle->j_union.audioRecorder.rb_callback = callback;
			ret = joshvm_audio_recorder_read(handle->j_union.audioRecorder.status,\
											 handle->j_union.audioRecorder.rec_rb,buffer,size,bytesRead);
			if(ret == JOSHVM_NOTIFY_LATER){	
				handle->j_union.audioRecorder.rb_callback_flag = NEED_CB;
			}
			break;
		case AUDIO_VAD_REC:
			if(handle->j_union.vad.rec_rb == NULL){//
				*bytesRead = 0;
				ret = JOSHVM_OK;				
			}		
			handle->j_union.vad.rb_callback = callback;
			ret = joshvm_audio_recorder_read(handle->j_union.vad.status,\
											 handle->j_union.vad.rec_rb,buffer,size,bytesRead);
			if(ret == JOSHVM_NOTIFY_LATER){
				handle->j_union.vad.rb_callback_flag = NEED_CB;
			}
			break;			
		default:
			ret = JOSHVM_INVALID_ARGUMENT;
			break;		
	}
	return ret;
}

int joshvm_esp32_media_write(joshvm_media_t* handle, unsigned char* buffer, int size, int* bytesWritten, void(*callback)(void*, int))
{
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}

	if((handle->j_union.audioTrack.status == AUDIO_STOP) || (handle->j_union.audioTrack.status == AUDIO_FINISH)){
		ESP_LOGE(TAG,"Can't write track-buffer,when you stopped audioTrack.You need to play track to write track-buffer.");
		return JOSHVM_FAIL;	
	}
	
	handle->j_union.audioTrack.rb_callback = callback;
	int ret = joshvm_audio_track_write(handle->j_union.audioTrack.status,handle->j_union.audioTrack.track_rb,buffer,size,bytesWritten);
	if(ret == JOSHVM_NOTIFY_LATER){
		handle->j_union.audioTrack.rb_callback_flag = NEED_CB;
	}
	return ret;
}

int joshvm_esp32_media_flush(joshvm_media_t* handle)
{
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}

	if(handle->media_type != AUDIO_TRACK){
		ESP_LOGE(TAG,"Flush only for audioTrack!");
		return JOSHVM_FAIL;
	}

	if((handle->j_union.audioTrack.status != AUDIO_STOP) && (handle->j_union.audioTrack.status != AUDIO_PAUSE)){
		ESP_LOGE(TAG,"Flush can only run when audioTrack was stopped or pause status!");
		return JOSHVM_FAIL;
	}

	ring_buffer_flush(handle->j_union.audioTrack.track_rb);
	return 0;
}

int joshvm_esp32_media_get_buffsize(joshvm_media_t* handle, int* size)
{	
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}

	switch(handle->media_type){
		case AUDIO_TRACK:
			*size = A_TRACK_RB_SIZE;
			break;
		case AUDIO_RECORDER:
			*size = A_RECORD_RB_SIZE;
			break;
		default :
			return JOSHVM_NOT_SUPPORTED;
			break;
	}
	return 0;
}

int joshvm_esp32_media_set_audio_sample_rate(joshvm_media_t* handle, uint32_t value)
{
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}

	if((value != 8000) && (value != 16000) && (value != 48000)
	&& (value != 44100) && (value != 22050) && (value != 11025)){
		ESP_LOGW(TAG,"sample_rate should be one of 8000 16000 48000 11025 22050 44100");
		return JOSHVM_FAIL;
	}
		
	int ret;
	switch(handle->media_type){
		case MEDIA_PLAYER:					
			ret = JOSHVM_NOT_SUPPORTED;
			break;
		case MEDIA_RECORDER:
			handle->j_union.mediaRecorder.sample_rate = value;
			//j_meida_rec_info.sample_rate = value;
			ret = JOSHVM_OK;
			break;
		case AUDIO_TRACK:
			handle->j_union.audioTrack.sample_rate = value;
			ret = JOSHVM_OK;
			break;
		case AUDIO_RECORDER:
			handle->j_union.audioRecorder.sample_rate = value;
			ret = JOSHVM_OK;
			break;
		default :
			ret = JOSHVM_NOT_SUPPORTED;
			break;
	}
	return ret;
}

int joshvm_esp32_media_set_channel_config(joshvm_media_t* handle, uint8_t value)
{
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}
	
	if((value != 1) && (value != 2)){
		ESP_LOGW(TAG,"channels only support 1 or 2");
		return JOSHVM_FAIL;
	}

	int ret;
	switch(handle->media_type){
		case MEDIA_PLAYER:					
			ret = JOSHVM_NOT_SUPPORTED;
			break;
		case MEDIA_RECORDER:
			handle->j_union.mediaRecorder.channel = value;
			//j_meida_rec_info.channel = value;
			ret = JOSHVM_OK;
			break;
		case AUDIO_TRACK:
			handle->j_union.audioTrack.channel = value;
			ret = JOSHVM_OK;
			break;
		case AUDIO_RECORDER:
			handle->j_union.audioRecorder.channel = value;
			ret = JOSHVM_OK;
			break;
		default :
			ret = JOSHVM_NOT_SUPPORTED;
			break;
	}
	return ret;
}

int joshvm_esp32_media_set_audio_bit_rate(joshvm_media_t* handle, uint8_t value)
{
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}
	if((value != 8) && (value != 16)){
		ESP_LOGW(TAG,"bitrates only support 8 or 16");
		return JOSHVM_FAIL;
	}

	int ret;
	switch(handle->media_type){
		case MEDIA_PLAYER:					
			ret = JOSHVM_NOT_SUPPORTED;
			break;
		case MEDIA_RECORDER:
			handle->j_union.mediaRecorder.bit_rate = value;
			//j_meida_rec_info.bit_rate = value;
			ret = JOSHVM_OK;
			break;
		case AUDIO_TRACK:
			handle->j_union.audioTrack.bit_rate = value;
			ret = JOSHVM_OK;
			break;
		case AUDIO_RECORDER:
			handle->j_union.audioRecorder.bit_rate = value;
			ret = JOSHVM_OK;
			break;
		default :
			ret = JOSHVM_NOT_SUPPORTED;
			break;
	}
	return ret;

}

static const char fatfs_file[7] = "sdcard";
static const char spiffs_file[9] = "userdata";
static char url_source[64];
int joshvm_esp32_media_set_source(joshvm_media_t* handle, char* source)
{
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}

	int ret;	
	switch(handle->media_type){
		case MEDIA_PLAYER:
			//spiffs
			if(strstr(source,spiffs_file) != NULL){
				sprintf(url_source,"spiffs:/%s",source);	
				handle->j_union.mediaPlayer.url = url_source;
				ESP_LOGI(TAG,"Set MediaPlayer spiffs Source:%s",url_source);
			}//fatfs
			else if(strstr(source,fatfs_file) != NULL){
				sprintf(url_source,"file:/%s",source);
				handle->j_union.mediaPlayer.url = url_source;
				ESP_LOGI(TAG,"Set MediaPlayer fatfs Source:%s",url_source);
			}//http_url
			else{
				handle->j_union.mediaPlayer.url = source;
				ESP_LOGI(TAG,"Set MediaPlayer http Source:%s",source);
			}			
			ret = JOSHVM_OK;
			break;
		default :
			ESP_LOGI(TAG,"Set MediaPlayer Source fail,media type :%d,source:%s",handle->media_type ,source);
			ret = JOSHVM_NOT_SUPPORTED;
			break;
	}
	return ret;
}

int joshvm_esp32_media_set_output_file(joshvm_media_t* handle, char* file)
{
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}

	int ret;
	switch(handle->media_type){
		case MEDIA_RECORDER:
			handle->j_union.mediaRecorder.url = file;
			//j_meida_rec_info.url = file;
			ret = JOSHVM_OK;
			break;
		default :
			ret = JOSHVM_NOT_SUPPORTED;
			break;
	}
	return ret;
}

int joshvm_esp32_media_set_output_format(joshvm_media_t* handle, int format)
{
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}

	int ret;
	switch(handle->media_type){
		case MEDIA_RECORDER:
			handle->j_union.mediaRecorder.format = format;
			//j_meida_rec_info.format = format;
			ret = JOSHVM_OK;
			break;
		default :
			ret = JOSHVM_NOT_SUPPORTED;
			break;
	}
	return ret;
}

int joshvm_esp32_media_get_position(joshvm_media_t* handle, int* pos)
{
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}

	int ret;
	switch(handle->media_type){
		case MEDIA_PLAYER:
			joshvm_audio_time_get(pos);
			ret = JOSHVM_OK;
			break;
		default :
			ret = JOSHVM_NOT_SUPPORTED;
			break;
	}
	return ret;	
}

int joshvm_esp32_media_set_position(joshvm_media_t* handle, int pos, void(*callback)(void*, int))
{
	ESP_LOGI(TAG,"joshvm_esp32_media_set_position");
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}

	int ret;
	switch(handle->media_type){
		case MEDIA_PLAYER:
			//handle->j_union.mediaPlayer.positon = pos;
			ret = JOSHVM_FAIL;
			break;
		default :
			ret = JOSHVM_NOT_SUPPORTED;
			break;
	}
	return ret;	
}

int joshvm_esp32_media_get_duration(joshvm_media_t* handle, int* duration)
{
	ESP_LOGI(TAG,"joshvm_esp32_media_get_duration");
	if(handle == NULL){
		ESP_LOGE(TAG,"project handle is null!");
		return JOSHVM_FAIL;
	}

	int ret;
	switch(handle->media_type){
		case MEDIA_PLAYER:
			//*duration = handle->j_union.mediaPlayer.duration;
			ret = JOSHVM_FAIL;
			break;
		default :
			ret = JOSHVM_NOT_SUPPORTED;
			break;
	}
	return ret;	
}

int joshvm_esp32_media_get_max_volume(int* volume)
{
	*volume = 100;
	return JOSHVM_OK;
}

int joshvm_esp32_media_get_volume(int* volume)
{
	ESP_LOGI(TAG,"joshvm_esp32_media_get_volume");
	return joshvm_volume_get_handler(volume);
}

int joshvm_esp32_media_set_volume(int volume)
{
	ESP_LOGI(TAG,"joshvm_esp32_media_set_volume");
	//adjust
	volume++;
	return joshvm_volume_set_handler(volume);
}

int joshvm_esp32_media_add_volume()
{
	ESP_LOGI(TAG,"joshvm_esp32_media_add_volume");
	joshvm_volume_adjust_handler(11);
	return 0;
}

int joshvm_esp32_media_sub_volume()
{
	ESP_LOGI(TAG,"joshvm_esp32_media_sub_volume");
	joshvm_volume_adjust_handler(-9);
	return 0;
}

int joshvm_esp32_get_sys_info(char* info, int size)
{
	char firmware_version[] = "<<<ESP32-LyraT-Mini_v1.1 Firmware Version v1.0.2.7>>>";
	if(size < strlen(firmware_version)){
		return JOSHVM_FAIL;
	}
	strncpy(info, firmware_version, strlen(firmware_version));
	return JOSHVM_OK;
}

