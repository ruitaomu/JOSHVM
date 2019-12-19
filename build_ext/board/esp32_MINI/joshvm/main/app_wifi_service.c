#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "audio_mem.h"
#include "app_wifi_service.h"
#include "wifi_service.h"
#include "airkiss_config.h"
#include "smart_config.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "joshvm_esp32_media.h"
#include "esp_spiffs.h"

static const char *TAG              = "APP_WIFI_SERVICE";

#define SSID_LENGTH	32
#define PASSWD_LENGTH 64
#define	PROPERTY_CFG	1
#define LAST_AIRKISS_CFG 2
#define	AIRKISS_CFG	3

//---enum
typedef enum{
	UNINITIALIZED = 0,
	DISCONNECTED,
	CONNECTING,
	CONNECTED,
	AIRKISS_SETTING
}app_wifi_state_e;

//---struct
typedef struct{
	char property_ssid[SSID_LENGTH];
	char property_password[PASSWD_LENGTH];
}app_wifi_propsta_config_t;

typedef struct{
	char *ssid;
	char *passwd;
}app_wifi_aksta_config_t;

static periph_service_handle_t wifi_serv = NULL;
static app_wifi_aksta_config_t *ak_sta = NULL;
static app_wifi_propsta_config_t *app_wifi_config = NULL;
static int8_t app_wifi_ak_profile_saveflag = UNINITIALIZED;
uint8_t app_wifi_state = UNINITIALIZED;
QueueHandle_t app_wifi_serv_queue = NULL;


static void wifi_config_init()
{
	app_wifi_config = (app_wifi_propsta_config_t*)audio_malloc(sizeof(app_wifi_propsta_config_t));
	memset(app_wifi_config->property_ssid,'\0',sizeof(app_wifi_config->property_ssid));
	memset(app_wifi_config->property_password,'\0',sizeof(app_wifi_config->property_password));
	
	ak_sta = (app_wifi_aksta_config_t *)audio_malloc(sizeof(app_wifi_aksta_config_t));
	ak_sta->ssid = (char *)audio_malloc(SSID_LENGTH * sizeof(char));
	ak_sta->passwd = (char *)audio_malloc(PASSWD_LENGTH * sizeof(char)); 	
}

static void wifi_config_deinit()
{
	audio_free(app_wifi_config);
	audio_free(ak_sta);
	audio_free(ak_sta->ssid);
	audio_free(ak_sta->passwd);
}

void app_wifi_airkissprofile_write(char *ssid,char *passwd)
{/*
	size_t total = 0, used = 0;
    int ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
*/
	ESP_LOGI(TAG, "Opening airkiss_profile");
    FILE* f = fopen("/appdb/unsecure/airkiss_profile.txt", "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }

    fprintf(f,"SSID:%s\n", ssid);
	fprintf(f,"PASSWD:%s\n", passwd);	
    fclose(f);
    ESP_LOGI(TAG, "airkiss_profile written");
}

static joshvm_err_t app_wifi_airkissprofile_read()
{
	// Open renamed file for reading
	ESP_LOGI(TAG, "Reading airkiss profile");
	FILE* f = fopen("/appdb/unsecure/airkiss_profile.txt", "r");
	if (f == NULL) {
		ESP_LOGE(TAG, "Failed to open airkiss profile for reading");
		return JOSHVM_FAIL;
	}
	
	fgets(ak_sta->ssid, SSID_LENGTH, f);	
	fgets(ak_sta->passwd, PASSWD_LENGTH, f);
	fclose(f);

	// strip newline
	char* pos = strchr(ak_sta->ssid, ':');
	ak_sta->ssid = pos+1;
	pos = strchr(ak_sta->ssid, '\n');
	if (pos) {
		*pos = '\0';
	}
	
	// strip newline
	pos = strchr(ak_sta->passwd, ':');
	ak_sta->passwd = pos+1;
	pos = strchr(ak_sta->passwd, '\n');
	if (pos) {
		*pos = '\0';
	}
	printf("Read from airkiss profile,ssid:'%s',passwd:'%s'\n",ak_sta->ssid, ak_sta->passwd);
	return JOSHVM_OK;
}

void app_wifi_get_airkisscfg(char *ssid,char *pwd)
{
	app_wifi_airkissprofile_write(ssid,pwd);	
}

static joshvm_err_t app_wifi_airkiss_cfg_connect()
{
	printf("wifi connecting with airkiss profile SSID and PASSWD.\n");
	if(app_wifi_airkissprofile_read() != JOSHVM_OK){
		return JOSHVM_FAIL;
	}
	wifi_config_t sta_cfg = {0};
	strncpy((char *)&sta_cfg.sta.ssid,ak_sta->ssid, strlen(ak_sta->ssid));
	strncpy((char *)&sta_cfg.sta.password,ak_sta->passwd, strlen(ak_sta->passwd));
	if(wifi_service_set_sta_info(wifi_serv, &sta_cfg) != ESP_OK){
		return JOSHVM_FAIL;
	}
	return JOSHVM_OK;
}

static joshvm_err_t app_wifi_property_cfg_connect()
{
	printf("wifi connecting with property SSID and PASSWD.\n");
	if((strlen(app_wifi_config->property_ssid) == 0)  || (strlen(app_wifi_config->property_password)) == 0){
		ESP_LOGE(TAG,"Property file's ssid or passwd is null!");
		return JOSHVM_FAIL;
	}

	wifi_config_t sta_cfg = {0};
	strncpy((char *)&sta_cfg.sta.ssid,(const char*)(app_wifi_config->property_ssid), strlen(app_wifi_config->property_ssid));
	strncpy((char *)&sta_cfg.sta.password,(const char*)(app_wifi_config->property_password), strlen(app_wifi_config->property_password));
	if(wifi_service_set_sta_info(wifi_serv, &sta_cfg) != ESP_OK){
		return JOSHVM_FAIL;
	}
	return JOSHVM_OK;
}

static void app_wifi_airkiss_enable()
{
	printf("enable airkiss.\n");
	app_wifi_state = AIRKISS_SETTING;
	app_wifi_ak_profile_saveflag = AIRKISS_SETTING;
	wifi_service_setting_start(wifi_serv, 0);
}

static esp_err_t app_wifi_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
    ESP_LOGD(TAG, "event type:%d,source:%p, data:%p,len:%d,ctx:%p",
             evt->type, evt->source, evt->data, evt->len, ctx);
		 
	if(evt->type == WIFI_SERV_EVENT_CONNECTING){
        ESP_LOGI(TAG, "PERIPH_WIFI_CONNECTING [%d]", __LINE__);
		app_wifi_state = CONNECTING;
	}
	else if (evt->type == WIFI_SERV_EVENT_CONNECTED) {		
        printf("PERIPH_WIFI_CONNECTED [%d]\n\n", __LINE__);
		if(app_wifi_ak_profile_saveflag == AIRKISS_SETTING){
			app_wifi_ak_profile_saveflag = UNINITIALIZED;
			wifi_config_t info;
			memset(&info, 0x00, sizeof(wifi_config_t));
			if (ESP_OK == esp_wifi_get_config(WIFI_IF_STA, &info)) {
				if (info.sta.ssid[0] != 0) {
					printf("Connect to Wi-Fi SSID:%s PWD:%s\n", info.sta.ssid,info.sta.password);
					app_wifi_get_airkisscfg((char*)info.sta.ssid,(char*)info.sta.password);
				}else{
					ESP_LOGW(TAG, "No wifi SSID stored!");
				}		
			} else {
				ESP_LOGW(TAG, "No wifi SSID stored!");
			}			
		}
	
		app_wifi_state = CONNECTED;			
    } else if (evt->type == WIFI_SERV_EVENT_DISCONNECTED) {
        ESP_LOGI(TAG, "PERIPH_WIFI_DISCONNECTED [%d]", __LINE__);
		uint32_t senddata = APP_WIFI_SERV_DISCONNECTED;
		xQueueSend(app_wifi_serv_queue,&senddata,0);

    } else if (evt->type == WIFI_SERV_EVENT_SETTING_TIMEOUT) {
		ESP_LOGI(TAG, "SWIFI_SERV_EVENT_SETTING_TIMEOUT [%d]", __LINE__);
		
    }
    return ESP_OK;
}

static void app_wifi_task(void *parameter)
{
	uint32_t r_queue = 0;
	int8_t cnt = 0;	

	while(1){
		xQueueReceive(app_wifi_serv_queue, &r_queue, portMAX_DELAY);
		switch(r_queue){
			case	APP_WIFI_SERV_DISCONNECTED:
				wifi_service_connect(wifi_serv);
				ESP_LOGI(TAG,"APP_WIFI_SERV_DISCONNECTED");
				app_wifi_state = DISCONNECTED;
			
				break;
			case	APP_WIFI_SERV_RECONNECTEDFAILED:				
				ESP_LOGI(TAG,"APP_WIFI_SERV_RECONNECTEDFAILED");
				cnt++;				
				if(cnt > 3) cnt = 1;				
				switch(cnt){
					case	PROPERTY_CFG:	
						app_wifi_state = CONNECTING;
						if(app_wifi_property_cfg_connect() == JOSHVM_OK){
							break;
						}
					//if property profile connect fail,it is need to connect airkiss profile without break case PROPERTY_CFG
						cnt++;
					case	LAST_AIRKISS_CFG:
						app_wifi_state = CONNECTING;
						if(app_wifi_airkiss_cfg_connect() == JOSHVM_FAIL){
							app_wifi_airkiss_enable();
						}						
					break;
					case 	AIRKISS_CFG:
						app_wifi_airkiss_enable();
					default:

					break;
				}	
				break;
			default:
				break;
		}
	}
}


void app_wifi_service(void)
{
	wifi_config_init();
/*	wifi_config_t sta_cfg = {0};
	strncpy((char *)&sta_cfg.sta.ssid,"JOSH", strlen("JOSH"));
	strncpy((char *)&sta_cfg.sta.password,"josh20177", strlen("josh20179"));
*/
	app_wifi_serv_queue = xQueueCreate(3, sizeof(uint32_t));
	if(NULL == app_wifi_serv_queue){
		ESP_LOGE(TAG,"app_wifi_ser_queue created failed");
	}

	wifi_service_config_t cfg = WIFI_SERVICE_DEFAULT_CONFIG();
	cfg.evt_cb = app_wifi_service_cb;
	cfg.cb_ctx = NULL;
	cfg.setting_timeout_s = 0x7FFFFFFF;
	wifi_serv = wifi_service_create(&cfg);

	int reg_idx = 0;
	esp_wifi_setting_handle_t h = NULL;
	airkiss_config_info_t air_info = AIRKISS_CONFIG_INFO_DEFAULT();
	air_info.lan_pack.appid = CONFIG_AIRKISS_APPID;
	air_info.lan_pack.deviceid = CONFIG_AIRKISS_DEVICEID;
	air_info.aes_key = CONFIG_DUER_AIRKISS_KEY;
	h = airkiss_config_create(&air_info);
	esp_wifi_setting_regitster_notify_handle(h, (void *)wifi_serv);
	wifi_service_register_setting_handle(wifi_serv, h, &reg_idx);
	//wifi_service_set_sta_info(wifi_serv, &sta_cfg);
	
	xTaskCreate(app_wifi_task,"app_wifi_task",3*1024,NULL,5,NULL);	
}

joshvm_err_t joshvm_esp32_wifi_set(char* ssid, char* password, int force)
{
	int ret = JOSHVM_FAIL;
	if((strlen(ssid) == 0)  || (strlen(password)) == 0){
		ESP_LOGE(TAG,"Property file's ssid or passwd is null!");
		return JOSHVM_FAIL;
	}

	strncpy(app_wifi_config->property_ssid,ssid,strlen(ssid));
	strncpy(app_wifi_config->property_password ,password,strlen(password));
	
	if(force == false){
		ESP_LOGI(TAG,"if saved ssid pwd can't connect,property cfg will be set and then connecting wifi!");
		return JOSHVM_NOTIFY_LATER; 
	}
	ESP_LOGI(TAG,"joshvm_esp32_wifi_set!");
	wifi_service_setting_stop(wifi_serv,0);
	wifi_config_t sta_cfg = {0};
	strncpy((char *)&sta_cfg.sta.ssid,ssid, strlen(ssid));
	strncpy((char *)&sta_cfg.sta.password,password, strlen(password));	
	ret = wifi_service_set_sta_info(wifi_serv, &sta_cfg);
		
	if(ret == ESP_OK){
		return JOSHVM_OK;
	}
	return JOSHVM_FAIL;
}

joshvm_err_t joshvm_esp32_wifi_get_state(int* state)
{
	*state = app_wifi_state;
	printf("wifi_state :%d\n",app_wifi_state);
	return JOSHVM_OK;
}


