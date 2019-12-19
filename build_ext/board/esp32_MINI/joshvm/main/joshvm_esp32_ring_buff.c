#include <string.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "audio_mem.h"
#include "joshvm_esp32_ring_buff.h"

#define TAG "JOSHVM_ESP32_RAW_BUFF"

void ring_buffer_init(ring_buffer_t *rb, int32_t buff_size)
{
    rb->buffer = (int8_t*)audio_malloc(buff_size);
    memset(rb->buffer, 0, buff_size);

    rb->read_offset = 0;
    rb->write_offset = 0;
    rb->valid_size = 0;
    rb->total_size = buff_size;
	rb->init_read_pointer = NOT_INTI;
}

void ring_buffer_deinit(ring_buffer_t *rb)
{
    if (rb->buffer != NULL){
        audio_free(rb->buffer);
		rb->buffer = NULL;
    }
    memset(rb, 0, sizeof(ring_buffer_t));
}

void ring_buffer_flush(ring_buffer_t *rb)
{
    rb->read_offset = rb->write_offset;    
    rb->valid_size = 0;
	rb->init_read_pointer = NOT_INTI;
}

int32_t ring_buffer_write(void *buffer_to_write, int32_t size, ring_buffer_t *rb,uint8_t cover_type)
{
	//ESP_LOGI(TAG,"ring_buffer_write");
    int32_t write_offset = rb->write_offset;
    int32_t total_size = rb->total_size;
    int32_t first_write_size = 0;
	int32_t written_size = 0;

	if((rb == NULL) || (rb->buffer == NULL)){
		ESP_LOGE(TAG,"ringbuffer is NULL![%s][%d]",__FILE__,__LINE__);
		return -1;
	}

	if(size < 0){
		ESP_LOGE(TAG,"write ringbuffer size is err![%s][%d]",__FILE__,__LINE__);
		return -1;
	}

	if(size + rb->valid_size > total_size){
		if(RB_NOT_COVER == cover_type){
			return 0;//written size is 0 byte
		}
		rb->init_read_pointer = NEED_INIT;	
	}

    if (size + write_offset <= total_size){
        memcpy(rb->buffer + write_offset, buffer_to_write, size);
		written_size = size;
    }
    else //ring_buffer->buffer的后段未写入的空间小于size,这时候需要先在后面写入一部分，然后返回头部，从前面接着写入
    {
        first_write_size = total_size - write_offset;
        memcpy(rb->buffer + write_offset, buffer_to_write, first_write_size);
        memcpy(rb->buffer, buffer_to_write + first_write_size, size - first_write_size);
		written_size = size;
    }
	
    rb->write_offset += size;
    rb->write_offset %= total_size;
    rb->valid_size += size;
	//adjust
	if(rb->init_read_pointer == NEED_INIT){
		rb->init_read_pointer = NOT_INTI;
		rb->read_offset = rb->write_offset;
		rb->valid_size = rb->total_size;	
	}
	//ESP_LOGI(TAG,"write rb valid_size = %d\n",rb->valid_size);
	return written_size;
}

int32_t ring_buffer_read(void *buff, int32_t size,ring_buffer_t *rb)
{
    int32_t read_offset = rb->read_offset;
	//int32_t write_offset = rb->write_offset;
    int32_t total_size = rb->total_size;
    int32_t first_read_size = 0;
	uint32_t read_size = 0;//real size of data have been read

	if((rb == NULL) || (rb->buffer == NULL)){
		ESP_LOGE(TAG,"ringbuffer is NULL![%s][%d]",__FILE__,__LINE__);
		return -1;
	}

	if(size < 0){
		ESP_LOGE(TAG,"read ringbuffer size is err![%s][%d]",__FILE__,__LINE__);
		return -1;
	}
	
    if (size > rb->valid_size){
		size = rb->valid_size;	
    }

    if (total_size - read_offset >= size){
        memcpy(buff, rb->buffer + read_offset, size);
    }
    else{
        first_read_size = total_size - read_offset;
        memcpy(buff, rb->buffer + read_offset, first_read_size);
        memcpy(buff + first_read_size, rb->buffer, size - first_read_size);
    }
	read_size = size;

    rb->read_offset += size;
    rb->read_offset %= total_size;
    rb->valid_size -= size;
	//printf("read rb valid_size = %d\n",rb->valid_size);
	return read_size;
}

void joshvm_rb_test(int8_t *buff,uint8_t size)
{
	for(int i=0;i<size;i++){
		printf("%x",*(buff+i));		
	}
	printf("\r\n");	
}
