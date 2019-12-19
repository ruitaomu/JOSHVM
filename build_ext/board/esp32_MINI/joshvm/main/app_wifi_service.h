/*
 *
 */

#ifndef _APP_WIFI_SERVICE_H_
#define _APP_WIFI_SERVICE_H_
#include "joshvm_esp32_media.h"




typedef enum{
	APP_WIFI_SERV_UNKNOWN = 0,
	APP_WIFI_SERV_CONNECTED,
	APP_WIFI_SERV_DISCONNECTED,
	APP_WIFI_SERV_RECONNECTEDFAILED,
	APP_WIFI_SERV_AIRKISS_CONNECTED

}app_wifi_serv_queue_t;

			   										
/**
 * @brief 
 *
 * @note 
 *
 * @param 
 *
 * @return
 *
 */
void app_wifi_service(void);

/**
 * @brief 
 *
 * @note 
 *
 * @param 
 *
 * @return
 *
 */
joshvm_err_t joshvm_esp32_wifi_set(char* ssid, char* password, int force);

/**
 * @brief 
 *
 * @note 
 *
 * @param [out] state
 *				0 - UNINITIALIZED
 *				1 - DISCONNECTED
 *				2 - CONNECTING
 *				3 - CONNECTED
 *				4 - AIRKISS SETTING
 *
 * @return err code
 *
 */
joshvm_err_t joshvm_esp32_wifi_get_state(int* state);



#endif
