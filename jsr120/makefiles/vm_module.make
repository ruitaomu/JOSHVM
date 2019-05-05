# Copyright (C) Max Mu
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# version 2, as published by the Free Software Foundation.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License version 2 for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
# 
# Please visit www.joshvm.org if you need additional information or
# have any questions.
JSR120_SRC_DIR = $(JSR120_DIR)/src

JSR_EXT_INCLUDE_DIRS += \
  -I"$(JSR120_SRC_DIR)/protocol/sms/native/share/inc"    \
  -I"$(JSR120_SRC_DIR)/core/common/native/share/inc"
  
ROMGEN_CFG_FILES += ${JSR120_SRC_DIR}/config/common/jsr120_rom.cfg

ifeq ($(IsTarget), true)
wma_os_platform = javacall
else
wma_os_platform = stubs
endif
  
ifeq ($(compiler), visCPP)
JSR120_Obj_Files = \
	smsProtocol.obj \
	jsr120_sms_pool.obj \
	jsr120_list_element.obj \
	jsr120_sms_listeners.obj \
	smsProtocol_$(wma_os_platform).obj
	
smsProtocol.obj: $(JSR120_SRC_DIR)/protocol/sms/native/share/smsProtocol.c
	$(BUILD_C_TARGET_NO_PCH)
jsr120_sms_pool.obj: $(JSR120_SRC_DIR)/core/common/native/ram_pool/jsr120_sms_pool.c
	$(BUILD_C_TARGET_NO_PCH)
jsr120_list_element.obj: $(JSR120_SRC_DIR)/core/common/native/share/jsr120_list_element.c
	$(BUILD_C_TARGET_NO_PCH)
jsr120_sms_listeners.obj: $(JSR120_SRC_DIR)/core/common/native/share/jsr120_sms_listeners.c
	$(BUILD_C_TARGET_NO_PCH)
smsProtocol_$(wma_os_platform).obj: $(JSR120_SRC_DIR)/protocol/sms/native/$(wma_os_platform)/smsProtocol_$(wma_os_platform).c
	$(BUILD_C_TARGET_NO_PCH)
	
else

JSR120_Obj_Files = \
	smsProtocol.o \
	jsr120_sms_pool.o \
	jsr120_list_element.o \
	jsr120_sms_listeners.o \
	smsProtocol_$(wma_os_platform).o

smsProtocol.o: $(JSR120_SRC_DIR)/protocol/sms/native/share/smsProtocol.c
	$(BUILD_C_TARGET)
jsr120_sms_pool.o: $(JSR120_SRC_DIR)/core/common/native/ram_pool/jsr120_sms_pool.c
	$(BUILD_C_TARGET)
jsr120_list_element.o: $(JSR120_SRC_DIR)/core/common/native/share/jsr120_list_element.c
	$(BUILD_C_TARGET)
jsr120_sms_listeners.o: $(JSR120_SRC_DIR)/core/common/native/share/jsr120_sms_listeners.c
	$(BUILD_C_TARGET)
smsProtocol_$(wma_os_platform).o: $(JSR120_SRC_DIR)/protocol/sms/native/$(wma_os_platform)/smsProtocol_$(wma_os_platform).c
	$(BUILD_C_TARGET)
endif