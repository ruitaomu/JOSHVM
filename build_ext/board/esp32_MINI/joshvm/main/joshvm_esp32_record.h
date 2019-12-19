#ifndef JOSHVM_ESP32_RECORD_H_
#define JOSHVM_ESP32_RECORD_H_

#include "audio_element.h"
#include "audio_pipeline.h"
#include "i2s_stream.h"
#include "filter_resample.h"
#include "fatfs_stream.h"
#include "http_stream.h"
#include "wav_encoder.h"
#include "esp_log.h"
#include "joshvm_esp32_media.h"


//---define



//---enum
enum{
	joshvm_meida_format_wav = 0,
	joshvm_meida_format_amrnb,
	joshvm_meida_format_amrwb,
	joshvm_meida_format_opus
}joshvm_meida_format_e;

//---struct



//---fun
/**
 * @brief init i2s_reader&i2s_writer ,cfg by default
 *
 * @note 
 * @param 	
 *			
 *
 * @return (error code)
 *     - 0: ok
 *     	 -1: fail
 *		...
 */
joshvm_err_t joshvm_esp32_i2s_create(void);

/**
 * @brief deinit i2s_reader&i2s_writer ,cfg by default
 *
 * @note 
 * @param 	
 *			
 *
 * @return (error code)
 *     - 0: ok
 *     	 -1: fail
 *		...
 */
joshvm_err_t joshvm_esp32_i2s_deinit(void);
	

/**
 * @brief init meida recorder ,cfg by default
 *
 * @note 
 * @param 	
 *			handle
 *
 * @return (error code)
 *     - 0: ok
 *     	 -1: fail
 *		...
 */
int joshvm_meida_recorder_init(joshvm_media_t * handle);

/**
 * @brief cfg meida recorder ,call by prepare
 *
 * @note 
 * @param 	
 *			handle
 *
 * @return (error code)
 *     - 0: ok
 *     	 -1: fail
 *		...
 */
int joshvm_meida_recorder_cfg(joshvm_media_t *handle);

/**
 * @brief get meida recorder state
 *
 * @note 
 * @param 	
 *			handle
 *[out]	  state
 *    		- 1:stop
 *			  2:pause
 *			  3:playing/recording
 *
 * @return (error code)
 *     - 0: ok
 *     	 -1: fail
 *		...
 */
int joshvm_media_get_state(joshvm_media_t* handle,int* state);

/**
 * @brief init audio track
 *
 * @note 
 * @param 	
 *			handle
 *
 * @return (error code)
 *     - 0: ok
 *     	 -1: fail
 *		...
 */
int joshvm_audio_track_init(joshvm_media_t* handle);

void joshvm_audio_track_task(void* handle);

int joshvm_audio_track_write(uint8_t status,ring_buffer_t* rb, unsigned char* buffer, int size, int* bytesWritten);


int joshvm_audio_recorder_init(joshvm_media_t* handle);

void joshvm_audio_recorder_task(void* handle);

int joshvm_audio_recorder_read(uint8_t status,ring_buffer_t* rb,unsigned char* buffer, int size, int* bytesRead);


void joshvm_media_recorder_release(joshvm_media_t* handle);

void joshvm_audio_track_release(joshvm_media_t* handle);

void joshvm_audio_rcorder_release(joshvm_media_t* handle);


#endif


