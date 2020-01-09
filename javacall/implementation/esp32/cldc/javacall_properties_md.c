#include <javacall_defs.h>
/*
 * /appdb/unsecure/properties.ini
 */
short property_file_name[JAVACALL_MAX_FILE_NAME_LENGTH+1] = {'/','a','p','p','d','b','/',
																'u','n','s','e','c','u','r','e','/',
																'p','r','o','p','e','r','t','i','e','s','.','i','n','i','.','j','a','r',0};

#ifdef ENABLE_DYNAMIC_PROP

#define JC_PROP_BUFLEN (64)

#if USE_ESP_MINI || USE_JOSH_EVB
extern int joshvm_esp32_wifi_get_state(int* state);
extern int joshvm_esp32_get_sys_info(char* info, int size);
#endif
javacall_result javacall_get_dynamic_property(const char* key, char** result) {
	static char buff[JC_PROP_BUFLEN + 1] = {0};

	if (key == NULL) {
		return JAVACALL_FAIL;
	}

	if (!strcmp(key, "wifi.state")) {
		int state = 0;
		#if USE_ESP_MINI || USE_JOSH_EVB
		if (joshvm_esp32_wifi_get_state(&state) != 0) {
			// unknown
			state = 99;
		}
		#endif
		snprintf(buff, JC_PROP_BUFLEN, "%d", state);
	} else if (!strcmp(key, "system.info")) {
		buff[0] = 0;
		#if USE_ESP_MINI || USE_JOSH_EVB
		if (joshvm_esp32_get_sys_info(buff, JC_PROP_BUFLEN) != 0) {
			buff[0] = 0;
		}
		#endif
	} else if (!strcmp(key, "microedition.commports")) {
		snprintf(buff, JC_PROP_BUFLEN, "COM0,COM1,COM2");
	} else {
		return JAVACALL_FAIL;
	}

	*result = buff;
	return JAVACALL_OK;
}
#endif
