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
LOGGING_SRC_DIR = $(LOGGING_UTIL_DIR)/src

JSR_EXT_INCLUDE_DIRS += -I $(LOGGING_SRC_DIR)/native/include
  
ROMGEN_CFG_FILES += ${LOGGING_SRC_DIR}/config/rom.config
 
ifeq ($(IsTarget), true)
os_platform = javacall
else
os_platform = stub
endif
  
ifeq ($(compiler), visCPP)
LOGGING_Obj_Files = josh_logging_kni.obj josh_logging.obj
	
josh_logging_kni.obj: $(LOGGING_SRC_DIR)/native/$(os_platform)/josh_logging_kni.c
	$(BUILD_C_TARGET_NO_PCH)
	
josh_logging.obj: $(LOGGING_SRC_DIR)/native/$(os_platform)/josh_logging.c
	$(BUILD_C_TARGET_NO_PCH)
else

LOGGING_Obj_Files = josh_logging_kni.o josh_logging.o

josh_logging_kni.o: $(LOGGING_SRC_DIR)/native/$(os_platform)/josh_logging_kni.c
	$(BUILD_C_TARGET)
	
josh_logging.o: $(LOGGING_SRC_DIR)/native/$(os_platform)/josh_logging.c
	$(BUILD_C_TARGET)

endif