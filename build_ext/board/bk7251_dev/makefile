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
RT_LIB_DIR=$(RTTHREAD_HOME_DIR)/../components/josh/libs

all : $(RT_LIB_DIR)/libpcsl_escfilenames.a  $(RT_LIB_DIR)/libpcsl_memory.a   $(RT_LIB_DIR)/libpcsl_print.a $(RT_LIB_DIR)/libpcsl_file.a $(RT_LIB_DIR)/libpcsl_network.a $(RT_LIB_DIR)/libpcsl_string.a $(RT_LIB_DIR)/libjavacall.a $(RT_LIB_DIR)/libcldc_vm.a

$(RT_LIB_DIR)/libcldc_vm.a : $(CLDC_OUTPUT_DIR)/dist/lib/libcldc_vm.a
	@echo "copy libcldc_vm..."
	cp $< $@
	
$(RT_LIB_DIR)/%.a : $(PCSL_OUTPUT_DIR)/javacall_arm/lib/%.a
	@echo "copy pcsl libs..."
	cp $< $(RT_LIB_DIR)

$(RT_LIB_DIR)/libjavacall.a : $(JAVACALL_OUTPUT_DIR)/lib/libjavacall.a
	@echo "copy javacall libs..."
	cp $< $(RT_LIB_DIR)

