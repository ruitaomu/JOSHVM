#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := joshvm

include $(IDF_PATH)/make/project.mk

$(eval $(call spiffs_create_partition_image,unsecadb,settings,FLASH_IN_PROJECT))