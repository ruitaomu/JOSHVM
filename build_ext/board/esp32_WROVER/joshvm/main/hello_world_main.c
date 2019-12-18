/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_heap_caps.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

static const char *TAG = "example";

static int exist_spiffs(const char* mount_point, const char* partition_label, const char* file_name)
{
	int result;
	
	printf("exist_spiffs: %s, %s, %s\n", mount_point, partition_label, file_name);
    
    esp_vfs_spiffs_conf_t conf = {
      .base_path = mount_point,
      .partition_label = partition_label,
      .max_files = 256,
      .format_if_mount_failed = true
    };
    
    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            printf("Failed to mount or format filesystem\n");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            printf("Failed to find SPIFFS partition\n");
        } else {
            printf("Failed to initialize SPIFFS (%s)\n", esp_err_to_name(ret));
        }
        return 0;
    }
    
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(partition_label, &total, &used);
    if (ret != ESP_OK) {
        printf("Failed to get SPIFFS partition information (%s)\n", esp_err_to_name(ret));
    } else {
        printf("Partition size: total: %d, used: %d\n", total, used);
    }

    // Check if destination file exists before renaming
    struct stat st;
    if (stat(file_name, &st) == 0) {  
		printf("%s exists\n", file_name);
		result = 1;
    } else {
		printf("%s doesn't exist\n", file_name);
		result = 0;
	}

    // All done, unmount partition and disable SPIFFS
    esp_vfs_spiffs_unregister(partition_label);
	
	return result;
}

static void rename_spiffs(const char* mount_point, const char* partition_label, const char* file_name1, const char* file_name2)
{
	printf("rename_spiffs: %s, %s, %s -> %s\n", mount_point, partition_label, file_name1, file_name2);
    
    esp_vfs_spiffs_conf_t conf = {
      .base_path = mount_point,
      .partition_label = partition_label,
      .max_files = 256,
      .format_if_mount_failed = true
    };
    
    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            printf("Failed to mount or format filesystem\n");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            printf("Failed to find SPIFFS partition\n");
        } else {
            printf("Failed to initialize SPIFFS (%s)\n", esp_err_to_name(ret));
        }
        return;
    }
	
    size_t total = 0, used = 0;    
    ret = esp_spiffs_info(partition_label, &total, &used);
    if (ret != ESP_OK) {
        printf("Failed to get SPIFFS partition information (%s)\n", esp_err_to_name(ret));
    } else {
        printf("Partition size: total: %d, used: %d\n", total, used);
    }

    // Check if destination file exists before renaming
    struct stat st;
    if (stat(file_name2, &st) == 0) {
        //Don't touch if exists
		printf("%s already exists\n", file_name2);
		return;
    }

    // Rename original file
    printf("Renaming file\n");
    if (rename(file_name1, file_name2) != 0) {
        printf("Rename failed\n");
        return;
    }

    // All done, unmount partition and disable SPIFFS
    esp_vfs_spiffs_unregister(partition_label);
}

extern void JavaTask();
void app_main()
{
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
	heap_caps_print_heap_info(MALLOC_CAP_8BIT);
	heap_caps_print_heap_info(MALLOC_CAP_EXEC);
	heap_caps_print_heap_info(MALLOC_CAP_32BIT);
	heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);
	heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
	heap_caps_print_heap_info(MALLOC_CAP_SPIRAM);
	heap_caps_print_heap_info(MALLOC_CAP_DMA);	
	
	if (!exist_spiffs("/appdb/unsecure", "unsecadb", "/appdb/unsecure/properties.ini.jar") &&
		exist_spiffs("/appdb/unsecure", "unsecadb", "/appdb/unsecure/_factory.ini")) {
		rename_spiffs("/appdb/unsecure", "unsecadb", "/appdb/unsecure/_factory.ini", "/appdb/unsecure/properties.ini.jar");
	}
	
	if (exist_spiffs("/appdb/unsecure", "unsecadb", "/appdb/unsecure/properties.ini.jar")) {
		printf("Starting JOSH VM...\n");
		JavaTask();
	} else {
		printf("Can't find properties file, failed to start JOSH VM\n");
	}

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
