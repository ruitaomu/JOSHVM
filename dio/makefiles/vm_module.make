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
DIO_SRC_DIR = $(DIO_DIR)/src

JSR_EXT_INCLUDE_DIRS += 
  
ROMGEN_CFG_FILES += ${DIO_SRC_DIR}/config/dio_rom.cfg 

ifeq ($(IsTarget),true)
platform = javacall
else
platform = stub
endif

ifeq ($(compiler), visCPP)
DIO_Obj_Files = \
	gpio_kni.obj \
	canbus_kni.obj \
	spibus_kni_$(platform).obj
	
gpio_kni.obj: $(DIO_SRC_DIR)/native/gpio_kni.c
	$(BUILD_C_TARGET_NO_PCH)
spibus_kni_$(platform).obj: $(DIO_SRC_DIR)/native/spibus_kni_$(platform).c
	$(BUILD_C_TARGET_NO_PCH)
canbus_kni.obj: $(DIO_SRC_DIR)/native/canbus_kni.c
	$(BUILD_C_TARGET_NO_PCH)
	
else

DIO_Obj_Files = \
	gpio_kni.o \
	canbus_kni.o \
	spibus_kni_$(platform).o

gpio_kni.o: $(DIO_SRC_DIR)/native/gpio_kni.c
	$(BUILD_C_TARGET)
spibus_kni_$(platform).o: $(DIO_SRC_DIR)/native/spibus_kni_$(platform).c
	$(BUILD_C_TARGET)
canbus_kni.o: $(DIO_SRC_DIR)/native/canbus_kni.c
	$(BUILD_C_TARGET)

endif