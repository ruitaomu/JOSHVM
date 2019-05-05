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

ifeq ($(JAVACALL_DIR),)
$(error JAVACALL_DIR is not set.)
endif

CC=$(TOOLCHAIN_HOME_DIR)/bin/arm-none-eabi-gcc.exe
AR=$(TOOLCHAIN_HOME_DIR)/bin/arm-none-eabi-ar.exe

ifneq ($(PLATFORM_EXTRA_MAKEFILE_DIR)x, x)
	include $(PLATFORM_EXTRA_MAKEFILE_DIR)/platform_extra.gmk
endif

include environment.gmk

#For each project specified toolchain shall be selected
include $(JAVACALL_DIR)/build/armgcc_win32/rules.gmk

#Specific targets can be achieved before and after achievement of common targets
#Specify these targets after including 'rules.gmk' file.
pre_target:
#post_target: $(JAVACALL_OUTPUT_LIB_DIR)/emulator.res
