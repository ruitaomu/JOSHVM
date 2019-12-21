#include <javacall_gpio.h>
#include <javacall_logging.h>
#include "driver/gpio.h"

#define PINS_MAX 40
#define ESP_INTR_FLAG_DEFAULT 0

typedef struct pin_handle {
    int isOpen;
    gpio_num_t pin;
    int notify_enabled;
    int64_t last_notify_time;
} pin_handle;

static pin_handle pinsList[PINS_MAX];

#define MIN_NOTIFY_INTERVAL_US (10000)

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    javacall_handle handle = (javacall_handle) arg;
    pin_handle* pinHandle = (pin_handle*)handle;

    int level = gpio_get_level(pinHandle->pin);
    int64_t now = esp_timer_get_time();
    if ((now > pinHandle->last_notify_time) &&
        (now - pinHandle->last_notify_time < MIN_NOTIFY_INTERVAL_US)) {
        return;
    }
    pinHandle->last_notify_time = now;
    javanotify_gpio_pin_value_changed(handle, -1);
}

void javacall_gpio_init(void) {
    memset(pinsList, 0, sizeof(pinsList));
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
}

void javacall_gpio_deinit(void) {
    int i;
    for (i=0; i<PINS_MAX; i++) {
        if (pinsList[i].isOpen != 0) {
            javacall_gpio_pin_close(&pinsList[i]);
        }
    }
    gpio_uninstall_isr_service();
}

static gpio_mode_t gpio_direction(javacall_gpio_dir dir, int is_output_opendrain) {
    switch (dir) {
        case JAVACALL_GPIO_INPUT_MODE:
            return GPIO_MODE_INPUT;
        case JAVACALL_GPIO_OUTPUT_MODE:
            return is_output_opendrain?GPIO_MODE_OUTPUT_OD:GPIO_MODE_OUTPUT;
        case JAVACALL_GPIO_BOTH_MODE_INIT_INPUT:
            return GPIO_MODE_INPUT_OUTPUT;
        case JAVACALL_GPIO_BOTH_MODE_INIT_OUTPUT:
            return is_output_opendrain?GPIO_MODE_INPUT_OUTPUT_OD:GPIO_MODE_INPUT_OUTPUT;
    }
}

javacall_dio_result javacall_gpio_pin_open(const javacall_int32 port,
        const javacall_int32 pin, const javacall_gpio_dir direction,
        const javacall_gpio_mode mode, const javacall_gpio_trigger_mode trigger,
        const javacall_bool initValue,
        const javacall_bool exclusive,
        /*OUT*/ javacall_handle* pHandle) {

    *pHandle = NULL;
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
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = gpio_direction(direction, mode & JAVACALL_MODE_OUTPUT_OPEN_DRAIN);
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = BIT64(pin);
    //disable pull-down mode
    io_conf.pull_down_en = ((mode & JAVACALL_MODE_INPUT_PULL_DOWN)?GPIO_PULLDOWN_ENABLE:GPIO_PULLDOWN_DISABLE);
    //disable pull-up mode
    io_conf.pull_up_en = ((mode & JAVACALL_MODE_INPUT_PULL_UP)?GPIO_PULLUP_ENABLE:GPIO_PULLUP_DISABLE);
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    /** Set direction **/
    if ((direction == JAVACALL_GPIO_OUTPUT_MODE)
        || (direction == JAVACALL_GPIO_BOTH_MODE_INIT_OUTPUT)) {
        if (initValue == JAVACALL_TRUE) {
            gpio_set_level((gpio_num_t)pin, 1);
        } else {
            gpio_set_level((gpio_num_t)pin, 0);
        }
    }

    /** Set trigger **/
    if ((trigger == JAVACALL_TRIGGER_FALLING_EDGE) || (trigger == JAVACALL_TRIGGER_LOW_LEVEL)) {
        gpio_set_intr_type((gpio_num_t)pin, GPIO_INTR_NEGEDGE);
    } else if ((trigger == JAVACALL_TRIGGER_RISING_EDGE) || (trigger == JAVACALL_TRIGGER_HIGH_LEVEL)) {
        gpio_set_intr_type((gpio_num_t)pin, GPIO_INTR_POSEDGE);
    } else if ((trigger == JAVACALL_TRIGGER_BOTH_EDGES) || (trigger == JAVACALL_TRIGGER_BOTH_LEVELS)) {
        gpio_set_intr_type((gpio_num_t)pin, GPIO_INTR_ANYEDGE);
    } else if (trigger == JAVACALL_TRIGGER_NONE) {
        gpio_set_intr_type((gpio_num_t)pin, GPIO_INTR_DISABLE);
    } else {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "invalid trigger\n");
        return JAVACALL_DIO_INVALID_CONFIG;
    }

    pinsList[i].isOpen = 1;
    pinsList[i].pin = (gpio_num_t)pin;
    pinsList[i].notify_enabled = 0;
    pinsList[i].last_notify_time = 0;

    *pHandle = (javacall_handle)&(pinsList[i]);
    return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_close(javacall_handle handle) {
    pin_handle* pinHandle = (pin_handle*)handle;

    javacall_gpio_pin_notification_stop(handle);

    gpio_num_t pin = pinHandle->pin;
    pinHandle->isOpen = 0;
    pinHandle->pin = -1;

    gpio_reset_pin(pin);

    return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_write(const javacall_handle handle, 
        const javacall_bool val)
{
    gpio_set_level(((pin_handle*)handle)->pin, val==JAVACALL_TRUE?1:0);
    return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_read(const javacall_handle handle,
        /*OUT*/javacall_bool* pVal)
{
#if 1
    pin_handle* pinHandle = (pin_handle*)handle;

    gpio_num_t pin = pinHandle->pin;

    if (gpio_get_level(pin)) {
        *pVal = JAVACALL_TRUE;
    } else {
        *pVal = JAVACALL_FALSE;
    }
#endif
    return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_direction_set(const javacall_handle handle, const javacall_bool direction) {
#if 1
    /** Set direction **/
    pin_handle* pinHandle = (pin_handle*)handle;
    gpio_num_t pin = pinHandle->pin;
    if (direction) {
        gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    } else {
        gpio_set_direction(pin, GPIO_MODE_INPUT);
    }
#endif
    return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_set_trigger(const javacall_handle handle, const javacall_gpio_trigger_mode trigger) {
    /** Set trigger **/
    pin_handle* pinHandle = (pin_handle*)handle;
    gpio_num_t pin = pinHandle->pin;

    /** Set trigger **/
    if (trigger == JAVACALL_TRIGGER_FALLING_EDGE) {
        gpio_set_intr_type(pin, GPIO_INTR_NEGEDGE);
    } else if (trigger == JAVACALL_TRIGGER_LOW_LEVEL) {
        gpio_set_intr_type(pin, GPIO_INTR_LOW_LEVEL);
    } else if (trigger == JAVACALL_TRIGGER_RISING_EDGE) {
        gpio_set_intr_type(pin, GPIO_INTR_POSEDGE);
    } else if (trigger == JAVACALL_TRIGGER_HIGH_LEVEL) {
        gpio_set_intr_type(pin, GPIO_INTR_HIGH_LEVEL);
    } else if ((trigger == JAVACALL_TRIGGER_BOTH_EDGES) || (trigger == JAVACALL_TRIGGER_BOTH_LEVELS)) {
        gpio_set_intr_type(pin, GPIO_INTR_ANYEDGE);
    } else if (trigger == JAVACALL_TRIGGER_NONE) {
        gpio_set_intr_type(pin, GPIO_INTR_DISABLE);
    } else {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "invalid trigger\n");
        return JAVACALL_DIO_INVALID_CONFIG;
    }
    return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_notification_start(const javacall_handle handle) {
    pin_handle* pinHandle = (pin_handle*)handle;
    gpio_num_t pin = pinHandle->pin;

    if (pinHandle->notify_enabled) {
        return JAVACALL_DIO_OK;
    }

    esp_err_t err = gpio_isr_handler_add(pin, gpio_isr_handler, (void*) handle);
    if (ESP_OK != err) {
        //Ignore the error case but report to log
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "Add isr handler of GPIO %d failed: %d\n", pin, err);
    }
    pinHandle->notify_enabled = 1;
    return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_notification_stop(const javacall_handle handle)
{
    pin_handle* pinHandle = (pin_handle*)handle;
    gpio_num_t pin = pinHandle->pin;

    if (!pinHandle->notify_enabled) {
        return JAVACALL_DIO_OK;
    }

    esp_err_t err = gpio_isr_handler_remove(pin);
    if (ESP_OK != err) {
        //Ignore the error case but report to log
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "Remove isr handler of GPIO %d failed: %d\n", pin, err);
    }
    pinHandle->notify_enabled = 0;
    return JAVACALL_DIO_OK;
}
