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

HTTPS_SRC_DIR += $(EXTRA_PROTOCOLS_DIR)/https/classes \
	$(EXTRA_PROTOCOLS_DIR)/https/reference/classes

JSR_JAVA_FILES_DIR += $(HTTPS_SRC_DIR)

DOC_SOURCE_PATH := $(DOC_SOURCE_PATH)$(BUILD_ROOT_DIR)/protocol/https/classes$(DOC_PATH_SEP)
  