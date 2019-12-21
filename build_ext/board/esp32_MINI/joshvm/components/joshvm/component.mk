COMPONENT_OWNBUILDTARGET = true
COMPONENT_INCLUDES += 
COMPONENT_ADD_LDFLAGS += -lpcsl_escfilenames -lpcsl_file -lpcsl_memory -lpcsl_network -lpcsl_print -lpcsl_string -ljavacall
COMPONENT_LINKER_DEPS += 
COMPONENT_SUBMODULES += 
COMPONENT_LIBRARIES += joshvm
component-joshvm-build: 
.PHONY: build
build: 
	cp $(PROJECT_PATH)/components/joshvm/lib*.a $(BUILD_DIR_BASE)/joshvm/
	