# put your *.o targets here, make should handle the rest!

SRCS = joshmain.c stm32l4xx_hal_msp.c stm32l4xx_it.c system_stm32l4xx.c syscalls.c sysmem.c

# all the files will be generated with this name (main.elf, main.bin, main.hex, etc)

PROJ_NAME=joshvm

# that's it, no need to change anything below this line!

###################################################

export CC=$(GNU_ARM_TOOLCHAIN_PATH)/bin/arm-none-eabi-gcc
export LD=$(GNU_ARM_TOOLCHAIN_PATH)/bin/arm-none-eabi-gcc
export AR=$(GNU_ARM_TOOLCHAIN_PATH)/bin/arm-none-eabi-ar
export AS=$(GNU_ARM_TOOLCHAIN_PATH)/bin/arm-none-eabi-as
export OBJCOPY=$(GNU_ARM_TOOLCHAIN_PATH)/bin/arm-none-eabi-objcopy

export MAP_FILE=STM32L4R9AIIx_FLASH.ld

CWD = $(shell pwd)
export ROOT=$(CWD)
export JOSHROOT=$(CWD)/../../..
export JAVACALL_LIB_DIR=$(JOSHROOT)/javacall/configuration/phoneMEFeature/stm32/output/lib
export PCSL_DIR=$(JOSHROOT)/pcsl/output/javacall_arm/lib
export PCSL_LIBS = $(PCSL_DIR)/libpcsl_escfilenames.a $(PCSL_DIR)/libpcsl_file.a $(PCSL_DIR)/libpcsl_memory.a $(PCSL_DIR)/libpcsl_network.a $(PCSL_DIR)/libpcsl_print.a $(PCSL_DIR)/libpcsl_string.a

export ASFLAGS=
export LDFLAGS=-T$(MAP_FILE) -L $(JAVACALL_LIB_DIR) -ljavacall
export LDFLAGS+=-nodefaultlibs 
export CFLAGS=-O2 -fno-common -mcpu=cortex-m4 -mthumb

###################################################

vpath %.c $(ROOT)/src  $(STM32_DRV_SRC_PATH)/Drivers/Src $(STM32_DRV_SRC_PATH)/Drivers/STM32L4xx_HAL_Driver/Src $(STM32_DRV_SRC_PATH)/Drivers/BSP/STM32L4R9I-Discovery $(STM32_DRV_SRC_PATH)/Drivers/BSP/Components/mfxstm32l152

CFLAGS += -I$(ROOT)/include -I$(STM32_DRV_SRC_PATH)/Drivers/CMSIS/Device/ST/STM32L4xx/Include -I$(STM32_DRV_SRC_PATH)/Drivers/STM32L4xx_HAL_Driver/Inc -I$(STM32_DRV_SRC_PATH)/Drivers/CMSIS/Include -I$(STM32_DRV_SRC_PATH)/Drivers/BSP/STM32L4R9I-Discovery/ -I$(STM32_DRV_SRC_PATH)/Middlewares/Third_Party/FatFs/src
CFLAGS += -DSTM32L4R9xx

SRCS += $(ROOT)/src/startup_stm32l4r9aiix.s # add startup file to build

HAL_SRCS = stm32l4xx_hal.c \
	stm32l4xx_hal_rcc.c \
	stm32l4xx_hal_rcc_ex.c \
	stm32l4xx_hal_cortex.c \
	stm32l4xx_hal_gpio.c \
	stm32l4xx_hal_pwr.c \
	stm32l4xx_hal_pwr_ex.c \
	stm32l4xx_hal_uart.c \
	stm32l4xx_hal_uart_ex.c \
	stm32l4xx_hal_usart.c \
    stm32l4xx_hal_usart_ex.c \
	stm32l4xx_hal_dma.c \
	stm32l4xx_hal_sd.c \
	stm32l4xx_hal_sd_ex.c \
	stm32l4xx_hal_i2c.c \
	stm32l4xx_hal_i2c_ex.c \
	stm32l4xx_ll_sdmmc.c \
	stm32l4r9i_discovery_io.c \
	stm32l4r9i_discovery_sd.c \
	mfxstm32l152.c \
	stm32l4r9i_discovery.c

#OBJS = $(SRCS:.c=.o)

###################################################

.PHONY: proj

all: 	proj

proj: 	$(PROJ_NAME).elf

$(PROJ_NAME).elf: $(SRCS) $(HAL_SRCS) $(JOSHROOT)/cldc/build/javacall_armv7m_gcc/dist/lib/libcldc_vm.a $(PCSL_LIBS) $(JAVACALL_LIB_DIR)/libjavacall.a 
	$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@ --specs=nosys.specs -Wl,-Map,$(PROJ_NAME).map
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

clean:
	rm -f *.o
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin
	rm -f $(PROJ_NAME).map
