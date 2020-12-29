#include <string.h>
#include "joshvm_esp32_record.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_mem.h"
#include "i2s_stream.h"
#include "filter_resample.h"
#include "fatfs_stream.h"
#include "spiffs_stream.h"
#include "http_stream.h"
#include "raw_stream.h"
#include "wav_encoder.h"
#include "amrnb_encoder.h"
#include "amrwb_encoder.h"
#include "opus_encoder.h"
#include "esp_log.h"

//---define
#define TAG  "JSOHVM_ESP32_RECORDER"
//16000/1000*30ms
#define VOICEBUFF_SIZE 480

//---creater cfg 
#define RECORD_RATE         48000
#define RECORD_CHANNEL      2
#define RECORD_BITS         16

#define SAVE_FILE_RATE      handle->j_union.mediaRecorder.sample_rate	
#define SAVE_FILE_CHANNEL   handle->j_union.mediaRecorder.channel
#define SAVE_FILE_BITS      handle->j_union.mediaRecorder.bit_rate

#define PLAYBACK_RATE       48000
#define PLAYBACK_CHANNEL    2
#define PLAYBACK_BITS       16

//---element cfg
#define M_REC_CFG_FORMAT	handle->j_union.mediaRecorder.format
#define M_REC_CFG_URL		handle->j_union.mediaRecorder.url
#define M_REC_CFG_RATE 		handle->j_union.mediaRecorder.sample_rate
#define M_REC_CFG_CHANNEL 	handle->j_union.mediaRecorder.channel
#define M_REC_CFG_BITS		handle->j_union.mediaRecorder.bit_rate

#define A_RECORD_RATE 		handle->j_union.audioRecorder.sample_rate
#define A_RECORD_CHA 		handle->j_union.audioRecorder.channel
#define A_RECORD_BITS		handle->j_union.audioRecorder.bit_rate

#define	A_TRACK_RATE		handle->j_union.audioTrack.sample_rate
#define	A_TRACK_CHA			handle->j_union.audioTrack.channel
#define A_TRACK_BITS		handle->j_union.audioTrack.bit_rate

#define TRACK_CHENK_TIMEOUT 800
//---variable
uint16_t track_check_time_cnt = 0;
//audio_element_handle_t josh_i2s_stream_reader = NULL,josh_i2s_stream_writer = NULL;
extern TaskHandle_t audio_track_handler;

typedef struct rsp_filter {
    resample_info_t *resample_info;
    unsigned char *out_buf;
    unsigned char *in_buf;
    void *rsp_hd;
    int in_offset;
} rsp_filter_t;

static esp_err_t is_valid_rsp_filter_samplerate(int samplerate)
{
    if (samplerate < 8000
        || samplerate > 48000) {
        ESP_LOGE(TAG, "The sample rate should be within range [8000,48000], here is %d Hz", samplerate);
        return ESP_FAIL;
    }
    return ESP_OK;
}

static esp_err_t is_valid_rsp_filter_channel(int channel)
{
    if (channel != 1 && channel != 2) {
        ESP_LOGE(TAG, "The number of channels should be either 1 or 2, here is %d", channel);
        return ESP_FAIL;
    }
    return ESP_OK;
}

static esp_err_t rsp_filter_set_dest_info(audio_element_handle_t self, int dest_rate, int dest_ch)
{
    rsp_filter_t *filter = (rsp_filter_t *)audio_element_getdata(self);
    if (filter->resample_info->dest_rate == dest_rate
        && filter->resample_info->dest_ch == dest_ch) {
        return ESP_OK;
    }
    if (is_valid_rsp_filter_samplerate(dest_rate) != ESP_OK
        || is_valid_rsp_filter_channel(dest_ch) != ESP_OK) {
        return ESP_FAIL;
    } else {
        filter->resample_info->dest_rate = dest_rate;
        filter->resample_info->dest_ch = dest_ch;
        ESP_LOGI(TAG, "reset sample rate of destination data : %d, reset channel of destination data : %d",
                 filter->resample_info->dest_rate, filter->resample_info->dest_ch);
    }
    return ESP_OK;
}

audio_element_handle_t create_i2s_stream(int sample_rates, int bits, int channels, audio_stream_type_t type)
{
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
	#if (defined CONFIG_ESP_LYRAT_MINI_V1_1_BOARD) || (defined CONFIG_JOSH_EVB_MEGA_ESP32_V1_0_BOARD) 
		//printf("ESP_LYRAT_MINI_V1_1_BOARD or MEGA_ESP32_V1_0_BOARD\n");
		if(AUDIO_STREAM_READER == type){
			i2s_cfg.i2s_port = 1;	
			i2s_cfg.i2s_config.use_apll = 0;
		}		
	#endif
	i2s_cfg.i2s_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_LEVEL2 | ESP_INTR_FLAG_LEVEL3;
    i2s_cfg.type = type;
	i2s_cfg.i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    audio_element_handle_t i2s_stream = i2s_stream_init(&i2s_cfg);
    mem_assert(i2s_stream);
    audio_element_info_t i2s_info = {0};
    audio_element_getinfo(i2s_stream, &i2s_info);
    i2s_info.bits = bits;
    i2s_info.channels = channels;
    i2s_info.sample_rates = sample_rates;
    audio_element_setinfo(i2s_stream, &i2s_info);
	//printf("i2s_info %d  %d  %d\r\n ",i2s_info.sample_rates,i2s_info.channels,i2s_info.bits);
    return i2s_stream;
}

static audio_element_handle_t create_filter(int source_rate, int source_channel, int dest_rate, int dest_channel, audio_codec_type_t type)
{
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg.src_rate = source_rate;
    rsp_cfg.src_ch = source_channel;
    rsp_cfg.dest_rate = dest_rate;
    rsp_cfg.dest_ch = dest_channel;
    rsp_cfg.type = type;
    return rsp_filter_init(&rsp_cfg);
}

static audio_element_handle_t create_wav_encoder(int sample_rate,int bits,int channels)
{
	wav_encoder_cfg_t wav_cfg = DEFAULT_WAV_ENCODER_CONFIG();	
	audio_element_handle_t wav_encoder = wav_encoder_init(&wav_cfg);
	mem_assert(wav_encoder);
	audio_element_info_t wav_info = {0};
	audio_element_getinfo(wav_encoder, &wav_info);
	wav_info.sample_rates = sample_rate;
	wav_info.channels = channels;
	wav_info.bits = bits;
	audio_element_setinfo(wav_encoder, &wav_info);		
	//printf("wav_info %d  %d  %d\r\n ",wav_info.sample_rates,wav_info.channels,wav_info.bits);	
	return wav_encoder;
}

static audio_element_handle_t create_amrnb_encoder(int sample_rate,int bits,int channels)
{
    amrnb_encoder_cfg_t amrnb_cfg = DEFAULT_AMRNB_ENCODER_CONFIG();
    audio_element_handle_t amrnb_en = amrnb_encoder_init(&amrnb_cfg);
	mem_assert(amrnb_en);
	audio_element_info_t amrnb_info = {0};
	audio_element_getinfo(amrnb_en, &amrnb_info);
	amrnb_info.sample_rates = sample_rate;
	amrnb_info.channels = channels;
	amrnb_info.bits = bits;
	audio_element_setinfo(amrnb_en, &amrnb_info);		
	//printf("amrnb_info %d  %d  %d\r\n ",amrnb_info.sample_rates,amrnb_info.channels,amrnb_info.bits);	
	return amrnb_en;
}

static audio_element_handle_t create_amrwb_encoder(int sample_rate,int bits,int channels)
{
    amrwb_encoder_cfg_t amrwb_cfg = DEFAULT_AMRWB_ENCODER_CONFIG();
    audio_element_handle_t amrwb_en = amrwb_encoder_init(&amrwb_cfg);
	mem_assert(amrwb_en);
	audio_element_info_t amrwb_info = {0};
	audio_element_getinfo(amrwb_en, &amrwb_info);
	amrwb_info.sample_rates = sample_rate;
	amrwb_info.channels = channels;
	amrwb_info.bits = bits;
	audio_element_setinfo(amrwb_en, &amrwb_info);		
	//printf("amrwb_info %d  %d  %d\r\n ",amrwb_info.sample_rates,amrwb_info.channels,amrwb_info.bits);	
	return amrwb_en;
}

static audio_element_handle_t create_opus_encoder(int sample_rate,int bits,int channels)
{
	opus_encoder_cfg_t opus_cfg = DEFAULT_OPUS_ENCODER_CONFIG();	
	audio_element_handle_t opus_en = encoder_opus_init(&opus_cfg);
	mem_assert(opus_en);
	audio_element_info_t opus_info = {0};
	audio_element_getinfo(opus_en, &opus_info);
	opus_info.sample_rates = sample_rate;
	opus_info.channels = channels;
	opus_info.bits = bits;
	audio_element_setinfo(opus_en, &opus_info);		
	//printf("opus_info %d  %d  %d\r\n ",opus_info.sample_rates,opus_info.channels,opus_info.bits);	
	return opus_en;
}

static audio_element_handle_t create_fatfs_stream(int sample_rates, int bits, int channels, audio_stream_type_t type)
{
    fatfs_stream_cfg_t fatfs_cfg = FATFS_STREAM_CFG_DEFAULT();
    fatfs_cfg.type = type;
    audio_element_handle_t fatfs_stream = fatfs_stream_init(&fatfs_cfg);
    mem_assert(fatfs_stream);
    audio_element_info_t writer_info = {0};
    audio_element_getinfo(fatfs_stream, &writer_info);
    writer_info.bits = bits;
    writer_info.channels = channels;
    writer_info.sample_rates = sample_rates;
    audio_element_setinfo(fatfs_stream, &writer_info);	
    return fatfs_stream;
}

static audio_element_handle_t create_spiffs_stream(int sample_rates, int bits, int channels, audio_stream_type_t type)
{
    spiffs_stream_cfg_t spiffs_cfg = SPIFFS_STREAM_CFG_DEFAULT();
    spiffs_cfg.type = type;
    audio_element_handle_t spiffs_stream = spiffs_stream_init(&spiffs_cfg);
    mem_assert(spiffs_stream);
    audio_element_info_t writer_info = {0};
    audio_element_getinfo(spiffs_stream, &writer_info);
    writer_info.bits = bits;
    writer_info.channels = channels;
    writer_info.sample_rates = sample_rates;
    audio_element_setinfo(spiffs_stream, &writer_info);	
    return spiffs_stream;
}

/*
joshvm_err_t joshvm_esp32_i2s_create(void)
{
	if(josh_i2s_stream_reader == NULL){
		josh_i2s_stream_reader = create_i2s_stream(RECORD_RATE,RECORD_BITS,RECORD_CHANNEL,AUDIO_STREAM_READER);
		printf("read     %p\n",josh_i2s_stream_reader);
		if(josh_i2s_stream_reader == NULL) return JOSHVM_FAIL;
	}
	//if(josh_i2s_stream_writer == NULL){
		josh_i2s_stream_writer = create_i2s_stream(PLAYBACK_RATE,PLAYBACK_BITS,PLAYBACK_CHANNEL,AUDIO_STREAM_WRITER);
		printf("write     %p\n",josh_i2s_stream_writer);
		if(josh_i2s_stream_writer == NULL) return JOSHVM_FAIL;
	//}
	return JOSHVM_OK;
}

joshvm_err_t joshvm_esp32_i2s_deinit(void)
{
	if(josh_i2s_stream_reader != NULL){
		audio_element_deinit(josh_i2s_stream_reader);
		josh_i2s_stream_reader = NULL;
	}
	if(josh_i2s_stream_writer != NULL){
		audio_element_deinit(josh_i2s_stream_writer);
		josh_i2s_stream_writer = NULL;
	}
	return JOSHVM_OK;
}
*/
static const char fatfs_file[7] = "sdcard";
int joshvm_meida_recorder_init(joshvm_media_t  * handle)
{	 
    ESP_LOGI(TAG, "joshvm_meida_recorder_init");

    audio_pipeline_handle_t recorder = NULL;
	audio_element_handle_t file_writer = NULL;	
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    recorder = audio_pipeline_init(&pipeline_cfg);
    if (NULL == recorder) {
        return JOSHVM_FAIL;
    }	
	//---create i2s element
	//joshvm_esp32_i2s_create();
	audio_element_handle_t i2s_stream_reader = create_i2s_stream(RECORD_RATE,RECORD_BITS,RECORD_CHANNEL,AUDIO_STREAM_READER);
	//---create resample_filter
	audio_element_handle_t filter = create_filter(RECORD_RATE,RECORD_CHANNEL,SAVE_FILE_RATE,SAVE_FILE_CHANNEL,AUDIO_CODEC_TYPE_ENCODER);
	//---create fatfs/spiffs element	
	if(strstr(handle->j_union.mediaRecorder.url,fatfs_file) != NULL){
		file_writer = create_fatfs_stream(SAVE_FILE_RATE,SAVE_FILE_BITS,SAVE_FILE_CHANNEL,AUDIO_STREAM_WRITER);	
	}else{
		printf("spiffs url %s\n",handle->j_union.mediaRecorder.url);
		file_writer = create_spiffs_stream(SAVE_FILE_RATE,SAVE_FILE_BITS,SAVE_FILE_CHANNEL,AUDIO_STREAM_WRITER);	
	}
	
	//---register
    audio_pipeline_register(recorder, i2s_stream_reader, "i2s_media_rec");
	audio_pipeline_register(recorder, filter, "resample_media_rec");
	
	//---create encoder element
	if(joshvm_meida_format_wav == handle->j_union.mediaRecorder.format){
		audio_element_handle_t wav_encoder = create_wav_encoder(SAVE_FILE_RATE,SAVE_FILE_BITS,SAVE_FILE_CHANNEL);
		audio_pipeline_register(recorder, wav_encoder, "encode_media_rec");
		handle->j_union.mediaRecorder.recorder_t.encoder = wav_encoder;
	}else if(joshvm_meida_format_amrnb == handle->j_union.mediaRecorder.format){		
		audio_element_handle_t amrnb_encoder = create_amrnb_encoder(SAVE_FILE_RATE,SAVE_FILE_BITS,SAVE_FILE_CHANNEL);		
		audio_pipeline_register(recorder, amrnb_encoder, "encode_media_rec");
		handle->j_union.mediaRecorder.recorder_t.encoder = amrnb_encoder;
	}else if(joshvm_meida_format_amrwb == handle->j_union.mediaRecorder.format){
		audio_element_handle_t amrwb_encoder = create_amrwb_encoder(SAVE_FILE_RATE,SAVE_FILE_BITS,SAVE_FILE_CHANNEL);		
		audio_pipeline_register(recorder, amrwb_encoder, "encode_media_rec");
		handle->j_union.mediaRecorder.recorder_t.encoder = amrwb_encoder;
	}else if(joshvm_meida_format_opus == handle->j_union.mediaRecorder.format){
		audio_element_handle_t opus_encoder = create_opus_encoder(SAVE_FILE_RATE,SAVE_FILE_BITS,SAVE_FILE_CHANNEL);
		audio_pipeline_register(recorder, opus_encoder, "encode_media_rec");
		handle->j_union.mediaRecorder.recorder_t.encoder = opus_encoder; 
	}
	if(strstr(handle->j_union.mediaRecorder.url,fatfs_file) != NULL){
		audio_pipeline_register(recorder, file_writer, "file_media_rec");
	}else{
		audio_pipeline_register(recorder, file_writer, "file_media_rec");
	}
	audio_pipeline_link(recorder, (const char *[]) {"i2s_media_rec","resample_media_rec","encode_media_rec","file_media_rec"}, 4);
	audio_element_set_uri(file_writer,handle->j_union.mediaRecorder.url);
	ESP_LOGI(TAG,"Set default url:%s",handle->j_union.mediaRecorder.url);	
    ESP_LOGI(TAG, "Recorder has been created");
	handle->j_union.mediaRecorder.recorder_t.i2s = i2s_stream_reader;
	handle->j_union.mediaRecorder.recorder_t.filter = filter;
	handle->j_union.mediaRecorder.recorder_t.stream_writer = file_writer;
	handle->j_union.mediaRecorder.recorder_t.pipeline = recorder;
	
    return JOSHVM_OK;
}

int joshvm_meida_recorder_formatcheck(joshvm_media_t *handle)
{	
	int url_size = strlen(handle->j_union.mediaRecorder.url);	
	switch(handle->j_union.mediaRecorder.format){
		case joshvm_meida_format_wav:
			if(strstr((handle->j_union.mediaRecorder.url + url_size - 4),".wav")\
			|| strstr((handle->j_union.mediaRecorder.url + url_size - 4),".Wav")\
			|| strstr((handle->j_union.mediaRecorder.url + url_size - 4),".WAV")\
			) return JOSHVM_OK;			
			break;
		case joshvm_meida_format_amrnb:
			if(strstr((handle->j_union.mediaRecorder.url + url_size - 6),".amrnb")\			
			|| strstr((handle->j_union.mediaRecorder.url + url_size - 6),".Amrnb")\
			|| strstr((handle->j_union.mediaRecorder.url + url_size - 6),".AMRNB")\
			|| strstr((handle->j_union.mediaRecorder.url + url_size - 6),".AMRnb")\
			|| strstr((handle->j_union.mediaRecorder.url + url_size - 4),".amr")\
			|| strstr((handle->j_union.mediaRecorder.url + url_size - 4),".Amr")\
			|| strstr((handle->j_union.mediaRecorder.url + url_size - 4),".AMR")\
			) return JOSHVM_OK;			
			break;
		case joshvm_meida_format_amrwb:
			if(strstr((handle->j_union.mediaRecorder.url + url_size - 6),".amrwb")\
			|| strstr((handle->j_union.mediaRecorder.url + url_size - 6),".Amrwb")\
			|| strstr((handle->j_union.mediaRecorder.url + url_size - 6),".AMRwb")\
			|| strstr((handle->j_union.mediaRecorder.url + url_size - 6),".AMRWB")\
			|| strstr((handle->j_union.mediaRecorder.url + url_size - 4),".amr")\
			|| strstr((handle->j_union.mediaRecorder.url + url_size - 4),".Amr")\
			|| strstr((handle->j_union.mediaRecorder.url + url_size - 4),".AMR")\
			) return JOSHVM_OK;			
			break;
		case joshvm_meida_format_opus:
			if(strstr((handle->j_union.mediaRecorder.url + url_size - 5),".opus")\
			|| strstr((handle->j_union.mediaRecorder.url + url_size - 5),".Opus")\
			|| strstr((handle->j_union.mediaRecorder.url + url_size - 5),".OPUS")\
			) return JOSHVM_OK;			
			break;
		default:
			return JOSHVM_FAIL;
		break;
	}
	return JOSHVM_FAIL;
}

int joshvm_meida_recorder_cfg(joshvm_media_t *handle)
{
	ESP_LOGI(TAG, "joshvm_meida_recorder_cfg");
	//---filter_destination cfg
	rsp_filter_set_dest_info(handle->j_union.mediaRecorder.recorder_t.filter,M_REC_CFG_RATE,M_REC_CFG_CHANNEL);
	
	//---encoder cfg 
	audio_element_info_t encoder_info = {0};
	audio_element_getinfo(handle->j_union.mediaRecorder.recorder_t.encoder, &encoder_info);
	encoder_info.sample_rates = M_REC_CFG_RATE;
	encoder_info.channels = M_REC_CFG_CHANNEL;
	encoder_info.bits = M_REC_CFG_BITS;
	audio_element_setinfo(handle->j_union.mediaRecorder.recorder_t.encoder, &encoder_info);		
	ESP_LOGI(TAG,"Prepare encoder_info %d  %d  %d",encoder_info.sample_rates,encoder_info.channels,encoder_info.bits);

	//---fatfs/spiffs cfg
	audio_element_info_t writer_info = {0};
    audio_element_getinfo(handle->j_union.mediaRecorder.recorder_t.stream_writer, &writer_info);
    writer_info.bits = M_REC_CFG_BITS;
    writer_info.channels = M_REC_CFG_CHANNEL;
    writer_info.sample_rates = M_REC_CFG_RATE;
    audio_element_setinfo(handle->j_union.mediaRecorder.recorder_t.stream_writer, &writer_info);	
	ESP_LOGI(TAG,"Prepare file_info %d  %d  %d",writer_info.sample_rates,writer_info.channels,writer_info.bits);
	int ret = audio_element_set_uri(handle->j_union.mediaRecorder.recorder_t.stream_writer,\
									handle->j_union.mediaRecorder.url);
	audio_pipeline_breakup_elements(handle->j_union.mediaRecorder.recorder_t.pipeline,\
									handle->j_union.mediaRecorder.recorder_t.stream_writer);
	audio_pipeline_relink(handle->j_union.mediaRecorder.recorder_t.pipeline, \
		(const char *[]) {"i2s_media_rec","resample_media_rec","encode_media_rec","file_media_rec"}, 4);
	ESP_LOGI(TAG,"Prepare url:%s,ret=%d",handle->j_union.mediaRecorder.url,ret);
	if(joshvm_meida_recorder_formatcheck(handle) != JOSHVM_OK){
		ESP_LOGE(TAG,"Set format does not match expanded-name!");
		return JOSHVM_FAIL;
	}
	
	return JOSHVM_OK;
}

int joshvm_media_get_state(joshvm_media_t* handle,int* state)
{
	int ret = JOSHVM_FAIL;
	switch(handle->media_type)
	{
		case MEDIA_RECORDER:
			if(handle->j_union.mediaRecorder.recorder_t.i2s == NULL){
				*state = JOSHVM_MEDIA_STOPPED;//??
				return JOSHVM_OK;
			}
			ret = audio_element_get_state(handle->j_union.mediaRecorder.recorder_t.i2s);
			break;
		case AUDIO_TRACK:
			if(handle->j_union.audioTrack.audiotrack_t.i2s == NULL){
				*state = JOSHVM_MEDIA_STOPPED;//??
				return JOSHVM_OK;
			}
			ret = audio_element_get_state(handle->j_union.audioTrack.audiotrack_t.i2s);		
			break;
		case AUDIO_RECORDER:
			if(handle->j_union.audioRecorder.audiorecorder_t.i2s == NULL){
				*state = JOSHVM_MEDIA_STOPPED;//??
				return JOSHVM_OK;
			}
			ret = audio_element_get_state(handle->j_union.audioRecorder.audiorecorder_t.i2s);
			
			break;
		default :
			ret = JOSHVM_NOT_SUPPORTED;
			break;
	}
	
	switch(ret){
		case AEL_STATE_FINISHED:
		case AEL_STATE_STOPPED:
			*state = JOSHVM_MEDIA_STOPPED;
			ret = JOSHVM_OK;
			break;
		case AEL_STATE_PAUSED:
			*state = JOSHVM_MEDIA_PAUSED;	
			ret = JOSHVM_OK;
			break;
		case AEL_STATE_RUNNING:
			*state = JOSHVM_MEDIA_RECORDING;
			ret = JOSHVM_OK;
			break;
		default:
			*state = JOSHVM_MEDIA_RESERVE;
			ret = JOSHVM_FAIL;
			break;		
	}
	return ret;	
}

int joshvm_audio_track_init(joshvm_media_t* handle)
{	
    //ESP_LOGI(TAG, "joshvm_audio_track_init");
	audio_element_handle_t raw_writer = NULL;
    audio_pipeline_handle_t audio_track = NULL;
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    audio_track = audio_pipeline_init(&pipeline_cfg);
    if (NULL == audio_track) {
        return JOSHVM_FAIL;
    }	

	//---create i2s element
	audio_element_handle_t i2s_stream_writer = create_i2s_stream(PLAYBACK_RATE,PLAYBACK_BITS,PLAYBACK_CHANNEL,AUDIO_STREAM_WRITER);
	//joshvm_esp32_i2s_create();
	//---create resample element
	audio_element_handle_t filter_sample_el = create_filter(A_TRACK_RATE,A_TRACK_CHA, PLAYBACK_RATE, PLAYBACK_CHANNEL, RESAMPLE_DECODE_MODE);
	//---create raw element
	raw_stream_cfg_t raw_cfg = RAW_STREAM_CFG_DEFAULT();
    raw_cfg.type = AUDIO_STREAM_WRITER;
    raw_writer = raw_stream_init(&raw_cfg);
 
    audio_pipeline_register(audio_track, raw_writer, "raw_audio_tra");
	audio_pipeline_register(audio_track, filter_sample_el, "upsample_audio_tra");
	audio_pipeline_register(audio_track, i2s_stream_writer, "i2s_audio_tra");
    audio_pipeline_link(audio_track, (const char *[]) {"raw_audio_tra","upsample_audio_tra", "i2s_audio_tra"}, 3);
	
    //ESP_LOGI(TAG, "track has been created");
	handle->j_union.audioTrack.audiotrack_t.i2s = i2s_stream_writer;	
	handle->j_union.audioTrack.audiotrack_t.filter = filter_sample_el;	
	handle->j_union.audioTrack.audiotrack_t.raw_writer = raw_writer;
	handle->j_union.audioTrack.audiotrack_t.pipeline = audio_track;
	ESP_LOGW(TAG,"joshvm_audio_track_init = %d",heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT));

	return audio_pipeline_run(audio_track);
}

void joshvm_audio_track_task(void* handle)
{
	QueueHandle_t que =((joshvm_media_t*)handle)->evt_que;
	uint16_t que_val = 0;
	uint8_t task_run = 1;
	int32_t read_size = 0;
	int16_t *voicebuff = (int16_t *)audio_malloc(VOICEBUFF_SIZE * sizeof(short));
	audio_element_handle_t raw_writer = ((joshvm_media_t*)handle)->j_union.audioTrack.audiotrack_t.raw_writer;
	while(task_run){
pause:	
		xQueueReceive(que, &que_val, portMAX_DELAY);
		if(que_val == QUE_TRACK_START){
			while(1){//playing	
				do{
					xQueueReceive(que, &que_val, 0);
					if(que_val == QUE_TRACK_PAUSE){
						goto pause;
					}
					read_size = ring_buffer_read(voicebuff,VOICEBUFF_SIZE * sizeof(short),((joshvm_media_t*)handle)->j_union.audioTrack.track_rb);
					if(read_size > 0){
						track_check_time_cnt = 0;//clear  time
						if(NEED_CB == ((joshvm_media_t*)handle)->j_union.audioTrack.rb_callback_flag){
							((joshvm_media_t*)handle)->j_union.audioTrack.rb_callback_flag = NO_NEED_CB;
							void(*callback)(void*, int) = ((joshvm_media_t*)handle)->j_union.audioTrack.rb_callback;
							if(callback)callback(handle,JOSHVM_OK);
						}						
						raw_stream_write(raw_writer,(char*)voicebuff,read_size);
					}else{
						//ulTaskNotifyTake( pdTRUE, portMAX_DELAY );// block when data tracked out
					}
				}while(read_size > 0);
				//get que_val	
				if(que != NULL){	
					xQueueReceive(que, &que_val, 0);
				}else{
					ESP_LOGW(TAG,"Are you close track without stop track before?");
					task_run = 0;
					break;
				}
				//stop
				if((que_val == QUE_TRACK_STOP) && (track_check_time_cnt * 200 >= TRACK_CHENK_TIMEOUT)){ //delete track_check_time_cnt
					task_run = 0;
					((joshvm_media_t*)handle)->j_union.audioTrack.status = AUDIO_FINISH;
					break;
				}
				//pause
				if(que_val == QUE_TRACK_PAUSE){
					goto pause;
				}
			}	
		}		
	}
	ESP_LOGI(TAG,"AudioTrack finish playing");
	joshvm_audio_track_release(handle);
	if(voicebuff != NULL){
		audio_free(voicebuff);
		voicebuff = NULL;
	}
	//ring_buffer_flush(audio_track_rb);
	vTaskDelete(NULL);
}

int joshvm_audio_track_write(uint8_t status,ring_buffer_t* rb, unsigned char* buffer, int size, int* bytesWritten)
{
	*bytesWritten = ring_buffer_write((int8_t*)buffer,size,rb,RB_NOT_COVER);
	if(*bytesWritten > 0){
//		if(audio_track_handler != NULL){
//		 xTaskNotifyGive( audio_track_handler );
//		}
		return JOSHVM_OK;
	}else if(*bytesWritten == 0){
		if(AUDIO_STOP == status){
			return JOSHVM_OK;				
		}else{
			return JOSHVM_NOTIFY_LATER;
		}
	}else{
		return JOSHVM_FAIL;
	}
}

int joshvm_audio_recorder_init(joshvm_media_t* handle)
{	
	audio_element_handle_t raw_reader = NULL;
    audio_pipeline_handle_t audio_recorder = NULL;
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    audio_recorder = audio_pipeline_init(&pipeline_cfg);
    if (NULL == audio_recorder) {
        return JOSHVM_FAIL;
    }	
	//---create i2s element
	audio_element_handle_t i2s_stream_reader = create_i2s_stream(RECORD_RATE,RECORD_BITS,RECORD_CHANNEL,AUDIO_STREAM_READER);
	//joshvm_esp32_i2s_create();
	//---create resample_filter
	audio_element_handle_t filter = create_filter(RECORD_RATE,RECORD_CHANNEL,A_RECORD_RATE,A_RECORD_CHA,AUDIO_CODEC_TYPE_ENCODER);
	//---create raw element
	raw_stream_cfg_t raw_cfg = RAW_STREAM_CFG_DEFAULT();
    raw_cfg.type = AUDIO_STREAM_READER;
    raw_reader = raw_stream_init(&raw_cfg);

    audio_pipeline_register(audio_recorder, i2s_stream_reader, "i2s_aduio_rec");
	audio_pipeline_register(audio_recorder, filter, "filter_aduio_rec");
    audio_pipeline_register(audio_recorder, raw_reader, "raw_aduio_rec");
    audio_pipeline_link(audio_recorder, (const char *[]) {"i2s_aduio_rec", "filter_aduio_rec","raw_aduio_rec"}, 3);
	
    //ESP_LOGI(TAG, "Recorder has been created");
	handle->j_union.audioRecorder.audiorecorder_t.i2s = i2s_stream_reader;	
	handle->j_union.audioRecorder.audiorecorder_t.filter = filter;	
	handle->j_union.audioRecorder.audiorecorder_t.raw_reader = raw_reader;
	handle->j_union.audioRecorder.audiorecorder_t.pipeline = audio_recorder;	
	
	return audio_pipeline_run(audio_recorder);
}

void joshvm_audio_recorder_task(void* handle)
{
	QueueHandle_t que = ((joshvm_media_t*)handle)->evt_que;
	int32_t actually_read = 0; 
	uint16_t que_val = 0;	
	int32_t written_size = 0;
	ring_buffer_t* audio_recorder_rb = ((joshvm_media_t*)handle)->j_union.audioRecorder.rec_rb;
	int16_t *voicebuff = (int16_t *)audio_malloc(VOICEBUFF_SIZE * sizeof(short));
	audio_element_handle_t raw_rec = ((joshvm_media_t*)handle)->j_union.audioRecorder.audiorecorder_t.raw_reader;    

	while(1){	
		actually_read = raw_stream_read(raw_rec,(char*)voicebuff,VOICEBUFF_SIZE * sizeof(short));
		written_size = ring_buffer_write(voicebuff,actually_read,audio_recorder_rb,RB_COVER);
		if((written_size > 0) && (NEED_CB == ((joshvm_media_t*)handle)->j_union.audioRecorder.rb_callback_flag)){
			((joshvm_media_t*)handle)->j_union.audioRecorder.rb_callback_flag = NO_NEED_CB;
			void(*callback)(void*, int) = ((joshvm_media_t*)handle)->j_union.audioRecorder.rb_callback;
			if (callback){
				callback(handle,JOSHVM_OK);
			}
		}
		if(que != NULL){
			xQueueReceive(que, &que_val, (portTickType)0);
		}else{
			ESP_LOGW(TAG,"Are you close recorder without stop recorder before?");
			break;
		}
		if(que_val == QUE_RECORD_STOP){
			ESP_LOGI(TAG,"break recorder task");
			break;
		}
	}
	if(voicebuff != NULL){
		audio_free(voicebuff);
		voicebuff = NULL;
	}
	vTaskDelete(NULL);
}

int joshvm_audio_recorder_read(uint8_t status,ring_buffer_t* rb,unsigned char* buffer, int size, int* bytesRead)
{
	*bytesRead = ring_buffer_read(buffer,size,rb);
	if(*bytesRead > 0){
		return JOSHVM_OK;
	}else if(*bytesRead == 0){	
		if(AUDIO_STOP == status){			
			return JOSHVM_OK;
		}else if(AUDIO_START == status){
			return JOSHVM_NOTIFY_LATER;
		}		
	}
	return JOSHVM_FAIL;	
}

void joshvm_media_recorder_release(joshvm_media_t* handle)
{
	audio_pipeline_terminate(handle->j_union.mediaRecorder.recorder_t.pipeline);
    audio_pipeline_unregister(handle->j_union.mediaRecorder.recorder_t.pipeline,\
							  handle->j_union.mediaRecorder.recorder_t.stream_writer);
    audio_pipeline_unregister(handle->j_union.mediaRecorder.recorder_t.pipeline,\
							  handle->j_union.mediaRecorder.recorder_t.encoder);
    audio_pipeline_unregister(handle->j_union.mediaRecorder.recorder_t.pipeline,\
							  handle->j_union.mediaRecorder.recorder_t.filter);
    audio_pipeline_unregister(handle->j_union.mediaRecorder.recorder_t.pipeline,\
							  handle->j_union.mediaRecorder.recorder_t.i2s);
	audio_pipeline_deinit(handle->j_union.mediaRecorder.recorder_t.pipeline);
    audio_element_deinit(handle->j_union.mediaRecorder.recorder_t.stream_writer);
    audio_element_deinit(handle->j_union.mediaRecorder.recorder_t.encoder);
	audio_element_deinit(handle->j_union.mediaRecorder.recorder_t.filter);
	audio_element_deinit(handle->j_union.mediaRecorder.recorder_t.i2s);
//	if(audio_element_deinit(handle->j_union.mediaRecorder.recorder_t.i2s) == ESP_OK){
//		josh_i2s_stream_reader = NULL;	
//	}
}

void joshvm_audio_track_release(joshvm_media_t* handle)
{
	audio_pipeline_terminate(handle->j_union.audioTrack.audiotrack_t.pipeline);
    audio_pipeline_unregister(handle->j_union.audioTrack.audiotrack_t.pipeline,\
		   					  handle->j_union.audioTrack.audiotrack_t.raw_writer);
	audio_pipeline_unregister(handle->j_union.audioTrack.audiotrack_t.pipeline,\
		                      handle->j_union.audioTrack.audiotrack_t.filter);
    audio_pipeline_unregister(handle->j_union.audioTrack.audiotrack_t.pipeline,\
		                      handle->j_union.audioTrack.audiotrack_t.i2s);
    audio_pipeline_deinit(handle->j_union.audioTrack.audiotrack_t.pipeline);
    audio_element_deinit(handle->j_union.audioTrack.audiotrack_t.raw_writer);	
    audio_element_deinit(handle->j_union.audioTrack.audiotrack_t.filter);
	audio_element_deinit(handle->j_union.audioTrack.audiotrack_t.i2s);
//    if(audio_element_deinit(handle->j_union.audioTrack.audiotrack_t.i2s) == ESP_OK){
//		josh_i2s_stream_writer = NULL;
//    }
}

void joshvm_audio_rcorder_release(joshvm_media_t* handle)
{
	audio_pipeline_terminate(handle->j_union.audioRecorder.audiorecorder_t.pipeline);	
    audio_pipeline_unregister(handle->j_union.audioRecorder.audiorecorder_t.pipeline, \
		                      handle->j_union.audioRecorder.audiorecorder_t.raw_reader);	
	audio_pipeline_unregister(handle->j_union.audioRecorder.audiorecorder_t.pipeline,\
							  handle->j_union.audioRecorder.audiorecorder_t.filter);
    audio_pipeline_unregister(handle->j_union.audioRecorder.audiorecorder_t.pipeline,\
		                      handle->j_union.audioRecorder.audiorecorder_t.i2s);
    audio_pipeline_deinit(handle->j_union.audioRecorder.audiorecorder_t.pipeline);
    audio_element_deinit(handle->j_union.audioRecorder.audiorecorder_t.raw_reader);	
    audio_element_deinit(handle->j_union.audioRecorder.audiorecorder_t.filter);
	audio_element_deinit(handle->j_union.audioRecorder.audiorecorder_t.i2s);
//	if(audio_element_deinit(handle->j_union.audioRecorder.audiorecorder_t.i2s) == ESP_OK){
//		josh_i2s_stream_reader = NULL;	
//	}
}


