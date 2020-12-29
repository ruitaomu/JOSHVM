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
JAMS_SRC_DIR = $(JAMS_DIR)/src

JSR_EXT_INCLUDE_DIRS += 

ROMGEN_CFG_FILES += ${JAMS_SRC_DIR}/config/jams_rom.cfg 
  
ifeq ($(IsTarget), true)
os_platform = javacall
else
os_platform = stub
endif
  
ifeq ($(compiler), visCPP)
JAMS_Obj_Files = jams_kni_$(os_platform).obj
	
jams_kni_$(os_platform).obj: $(JAMS_SRC_DIR)/native/jams_kni_$(os_platform).c
	$(BUILD_C_TARGET_NO_PCH)

else

JAMS_Obj_Files = jams_kni_$(os_platform).o

jams_kni_$(os_platform).o: $(JAMS_SRC_DIR)/native/jams_kni_$(os_platform).c
	$(BUILD_C_TARGET)

endif