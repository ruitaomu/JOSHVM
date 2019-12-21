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

#include "javacall_i2c.h"
#include "javacall_logging.h"
#include "javacall_memory.h"

#include "driver/i2c.h"

#define I2C_GPIO_SDA 18
// #define I2C_GPIO_SCL 19

#define I2C_GPIO_SCL 23

#define I2C_MASTER_FREQ_HZ 100000
#define WRITE_BIT I2C_MASTER_WRITE              /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ                /*!< I2C master read */
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */

typedef struct i2c_bus_t {
    javacall_int32 busNum;
    struct i2c_bus_t* next;
    struct i2c_slave_device_t* slaves;
} i2c_bus;

typedef struct i2c_message_t {
    char* data;
    int len;
    javacall_bool write;
    struct i2c_message_t* next;
} i2c_message;

typedef struct i2c_slave_device_t {
    javacall_int32 address;
    javacall_int32 addressSize;
    javacall_int32 clockFrequency;
    i2c_bus* bus;
    struct i2c_slave_device_t* next;
    i2c_message* headMsg;
    i2c_message* tailMsg;
} i2c_slave_device;

static i2c_bus* g_i2c_buses = NULL;

static javacall_dio_result create_bus(javacall_int32 busNum, javacall_int32 clockFrequency, i2c_bus** bus) {
    if (busNum != I2C_NUM_0) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "wrong bus number:%d\n", busNum);
        return JAVACALL_DIO_INVALID_CONFIG;
    }

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_GPIO_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_GPIO_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = clockFrequency
    };

    esp_err_t result = i2c_param_config(busNum, &conf);
    if (result != ESP_OK) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "i2c config error:%d\n", result);
        return JAVACALL_DIO_FAIL;
    }
    result = i2c_driver_install(busNum, conf.mode, 0, 0, 0);
    if (result != ESP_OK) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "i2c driver install error:%d\n", result);
        return JAVACALL_DIO_FAIL;
    }

    i2c_bus* tmpBus = (i2c_bus*)javacall_malloc(sizeof(i2c_bus));
    if (tmpBus == NULL) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "memory allocation error\n");
        return JAVACALL_DIO_OUT_OF_MEMORY;
    }

    memset(tmpBus, 0, sizeof(i2c_bus));
    tmpBus->busNum = busNum;
    tmpBus->next = g_i2c_buses;
    g_i2c_buses = tmpBus;
    *bus = tmpBus;

    return JAVACALL_DIO_OK;
}

static javacall_dio_result delete_bus(javacall_int32 busNum) {
    i2c_bus *prev, *cur;

    prev = NULL;
    cur = g_i2c_buses;
    while (cur) {
        if (cur->busNum == busNum) {
            break;
        }
        prev = cur;
        cur = cur->next;
    }

    if (NULL == cur) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "no bus in list\n");
        return JAVACALL_DIO_FAIL;
    }

    if (prev == NULL) {
        g_i2c_buses = cur->next;
        i2c_driver_delete(cur->busNum);
    } else {
        prev->next = cur->next;
    }
    javacall_free(cur);
    return JAVACALL_DIO_OK;
}

static javacall_dio_result open_slave(i2c_slave_device* device, javacall_int32 busNum) {
    javacall_dio_result ret;
    i2c_bus* bus;
    i2c_slave_device* tmpDevice;

    bus = g_i2c_buses;
    while (bus) {
        if (bus->busNum == busNum) {
            break;
        }
        bus = bus->next;
    }

    if (bus == NULL) {
        // try to create bus
        ret = create_bus(busNum, device->clockFrequency, &bus);
        if (ret != JAVACALL_DIO_OK) {
            return ret;
        }
    }

    tmpDevice = bus->slaves;
    while (tmpDevice) {
        if (tmpDevice->address == device->address) {
            return JAVACALL_DIO_BUSY;
        }
        tmpDevice = tmpDevice->next;
    }

    device->bus = bus;
    device->next = bus->slaves;
    bus->slaves = device;
    return JAVACALL_DIO_OK;
}

static void close_slave(i2c_slave_device* device) {
    i2c_slave_device *prev, *cur;

    prev = NULL;
    cur = device->bus->slaves;
    while (cur) {
        if (cur == device) {
            break;
        }
        prev = cur;
        cur = cur->next;
    }

    if (NULL == cur) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "no device in list\n");
        return;
    }

    if (prev == NULL) {
        device->bus->slaves = device->next;
        if (device->next == NULL) {
            delete_bus(device->bus->busNum);
        }
    } else {
        prev->next = device->next;
    }

    javacall_free(device);
}

static void cleanMsg(i2c_slave_device* device) {
    i2c_message* curMsg;
    i2c_message* nextMsg = device->headMsg;

    while (nextMsg != NULL) {
        curMsg = nextMsg;
        nextMsg = nextMsg->next;
        javacall_free(curMsg->data);
        javacall_free(curMsg);
    }
    device->headMsg = NULL;
    device->tailMsg = NULL;
}

static void append_cmd_link(i2c_cmd_handle_t cmd, i2c_slave_device* device, i2c_message* msg) {
    i2c_master_start(cmd);
    if (msg->write) {
        i2c_master_write_byte(cmd, (device->address) << 1 | WRITE_BIT, ACK_CHECK_EN);
        i2c_master_write(cmd, msg->data, msg->len, ACK_CHECK_EN);
    } else {
        i2c_master_write_byte(cmd, (device->address) << 1 | READ_BIT, ACK_CHECK_EN);
        if (msg->len > 1) {
            i2c_master_read(cmd, msg->data, msg->len - 1, ACK_VAL);
        }
        i2c_master_read_byte(cmd, msg->data + msg->len - 1, NACK_VAL);
    }
}

javacall_dio_result javacall_i2c_open(javacall_int32 busNum,
        javacall_int32 devAddr, javacall_int32 addrSize,
        javacall_int32 clockFrequency,
        const javacall_bool exclusive,
        /*OUT*/javacall_handle* pHandle) {
    javacall_dio_result ret;

    if (JAVACALL_TRUE != exclusive) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "Shared mode is unsupported for I2C device\n");
        return JAVACALL_DIO_UNSUPPORTED_ACCESS_MODE;
    }

    if (clockFrequency == PERIPHERAL_CONFIG_DEFAULT) {
        clockFrequency = I2C_MASTER_FREQ_HZ;
    }

    if (addrSize == PERIPHERAL_CONFIG_DEFAULT) {
        addrSize = 7;
    }

    if (addrSize != 7 && addrSize != 10) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "[I2C] Configuration error: addrSize != 7 or 10\n");
        return JAVACALL_DIO_INVALID_CONFIG;
    }

    if (busNum == PERIPHERAL_CONFIG_DEFAULT) {
        busNum = I2C_NUM_0;
    }

    if (busNum < 0) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI,
                "[I2C] Configuration error: busNum should not be less than zero\n");
        return JAVACALL_DIO_FAIL;
    }

    i2c_slave_device* device;
    *pHandle = NULL;

    device = (i2c_slave_device*)javacall_malloc(sizeof(i2c_slave_device));
    if (device == NULL) {
        javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "[I2C] Memory allocation error\n");
        return JAVACALL_DIO_OUT_OF_MEMORY;
    }

    memset(device, 0, sizeof(i2c_slave_device));
    device->address = devAddr;
    device->addressSize = addrSize;
    device->clockFrequency = clockFrequency;

    ret = open_slave(device, busNum);
    if (ret != JAVACALL_DIO_OK) {
        javacall_free(device);
        return ret;
    }

    *pHandle = (javacall_handle)device;
    return JAVACALL_DIO_OK;
}

javacall_dio_result javacall_i2c_transfer_sync(const javacall_handle handle,
                                                const javacall_i2c_message_type type,
                                                const javacall_bool write,
                                                char* pData, int len,
                                                javacall_int32 *const pBytes) {

    javacall_dio_result result = JAVACALL_DIO_OK;
    int needClean = 1;
    i2c_slave_device* device = (i2c_slave_device*)handle;

    do {
        if (type == JAVACALL_I2C_COMBINED_START || type == JAVACALL_I2C_COMBINED_BODY) {
            i2c_message* msg = (i2c_message*) javacall_malloc(sizeof(i2c_message));
            if (msg == NULL) {
                javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "failed to alloc i2c message\n");
                result = JAVACALL_DIO_OUT_OF_MEMORY;
                break;
            }

            msg->data = (char *)javacall_malloc(len);
            if (msg->data == NULL) {
                javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "failed to alloc i2c message\n");
                javacall_free(msg);
                result = JAVACALL_DIO_OUT_OF_MEMORY;
                break;
            }

            memcpy(msg->data, pData, len);
            msg->len = len;
            msg->write = write;
            msg->next = NULL;
            if (device->headMsg == NULL) {
                device->headMsg = msg;
                device->tailMsg = msg;
            } else {
                device->tailMsg-> next = msg;
                device->tailMsg = msg;
            }

            needClean = 0;
            *pBytes = len;
        } else {
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            if (type == JAVACALL_I2C_COMBINED_END) {
                i2c_message* msg = device->headMsg;
                while (msg) {
                    append_cmd_link(cmd, device, msg);
                    msg = msg->next;
                }
            }
            i2c_message lastMsg;
            lastMsg.data = pData;
            lastMsg.len = len;
            lastMsg.write = write;
            append_cmd_link(cmd, device, &lastMsg);
            i2c_master_stop(cmd);
            esp_err_t ret = i2c_master_cmd_begin(device->bus->busNum, cmd, 500 / portTICK_RATE_MS);
            i2c_cmd_link_delete(cmd);
            if (ret == ESP_OK) {
                javacall_logging_printf(JAVACALL_LOGGING_INFORMATION, JC_DAAPI, "transfer command success\n");
                *pBytes = len;
            } else if (ret == ESP_ERR_TIMEOUT) {
                javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "bus is busy\n");
                result = JAVACALL_DIO_BUSY;
                break;
            } else {
                javacall_logging_printf(JAVACALL_LOGGING_ERROR, JC_DAAPI, "transfer command failed. %d\n", ret);
                result = JAVACALL_DIO_FAIL;
                break;
            }
        }
    } while (0);

    if (needClean) {
        cleanMsg(device);
    }
    return result;
}

void javacall_i2c_close(javacall_handle handle) {
    close_slave((i2c_slave_device*)handle);
}
