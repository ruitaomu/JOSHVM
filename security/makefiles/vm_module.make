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

#######(If ENABLE_SECURITY enabled)#######
ifeq ($(ENABLE_SECURITY), true)

ROMGEN_CFG_FILES += $(SECURITY_DIR)/makefiles/rom.config
  
ifeq ($(ENABLE_SECURITY_NATIVE_RSA_SIGNATURE), true)

ROMGEN_CFG_FILES += $(SECURITY_DIR)/makefiles/rom_native_rsa.config

SECURITY_CRYPTO_DIR = $(SECURITY_DIR)/crypto/reference/native

JSR_EXT_INCLUDE_DIRS += -I"$(SECURITY_CRYPTO_DIR)"



ifeq ($(compiler), visCPP)

CRYPTO_Obj_Files := \
	NativeCrypto.obj \
	bnlib.obj
	
NativeCrypto.obj: $(SECURITY_CRYPTO_DIR)/NativeCrypto.c
	$(BUILD_C_TARGET_NO_PCH)
bnlib.obj: $(SECURITY_CRYPTO_DIR)/bnlib.c
	$(BUILD_C_TARGET_NO_PCH)
else

CRYPTO_Obj_Files := \
	NativeCrypto.o \
	bnlib.o

NativeCrypto.o: $(SECURITY_CRYPTO_DIR)/NativeCrypto.c
	$(BUILD_C_TARGET)
bnlib.o: $(SECURITY_CRYPTO_DIR)/bnlib.c
	$(BUILD_C_TARGET)

endif

SECURITY_Obj_Files = $(CRYPTO_Obj_Files)

endif
#######(End of if ENABLE_SECURITY)#######
endif