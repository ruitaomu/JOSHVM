#include <javacall_gpio.h>
#include <javacall_logging.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>

#define BUFFLEN	 256
#define PINS_MAX 50

typedef struct pin_handle {
	int isOpen;
	int fd;
	int pin;	
	int notify_enabled;
	int pthread_created;
}pin_handle;

static pin_handle pinsList[PINS_MAX];

void javacall_gpio_init(void) {
	memset(pinsList, 0, sizeof(pinsList));
}

void javacall_gpio_deinit(void) {
	int i;
	for (i=0; i<PINS_MAX; i++) {
		if (pinsList[i].isOpen != 0) {
			javacall_gpio_pin_close(&pinsList[i]);
        }
	}
}

javacall_dio_result javacall_gpio_pin_open(const javacall_int32 port,
        const javacall_int32 pin, const javacall_gpio_dir direction,
        const javacall_gpio_mode mode, const javacall_gpio_trigger_mode trigger,
        const javacall_bool initValue,
        const javacall_bool exclusive,
        /*OUT*/ javacall_handle* pHandle) {
        
    char gp[8] = {0};
	char path[BUFFLEN];
	char *cmd;
	int i;

	if (JAVACALL_TRUE != exclusive) {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI,  "NOT exclusive\n");
		return JAVACALL_DIO_UNSUPPORTED_ACCESS_MODE;
	}

	for (i=0; i<PINS_MAX; i++) {
		if (pinsList[i].isOpen == 0) {
			break;
        }
	}

	if (i >= PINS_MAX) {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "Too many pins open\n");
		return JAVACALL_DIO_FAIL;
	}

	/** Enable GPIO Pin **/
	
	int fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd == -1) {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "open export dir fail\n");
		return JAVACALL_DIO_FAIL;
	}	
	javautil_snprintf(gp, 8, "%d", pin);
	write(fd, gp, strlen(gp));		
	close(fd);

	/** Set direction **/
	
	if (direction == JAVACALL_GPIO_INPUT_MODE) {
		cmd = "in";
	} else if (direction == JAVACALL_GPIO_OUTPUT_MODE) {
		if (initValue == JAVACALL_TRUE) {
			cmd = "high";
		} else {
			cmd = "low";
		}		
	} else {
		javacall_logging_printf(JAVACALL_LOGGING_WARNING, JC_DAAPI, "invalid config");
		return JAVACALL_DIO_INVALID_CONFIG;
	}
	
	javautil_snprintf(path, BUFFLEN, "/sys/class/gpio/gpio%d/direction", pin);		
	fd = open(path, O_WRONLY);		
	if (fd == -1) {	
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "set %d pin direction error\n", pin);
		return JAVACALL_DIO_FAIL;
	}
	write(fd, cmd, strlen(cmd));
	close(fd);

	/** Set trigger **/	
		
	if ((trigger == JAVACALL_TRIGGER_FALLING_EDGE) || (trigger == JAVACALL_TRIGGER_LOW_LEVEL)) {
		cmd = "falling";
	} else if ((trigger == JAVACALL_TRIGGER_RISING_EDGE) || (trigger == JAVACALL_TRIGGER_HIGH_LEVEL)) {
		cmd = "rising";
	} else if ((trigger == JAVACALL_TRIGGER_BOTH_EDGES) || (trigger == JAVACALL_TRIGGER_BOTH_LEVELS)) {
		cmd = "both";
	} else if (trigger == JAVACALL_TRIGGER_NONE) {
		cmd = "none";
	} else {
		cmd = NULL;
	}
	
	if (cmd == NULL) {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "invalid trigger\n");
		return JAVACALL_DIO_INVALID_CONFIG;
	}

	javautil_snprintf(path, BUFFLEN, "/sys/class/gpio/gpio%d/edge", pin);
	fd = open(path, O_WRONLY);		
	if(fd == -1){		
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "set %d pin edge error\n", pin);
		return JAVACALL_DIO_FAIL;
	}
	write(fd, cmd, strlen(cmd));		
	close(fd);

	/** Open value for r/w and holds the fd in handle **/

	javautil_snprintf(path, BUFFLEN, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_RDWR);
	if(fd == -1){
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "open value of %d error\n", pin);
		return JAVACALL_DIO_FAIL;
	}

	pinsList[i].isOpen = 1;
	pinsList[i].fd = fd;
	pinsList[i].pin = pin;
	pinsList[i].notify_enabled = 0;
	pinsList[i].pthread_created = 0;

	*pHandle = (javacall_handle)&(pinsList[i]);

	return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_close(javacall_handle handle) {
	char gp[8] = {0};
	javacall_dio_result ret = JAVACALL_DIO_FAIL;

	pin_handle* pinHandle = (pin_handle*)handle;
	
	javacall_gpio_pin_notification_stop(handle);

	int fd = pinHandle->fd;

	pinHandle ->isOpen = 0;
	pinHandle->fd = -1;
	pinHandle->pin = -1;
	pinHandle->pthread_created = 0;
	
	close(fd);
	
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (fd >= 0) {
		ret = JAVACALL_DIO_OK;
		javautil_snprintf(gp, 8, "%d", pinHandle->pin);
		write(fd, gp, strlen(gp));		
		close(fd);
	}	
	
	return ret;
}

javacall_dio_result javacall_gpio_pin_write(const javacall_handle handle, 
        const javacall_bool val)
{
	static const unsigned char h = '1';
	static const unsigned char l = '0';
	pin_handle* pinHandle = (pin_handle*)handle;
	
	if (write(pinHandle->fd, JAVACALL_TRUE==val?&h:&l, 1) != 1) {
		return JAVACALL_DIO_FAIL;
	} else {
		return JAVACALL_DIO_OK;
	}
}

javacall_dio_result javacall_gpio_pin_read(const javacall_handle handle,
        /*OUT*/javacall_bool* pVal)
{
	static char buf[2];
	int res;
	int fd;
	pin_handle* pinHandle = (pin_handle*)handle;

	fd = pinHandle->fd;

	lseek(fd, 0, SEEK_SET);
	res = read(fd, buf, 2);
	if(res == 0) {
		javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_DAAPI, "javacall_gpio_pin_read busy\n");
		return JAVACALL_DIO_BUSY;
	} else if(res == -1){ 
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "javacall_gpio_pin_read fail\n");
		return JAVACALL_DIO_FAIL;
	}                
	buf[1] = '\0';                
	//javacall_printf("read gpio-%d res:%d, value:%s\n", pinHandle->pin, res, buf);                
	if (res != 2) {                    
		return JAVACALL_DIO_FAIL;               
	} else {
		if (buf[0] == '0') {
			*pVal = JAVACALL_FALSE;
		} else {
			*pVal = JAVACALL_TRUE;
		}
		return JAVACALL_DIO_OK;
	}
}

javacall_dio_result javacall_gpio_pin_direction_set(const javacall_handle handle, const javacall_bool direction) {
	/** Set direction **/
	pin_handle* pinHandle = (pin_handle*)handle;
	int fd;
	char path[BUFFLEN];
	char *cmd;
	int pin = pinHandle->pin;
	
	if (direction == JAVACALL_GPIO_INPUT_MODE) {
		cmd = "in";
	} else if (direction == JAVACALL_GPIO_OUTPUT_MODE) {
		cmd = "out";
	} else {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "invalid config");
		return JAVACALL_DIO_INVALID_CONFIG;
	}
	
	javautil_snprintf(path, BUFFLEN, "/sys/class/gpio/gpio%d/direction", pin);		
	fd = open(path, O_WRONLY);		
	if (fd == -1) {	
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "set %d pin direction error\n", pin);
		return JAVACALL_DIO_FAIL;
	}
	write(fd, cmd, strlen(cmd));
	close(fd);
	return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_set_trigger(const javacall_handle handle, const javacall_gpio_trigger_mode trigger) {
	/** Set trigger **/	
	pin_handle* pinHandle = (pin_handle*)handle;
	int fd;
	char path[BUFFLEN];
	char *cmd;
	int pin = pinHandle->pin;
		
	if ((trigger == JAVACALL_TRIGGER_FALLING_EDGE) || (trigger == JAVACALL_TRIGGER_LOW_LEVEL)) {
		cmd = "falling";
	} else if ((trigger == JAVACALL_TRIGGER_RISING_EDGE) || (trigger == JAVACALL_TRIGGER_HIGH_LEVEL)) {
		cmd = "rising";
	} else if ((trigger == JAVACALL_TRIGGER_BOTH_EDGES) || (trigger == JAVACALL_TRIGGER_BOTH_LEVELS)) {
		cmd = "both";
	} else if (trigger == JAVACALL_TRIGGER_NONE) {
		cmd = "none";
	} else {
		cmd = NULL;
	}
	
	if (cmd == NULL) {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "invalid trigger\n");
		return JAVACALL_DIO_INVALID_CONFIG;
	}

	javautil_snprintf(path, BUFFLEN, "/sys/class/gpio/gpio%d/edge", pin);
	fd = open(path, O_WRONLY);		
	if(fd == -1){		
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "set %d pin edge error\n", pin);
		return JAVACALL_DIO_FAIL;
	}
	write(fd, cmd, strlen(cmd));		
	close(fd);
	return JAVACALL_DIO_OK;
}

static void *gpio_thrd_func(void *arg){	
	int gpio_thread_stop = 0;
	pin_handle* handle = (pin_handle*)arg;
	javacall_bool val;
	int fd = handle->fd;
	fd_set exceptfds;
	int r;
	
	while (handle->isOpen) {
		FD_ZERO(&exceptfds);
		FD_SET(fd, &exceptfds);
		r = select(fd+1, NULL, NULL, &exceptfds, NULL);
		if (handle->notify_enabled && (r > 0) && FD_ISSET(fd, &exceptfds)) {
			lseek(fd, 0, SEEK_SET);
			if (JAVACALL_DIO_OK == javacall_gpio_pin_read((javacall_handle)handle, &val)) {
				javanotify_gpio_pin_value_changed((javacall_handle)handle, val?1:0);
			}
		}		
	}
}

static javacall_dio_result start_gpio_listening_thread(const javacall_handle handle)
{
	pthread_attr_t attr;
	pthread_t pid;
	
	if (pthread_attr_init(&attr) != 0) {
		return JAVACALL_DIO_FAIL;
	} else {
		if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
			pthread_attr_destroy(&attr);
			return JAVACALL_DIO_FAIL;
		}
	}
	
	if (pthread_create(&pid, &attr, gpio_thrd_func, handle)!=0) {
	  return JAVACALL_DIO_FAIL;
	}
	
	((pin_handle*)handle)->pthread_created = 1;
	
	return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_notification_start(const javacall_handle handle) {
	pin_handle* pinHandle = (pin_handle*)handle;

	pinHandle->notify_enabled = 1;
	
	if (pinHandle->pthread_created == 0) {
		if (start_gpio_listening_thread(handle) != JAVACALL_DIO_OK) {
			pinHandle->notify_enabled = 0;
			return JAVACALL_DIO_FAIL;
		} else {
			return JAVACALL_DIO_OK;
		}
	} else {
		return JAVACALL_DIO_OK;
	}
}

javacall_dio_result javacall_gpio_pin_notification_stop(const javacall_handle handle)
{
	((pin_handle*)handle)->notify_enabled = 0;
	
	return JAVACALL_DIO_OK;
}

