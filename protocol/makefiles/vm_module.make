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
include $(EXTRA_PROTOCOLS_DIR)/makefiles/modules.config 

include $(EXTRA_PROTOCOLS_DIR)/makefiles/socket/vm_module.make

ifeq ($(LOCAL_CONFIG_USE_HTTP), true)
include $(EXTRA_PROTOCOLS_DIR)/makefiles/http/vm_module.make
endif

ifeq ($(ENABLE_SECURITY), true)
ifeq ($(LOCAL_CONFIG_USE_HTTP), true)
include $(EXTRA_PROTOCOLS_DIR)/makefiles/https/vm_module.make
endif
include $(EXTRA_PROTOCOLS_DIR)/makefiles/ssl/vm_module.make
endif

ifeq ($(LOCAL_CONFIG_USE_SOCKETCAN), true)
include $(EXTRA_PROTOCOLS_DIR)/makefiles/socket_can/vm_module.make
endif

include $(EXTRA_PROTOCOLS_DIR)/makefiles/serversocket/vm_module.make