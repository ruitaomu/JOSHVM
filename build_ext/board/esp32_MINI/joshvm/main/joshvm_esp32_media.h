#ifndef _JOSHVM_ESP32_MEDIA_H_
#define _JOSHVM_ESP32_MEDIA_H_

#include "joshvm_esp32_ring_buff.h"
#include "freertos/event_groups.h"

//---define
//#define QUE_TRACK_START 1
//#define QUE_TRACK_STOP 2
//#define QUE_RECORD_STOP 3
#define OBJ_CREATED	1
#define OBJ_CREATED_NOT	2


//---enum
typedef enum{
	QUE_TRACK_PAUSE = 0,
	QUE_TRACK_START,
	QUE_TRACK_STOP,
	QUE_RECORD_STOP
}joshvm_evt_que_t;

typedef enum{
	JOSHVM_OK 				= 0,
	JOSHVM_FAIL 			= -1,
	JOSHVM_NOTIFY_LATER 	= -2,
	JOSHVM_OUT_OF_MEMORY	= -3,
	JOSHVM_LIMIT_RESOURCE	= -4,
	JOSHVM_INVALID_ARGUMENT	= -5,
	JOSHVM_INVALID_STATE	= -6,	
	JOSHVM_NOT_SUPPORTED	= -7, 
	JOSHVM_TIMEOUT			= -8
}joshvm_err_t;

typedef enum{
	MEDIA_PLAYER = 0,
	MEDIA_RECORDER,
	AUDIO_TRACK,
	AUDIO_RECORDER,
	AUDIO_VAD_REC
}joshvm_type_t;

enum{
	AUDIO_UNKNOW = 0,
	AUDIO_START,
	AUDIO_STOP,
	AUDIO_FINISH,//for track when stop and data playout will set audio_finish
	AUDIO_PAUSE,
	NO_NEED_CB,
	NEED_CB,
}joshvm_audio_rb_callback_status_e;

enum{
	OBJ_release_need = 0,
	OBJ_release_no
}joshvm_media_release_e;
	
//---struct
typedef struct{
	int8_t status;
	char *url;						//dataSource
	void(*callback)(void*, int);
	uint32_t sample_rate;
	uint8_t channel;
	uint8_t	bit_rate;
	int positon;
	int duration;	
	EventGroupHandle_t evt_group_stop;//for player stop
	int obj_release_flag;
}joshvm_media_mediaplayer_t;

typedef struct{
	int format;
	char *url;	
	uint32_t sample_rate;
	uint8_t channel;
	uint8_t	bit_rate;
	struct{
		void* i2s;
		void* filter;
		void* encoder;
		void* stream_writer;
		void* pipeline;
	}recorder_t;
	int obj_release_flag;
}joshvm_media_mediarecorder_t;

typedef struct{
	uint8_t status;
	uint8_t rb_callback_flag;
	void(*rb_callback)(void*, int);
	uint32_t sample_rate;
	uint8_t channel;
	uint8_t	bit_rate;
	struct{
		void* i2s;
		void* filter;
		void* raw_writer;
		void* pipeline;
	}audiotrack_t;

	ring_buffer_t *track_rb;
	void(*callback)(void*, int);
	int obj_release_flag;
}joshvm_media_audiotrack_t;

typedef struct{
	uint8_t status;
	uint8_t rb_callback_flag;
	void(*rb_callback)(void*, int);
	uint32_t sample_rate;
	uint8_t channel;
	uint8_t	bit_rate;
	struct{
		void* i2s;
		void* filter;
		void* raw_reader;
		void* pipeline;
	}audiorecorder_t;

	ring_buffer_t *rec_rb;
	int obj_release_flag;
}joshvm_media_audiorecorder_t;

typedef struct{
	uint8_t status;					//begin  & end record voice data 
	uint8_t rb_callback_flag;
	void(*rb_callback)(void*, int);
	ring_buffer_t *rec_rb;

}joshvm_media_audio_vad_rec_t;

typedef struct {
	uint8_t media_type;
	QueueHandle_t	evt_que;

	union{
		joshvm_media_mediaplayer_t mediaPlayer;
		joshvm_media_mediarecorder_t mediaRecorder;
		joshvm_media_audiotrack_t audioTrack;
		joshvm_media_audiorecorder_t audioRecorder;
		joshvm_media_audio_vad_rec_t vad;
	}j_union;
	
}joshvm_media_t;


//---fun
void joshvm_esp32_media_callback(joshvm_media_t *handle,joshvm_err_t errcode);

joshvm_err_t joshvm_mep32_board_init(void);

/**
 * @brief Create 
 *
 * @note 
 * @param 	type  
 *				-0	create MediaPlyaer
 *				-1	create MediaPlyaer
 *				-2	create MediaPlyaer
 *				-3	create MediaPlyaer
 *			handle
 *
 * @return (error code)
 *     - 0: ok
 *     	 -1: fail
 *		...
 */
int joshvm_esp32_media_create(int type, void** handle);
/**
 * @brief Release 
 *
 * @note 
 *
 *
 * @param handle
 *
 * @return (error code)
 *     	 - 0: ok
 *     	 -1: fail
 *		...
 */
int joshvm_esp32_media_close(joshvm_media_t* handle);


/**
 * @brief prepare before play / record 
 *
 * @note 
 *
 *
 * @param handle
 *        callback
 *
 * @return (error code)
 *     - 0: ok
 *     	 -1: fail
 *		...
 */
int joshvm_esp32_media_prepare(joshvm_media_t* handle, void(*callback)(void*, int));
/**
 * @brief run Media / Audio 
 *
 * @note 
 *
 *
 * @param handle
 *        callback
 *
 * @return (error code)
 *     - 0: ok
 *     	 -1: fail
 *		...
 */
int joshvm_esp32_media_start(joshvm_media_t* handle, void(*callback)(void*, int));


/**
 * @brief pause Media / Audio 
 *
 * @note 
 *
 *
 * @param handle
 *        callback
 *
 * @return (error code)
 *     - 0: ok
 *     	 -1: fail
 *		...
 */
int joshvm_esp32_media_pause(joshvm_media_t* handle);

/**
 * @brief stop Media / Audio 
 *
 * @note 
 *
 *
 * @param handle
 *    
 * @return (error code)
 *    -	0: ok
 *     	-1: fail
 *		...
 */
int joshvm_esp32_media_stop(joshvm_media_t* handle);

/**
 * @brief reset Media / Audio,reset parameter,release 
 *
 * @note 
 *
 *
 * @param handle
 *    
 * @return (error code)
 *    -	0: ok
 *     	-1: fail
 *		...
 */
int joshvm_esp32_media_reset(joshvm_media_t* handle);

/**
 * @brief release 
 *
 * @note 
 *
 * @param handle
 *[out]	  state
 *    		- 1:stop
 *			  2:pause
 *			  3:playing/recording
 * @return (error code)
 *    -	0: ok
 *     	-1: fail
 *		...
 */
 enum{
 	JOSHVM_MEDIA_RESERVE = 0,//after play/record
	JOSHVM_MEDIA_STOPPED = 1,
	JOSHVM_MEDIA_PAUSED = 2,
	JOSHVM_MEDIA_PLAYING = 3,
	JOSHVM_MEDIA_RECORDING = 3
 }media_state_e;
int joshvm_esp32_media_get_state(joshvm_media_t* handle, int* state);

/**
 * @brief read 
 *
 * @note 
 *
 * @param 	handle
 *			buffer:read address
 *			size:the most number of bytes to read 
 *			byteRead:number of bytes to read
 *			callback:
 * @return (error code)
 *    -	0: ok
 *     	-1: fail
 *		...
 */
int joshvm_esp32_media_read(joshvm_media_t* handle, unsigned char* buffer, int size, int* bytesRead, void(*callback)(void*, int));

/**
 * @brief  write
 *
 * @note 
 *
 * @param 	handle
 *			buffer:write address
 *			size:the most number of bytes to writed
 *			byteWritten:number of bytes to writed
 *			callback:
 * @return (error code)
 *    -	0: ok
 *     	-1: fail
 *		...
 */
int joshvm_esp32_media_write(joshvm_media_t* handle, unsigned char* buffer, int size, int* bytesWritten, void(*callback)(void*, int));

/**
 * @brief  clear buffer,keep the state of paused / stopped 
 *
 * @note 
 *
 * @param 	handle
 *
 * @return (error code)
 *    -	0: ok
 *     	-1: fail
 *		...
 */
int joshvm_esp32_media_flush(joshvm_media_t* handle);

/**
 * @brief   
 *
 * @note 
 *
 * @param 	handle
 *			size: the size of the buffer can be return
 *
 * @return (error code)
 *    -	0: ok
 *     	-1: fail
 *		...
 */
int joshvm_esp32_media_get_buffsize(joshvm_media_t* handle, int* size);

/**
 * @brief   
 *
 * @note 
 *
 * @param 	handle
 *			value:set sample rate
 *
 * @return (error code)
 *    -	0: ok
 *     	-1: fail
 *		...
 */
int joshvm_esp32_media_set_audio_sample_rate(joshvm_media_t* handle, uint32_t value);
int joshvm_esp32_media_set_channel_config(joshvm_media_t* handle, uint8_t value);
int joshvm_esp32_media_set_audio_bit_rate(joshvm_media_t* handle, uint8_t value);

/**
 * @brief   
 *
 * @note 
 *
 * @param 	handle
 *			sourse:Such as "file://sdcard/test.wav" or "http://iot.espressif.com/file/example.mp3".
 *               If NULL to be set, the uri setup by`esp_audio_setup` will used.
 *
 * @return (error code)
 *    -	0: ok
 *     	-1: fail
 *		...
 */
int joshvm_esp32_media_set_source(joshvm_media_t* handle, char* source);

/**
 * @brief   
 *
 * @note 
 *
 * @param 	handle:
 *			file:file name and file path,Such as "file://sdcard/test.wav"
 *
 * @return (error code)
 *    -	0: ok
 *     	-1: fail
 *		...
 */
int joshvm_esp32_media_set_output_file(joshvm_media_t* handle, char* file);

/**
 * @brief   
 *
 * @note 
 *
 * @param 	handle:
 *			format:output format
 *
 * @return (error code)
 *    -	0: ok
 *     	-1: fail
 *		...
 */
int joshvm_esp32_media_set_output_format(joshvm_media_t* handle, int format);

/**
 * @brief   
 *
 * @note 
 *
 * @param 	handle:
 *			pos:current position
 *
 * @return (error code)
 *    -	0: ok
 *     	-1: fail
 *		...
 */
int joshvm_esp32_media_get_position(joshvm_media_t* handle, int* pos);

/**
 * @brief   
 *
 * @note 
 *
 * @param 	handle:
 *			pos:current position
 *			callback:
 * @return (error code)
 *    -	0: ok
 *     	-1: fail
 *		...
 */
int joshvm_esp32_media_set_position(joshvm_media_t* handle, int pos, void(*callback)(void*, int));

/**
 * @brief   
 *
 * @note 
 *
 * @param 	handle:
 *			duration : get tatol time of media (ms)
 * @return (error code)
 *    -	0: ok
 *     	-1: fail
 *		...
 */
int joshvm_esp32_media_get_duration(joshvm_media_t* handle, int* duration);

int joshvm_esp32_media_get_max_volume(int* volume);
int joshvm_esp32_media_get_volume(int* volume);
int joshvm_esp32_media_set_volume(int volume);
int joshvm_esp32_media_add_volume();
int joshvm_esp32_media_sub_volume();



#endif

