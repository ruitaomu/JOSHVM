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

PROTOCOL_SRC_DIR = $(EXTRA_PROTOCOLS_DIR)/socket_can/native

ROMGEN_CFG_FILES += $(EXTRA_PROTOCOLS_DIR)/makefiles/socket_can/rom.config

ifeq ($(compiler), visCPP)
PROTOCOL_Obj_Files += \
	canbus_kni.obj 
	
canbus_kni.obj: $(PROTOCOL_SRC_DIR)/canbus_kni.c
	$(BUILD_C_TARGET_NO_PCH)
	
else

PROTOCOL_Obj_Files += \
	canbus_kni.o

canbus_kni.o: $(PROTOCOL_SRC_DIR)/canbus_kni.c
	$(BUILD_C_TARGET)
	
endif

