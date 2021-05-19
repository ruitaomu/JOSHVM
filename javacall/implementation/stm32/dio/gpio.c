/*
 * Copyright (C) Max Mu
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Please visit www.joshvm.org if you need additional information or
 * have any questions.
 */
#include <javacall_gpio.h>
#include <javacall_logging.h>

#define PINS_MAX 40
#define INVALID_PIN 0

typedef struct pin_handle {
    uint8_t isOpen;
	uint8_t notify_enabled;
	uint16_t validPin;
	javacall_int32 port;
    javacall_int32 pin;
    GPIO_TypeDef * validPort;
	javacall_gpio_dir direction;
	javacall_gpio_mode mode;
	javacall_gpio_trigger_mode trigger;
	javacall_bool currentdir;
} pin_handle;

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

static int enable_gpio_port(GPIO_TypeDef* validPort) {
	if (validPort == GPIOA) {
		__HAL_RCC_GPIOA_CLK_ENABLE();
	} else if (validPort == GPIOB) {
		__HAL_RCC_GPIOB_CLK_ENABLE();
	} else if (validPort == GPIOC) {
		__HAL_RCC_GPIOC_CLK_ENABLE();
	} else if (validPort == GPIOD) {
		__HAL_RCC_GPIOD_CLK_ENABLE();
	} else if (validPort == GPIOE) {
		__HAL_RCC_GPIOE_CLK_ENABLE();
	} else if (validPort == GPIOF) {
		__HAL_RCC_GPIOF_CLK_ENABLE();
	} else if (validPort == GPIOG) {
		__HAL_RCC_GPIOG_CLK_ENABLE();
	} else if (validPort == GPIOH) {
		__HAL_RCC_GPIOH_CLK_ENABLE();
	} else if (validPort == GPIOI) {
		__HAL_RCC_GPIOI_CLK_ENABLE();
	} else {
		return -1;
	}

	return 0;
}

static int disable_gpio_port(GPIO_TypeDef* validPort) {
	if (validPort == GPIOA) {
		__HAL_RCC_GPIOA_CLK_DISABLE();
	} else if (validPort == GPIOB) {
		__HAL_RCC_GPIOB_CLK_DISABLE();
	} else if (validPort == GPIOC) {
		__HAL_RCC_GPIOC_CLK_DISABLE();
	} else if (validPort == GPIOD) {
		__HAL_RCC_GPIOD_CLK_DISABLE();
	} else if (validPort == GPIOE) {
		__HAL_RCC_GPIOE_CLK_DISABLE();
	} else if (validPort == GPIOF) {
		__HAL_RCC_GPIOF_CLK_DISABLE();
	} else if (validPort == GPIOG) {
		__HAL_RCC_GPIOG_CLK_DISABLE();
	} else if (validPort == GPIOH) {
		__HAL_RCC_GPIOH_CLK_DISABLE();
	} else if (validPort == GPIOI) {
		__HAL_RCC_GPIOI_CLK_DISABLE();
	} else {
		return -1;
	}

	return 0;
}

uint16_t get_valid_pin(javacall_int32 pin) {
	if (pin < 0 || pin > 15) {
		return INVALID_PIN;
	}

	return (uint16_t) ((1u<<pin) & 0xFFFF);
}

static GPIO_TypeDef* validPorts[] = {
	GPIOA,
	GPIOB,
	GPIOC,
	GPIOD,
	GPIOE,
	GPIOF,
	GPIOG,
	GPIOH,
	GPIOI
};

GPIO_TypeDef* get_valid_port(javacall_int32 port) {
	if (port < 0 || port >= sizeof(validPorts)/sizeof(GPIO_TypeDef*)) {
		return NULL;
	} else {
		return validPorts[port];
	}
}

static uint32_t get_valid_mode(javacall_bool isOutput, javacall_gpio_mode mode, javacall_gpio_trigger_mode trigger) {
	uint32_t val = 0;

	/** Set trigger **/
    if ((trigger == JAVACALL_TRIGGER_FALLING_EDGE) || (trigger == JAVACALL_TRIGGER_LOW_LEVEL)) {
        val = GPIO_MODE_IT_FALLING;
    } else if ((trigger == JAVACALL_TRIGGER_RISING_EDGE) || (trigger == JAVACALL_TRIGGER_HIGH_LEVEL)) {
        val = GPIO_MODE_IT_RISING;
    } else if ((trigger == JAVACALL_TRIGGER_BOTH_EDGES) || (trigger == JAVACALL_TRIGGER_BOTH_LEVELS)) {
        val = GPIO_MODE_IT_RISING_FALLING;
    } else if (trigger == JAVACALL_TRIGGER_NONE) {
        val = 0;
    } else {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "invalid trigger\n");
        return JAVACALL_DIO_INVALID_CONFIG;
    }

	int is_output_opendrain = mode & JAVACALL_MODE_OUTPUT_OPEN_DRAIN;

    if (!isOutput) {
        return GPIO_MODE_INPUT | val;
	} else {
        return is_output_opendrain ? GPIO_MODE_OUTPUT_OD : GPIO_MODE_OUTPUT_PP;
    }
	return GPIO_MODE_OUTPUT_OD; //Fallback to GPIO_MODE_OUTPUT_OD as no valid dir found
}

static uint32_t get_valid_pull(javacall_gpio_mode mode) {
	if (mode == JAVACALL_MODE_INPUT_PULL_UP) {
		return GPIO_PULLUP;
	} else if (mode == JAVACALL_MODE_INPUT_PULL_DOWN) {
		return GPIO_PULLDOWN;
	} else {
		return GPIO_NOPULL;
	}
}

static void enable_gpio(uint16_t validPin, GPIO_TypeDef *validPort, javacall_bool direction, javacall_gpio_mode mode, javacall_gpio_trigger_mode trigger) {
	/** Enable GPIO Pin **/
    GPIO_InitTypeDef  GPIO_InitStructure = {0};
    GPIO_InitStructure.Pin   = validPin;
    GPIO_InitStructure.Mode  = get_valid_mode(direction, mode, trigger);
    GPIO_InitStructure.Pull  = get_valid_pull(mode);
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	HAL_GPIO_Init(validPort, &GPIO_InitStructure);
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

	uint16_t validPin = get_valid_pin(pin);
	if (validPin == INVALID_PIN) {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "Invalid PIN: %d\n", pin);
        return JAVACALL_DIO_FAIL;
	}

	GPIO_TypeDef * validPort = get_valid_port(port);
	if (validPort == NULL) {
		javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "Invalid Port: %d\n", port);
        return JAVACALL_DIO_FAIL;
	}

	/** Enable GPIO port **/
	if (enable_gpio_port(validPort) != 0) {
		return JAVACALL_DIO_FAIL;
	}

	javacall_bool isOutput;
	/** Set direction **/
    if ((direction == JAVACALL_GPIO_OUTPUT_MODE)
        || (direction == JAVACALL_GPIO_BOTH_MODE_INIT_OUTPUT)) {
		
		isOutput = JAVACALL_TRUE;
    } else {
		isOutput = JAVACALL_FALSE;
	}   

	enable_gpio(validPin, validPort, isOutput, mode, trigger);

    if (isOutput) {
		if (initValue == JAVACALL_TRUE) {
			HAL_GPIO_WritePin(validPort, validPin, GPIO_PIN_SET);
		} else {
			HAL_GPIO_WritePin(validPort, validPin, GPIO_PIN_RESET);
		}
	}

    pinsList[i].currentdir = isOutput;
    pinsList[i].isOpen = 1;
    pinsList[i].pin = pin;
    pinsList[i].notify_enabled = 0;
    pinsList[i].port = port;
	pinsList[i].validPin = validPin;
	pinsList[i].validPort = validPort;
	pinsList[i].direction = direction;
	pinsList[i].mode = mode;
	pinsList[i].trigger = trigger;

    *pHandle = (javacall_handle)&(pinsList[i]);
    return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_close(javacall_handle handle) {
    pin_handle* pinHandle = (pin_handle*)handle;

    javacall_gpio_pin_notification_stop(handle);
	
	//disable_gpio_port(pinHandle->validPort);

    pinHandle->isOpen = 0;
    pinHandle->pin = -1;
	pinHandle->port = -1;
	pinHandle->validPin = INVALID_PIN;
	pinHandle->validPort = NULL;
    
    return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_write(const javacall_handle handle, 
        const javacall_bool val)
{
	pin_handle* pinHandle = (pin_handle*)handle;
    HAL_GPIO_WritePin(pinHandle->validPort, pinHandle->validPin, val==JAVACALL_TRUE?GPIO_PIN_SET:GPIO_PIN_RESET);
    return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_read(const javacall_handle handle,
        /*OUT*/javacall_bool* pVal)
{

    pin_handle* pinHandle = (pin_handle*)handle;

    if (HAL_GPIO_ReadPin(pinHandle->validPort, pinHandle->validPin) != GPIO_PIN_RESET) {
        *pVal = JAVACALL_TRUE;
    } else {
        *pVal = JAVACALL_FALSE;
    }

    return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_direction_set(const javacall_handle handle, const javacall_bool direction) {
    /** Set direction **/
    pin_handle* pinHandle = (pin_handle*)handle;
    javacall_gpio_dir  pindir = pinHandle->direction;
	javacall_gpio_mode pinmode = pinHandle->mode;
	javacall_gpio_trigger_mode pintrig = pinHandle->trigger;
	javacall_bool current_pindir = pinHandle->currentdir;

	if (direction == current_pindir) {
		return JAVACALL_DIO_OK;
	}

	enable_gpio(pinHandle->validPin, pinHandle->validPort, direction, pinmode, pintrig);

    return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_set_trigger(const javacall_handle handle, const javacall_gpio_trigger_mode trigger) {
    pin_handle* pinHandle = (pin_handle*)handle;
    javacall_bool  pindir = pinHandle->currentdir;
	javacall_gpio_mode pinmode = pinHandle->mode;
	javacall_gpio_trigger_mode pintrig = pinHandle->trigger;

	if (pintrig == trigger) {
		return JAVACALL_DIO_OK;
	}

	enable_gpio(pinHandle->validPin, pinHandle->validPort, pindir, pinmode, trigger);

    return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_notification_start(const javacall_handle handle) {
    pin_handle* pinHandle = (pin_handle*)handle;

    if (pinHandle->notify_enabled) {
        return JAVACALL_DIO_OK;
    }

    pinHandle->notify_enabled = 1;

    return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_gpio_pin_notification_stop(const javacall_handle handle)
{
    pin_handle* pinHandle = (pin_handle*)handle;

    if (!pinHandle->notify_enabled) {
        return JAVACALL_DIO_OK;
    }
	
	pinHandle->notify_enabled = 0;
    return JAVACALL_DIO_OK;
}

void javacall_gpio_incoming_event(uint16_t pin) {
    int i;
    for (i=0; i<PINS_MAX; i++) {
        if (pinsList[i].isOpen && 
			pinsList[i].notify_enabled &&
			pinsList[i].validPin == pin) {
            javanotify_gpio_pin_value_changed(&pinsList[i], -1);
        }
    }	
}

