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
JSR75_SRC_DIR = $(JSR75_DIR)/src

JSR_EXT_INCLUDE_DIRS += 
  
ROMGEN_CFG_FILES += ${JSR75_SRC_DIR}/config/common/jsr75_rom.cfg 

ifeq ($(IsTarget), true)
os_platform = javacall
else
os_platform = stubs
endif
  
ifeq ($(compiler), visCPP)
JSR75_Obj_Files = \
	jsr75_FileConnection.obj \
	jsr75_initializer_kni.obj
	
jsr75_FileConnection.obj: $(JSR75_SRC_DIR)/kvem/native/$(os_platform)/jsr75_FileConnection.c
	$(BUILD_C_TARGET_NO_PCH)
jsr75_initializer_kni.obj: $(JSR75_SRC_DIR)/core/native/jsr75_initializer_kni.c
	$(BUILD_C_TARGET_NO_PCH)
else

JSR75_Obj_Files = \
	jsr75_FileConnection.o \
	jsr75_initializer_kni.o

jsr75_FileConnection.o: $(JSR75_SRC_DIR)/kvem/native/$(os_platform)/jsr75_FileConnection.c
	$(BUILD_C_TARGET)
jsr75_initializer_kni.o: $(JSR75_SRC_DIR)/core/native/jsr75_initializer_kni.c
	$(BUILD_C_TARGET)

endif