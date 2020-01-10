#!/bin/sh
USAGE="`basename $0`: [-CTcdghjp] [-r rootdir] [-J JDK_DIR]"
HELP="Options:
-C               performs a clean before building
-c               build CLDC HI
-d               debug build
-e               erase build output / clean
-h               print this help information
-j               build javacall_impl
-p               build pcsl
-J JDK_DIR       specify the JDK directory (i.e. c:/jdk1.4)
-r rootdir       use <rootdir> as the root of the tree for building"


BUILD_TARGET=JAVACALL_esp32
unset DEBUG_OPTION
unset CLEAN
unset BUILD_JAVACALL_IMPL
unset BUILD_PCSL
unset BUILD_CLDC
unset ERASE_BUILD_OUTPUT
unset TOOLCHAIN_HOME_DIR
ENABLE_JSR_120=false
ENABLE_JSR_75=true
ENABLE_JAMS=true
ENABLE_JAVACALL_TEST=false
ENABLE_DIO=true
ENABLE_CELLULAR=true
ENABLE_EXTRA_PROTOCOLS=true
#NOT SUPPORT
ENABLE_DIRECTUI=true
#NOT SUPPORT
ENABLE_JSR179=false
#NOT SUPPORT
ENABLE_SENSORS=false
ENABLE_SECURITY=true
ENABLE_JSR280=true
ENABLE_MEDIA=false
ENABLE_DYNAMIC_PROP=true

#board can be ESP_WROVER (for ESP-WROVER-KIT), ESP_MINI (for ESP32-LyraT-Mini)
#TARGET_BOARD=ESP_WROVER
TARGET_BOARD=ESP_MINI

while getopts \?J:Ct:cP:dDIehjpr: c
do
	case $c in
	C)	CLEAN="true";;
	J)	JDK_DIR=$OPTARG
		export JDK_DIR;;
	t)  TOOLCHAIN_HOME_DIR=$OPTARG;;
	c)	BUILD_CLDC="true";;
	d)	DEBUG_OPTION="USE_DEBUG=true";;
	D)	DEBUG_OPTION="USE_DEBUG=true";;
	e)	ERASE_BUILD_OUTPUT="true";CLEAN="true";;
	h)	echo "${USAGE}"
		echo "${HELP}"
		exit 2;;
	j)	BUILD_JAVACALL_IMPL="true";;
	p)	BUILD_PCSL="true";;
	P)	PLATFORM_DIR=$OPTARG;;
	I)	INSTALL_LIB="true";;
	r)	WS_ROOT=$OPTARG;;
	\?)	echo "${USAGE}"
		echo "${HELP}"
		exit 2;;
	esac
done

echo
echo
echo "-------------- BUILD ${BUILD_TARGET}----------------"

if [ -z "${JC_COMM_ROOT}" ]
then
	JC_COMM_ROOT=`pwd`
	echo JC_COMM_ROOT not specified using ${JC_COMM_ROOT}
fi

if [ -z "${WS_ROOT}" ]
then
	WS_ROOT=`pwd`
	echo WS_ROOT not specified using ${WS_ROOT}
fi

if [ -z "${BUILD_JAVACALL_IMPL}" -a -z "${BUILD_PCSL}" -a -z "${BUILD_CLDC}" -a -z "${BUILD_MIDP}" ]
then
	BUILD_JAVACALL_IMPL=true
	BUILD_PCSL=true
	BUILD_CLDC=true
fi

if [ -z "${PLATFORM_DIR}" ]
then
	echo PLATFORM_DIR set to ${IDF_PATH}
	PLATFORM_DIR=${IDF_PATH}  
fi

if [ "${TARGET_BOARD}" = "ESP_WROVER" ]
then
	BOARD_EXT_DIR=${WS_ROOT}/build_ext/board/esp32_WROVER
else
	BOARD_EXT_DIR=${WS_ROOT}/build_ext/board/esp32_MINI
	ENABLE_MEDIA=true
fi

JAVACALL_DIR=${WS_ROOT}/javacall
JAVACALL_COMM_DIR=${JC_COMM_ROOT}/javacall
PLATFORM_EXTRA_MAKEFILE_DIR=${BOARD_EXT_DIR}
PLATFORM_EXTRA_LIB_DIR=${BOARD_EXT_DIR}/lib
PCSL_DIR=${WS_ROOT}/pcsl
CLDC_DIR=${WS_ROOT}/cldc
TOOLS_DIR=${WS_ROOT}/tools
JSR120_DIR=${WS_ROOT}/jsr120
JSR75_DIR=${WS_ROOT}/jsr75
JAMS_DIR=${WS_ROOT}/jams
DIO_DIR=${WS_ROOT}/dio
CELLULAR_DIR=${WS_ROOT}/cellular
EXTRA_PROTOCOLS_DIR=${WS_ROOT}/protocol
JSR179_DIR=${WS_ROOT}/jsr179
SENSORS_DIR=${WS_ROOT}/sensors
DIRECTUI_DIR=${WS_ROOT}/directui
SECURITY_DIR=${WS_ROOT}/security
LOGGING_UTIL_DIR=${WS_ROOT}/log
JSR280_DIR=${WS_ROOT}/jsr280
MEDIA_DIR=${WS_ROOT}/media
XMLPARSER_DIR=${WS_ROOT}/xmlparser

JAVACALL_OUTPUT_DIR=${JAVACALL_COMM_DIR}/configuration/phoneMEFeature/esp32/output
PCSL_OUTPUT_DIR=${PCSL_DIR}/output

if [ "${BUILD_JAVACALL_IMPL}" = "true" ]
then
	BUILDMODULE=${JAVACALL_COMM_DIR}/configuration/phoneMEFeature/esp32
	cd ${BUILDMODULE}
	if [ "${CLEAN}" = "true" ]; then
		echo "Cleaning:    ${BUILDMODULE}"
		rm -rf ${JAVACALL_OUTPUT_DIR}
	fi
	if [ "${ERASE_BUILD_OUTPUT}" != "true" ]; then
	    echo "Building:    ${BUILDMODULE}"
		make USE_NATIVE_AMS=${USE_NAMS} \
			JAVACALL_DIR=${JAVACALL_DIR}  \
			JAVACALL_COMM_DIR=${JAVACALL_COMM_DIR} \
			JAVACALL_OUTPUT_DIR=${JAVACALL_OUTPUT_DIR} \
			USE_PROPERTIES_FROM_FS=true \
			BOARD_EXT_DIR=${BOARD_EXT_DIR} \
			PLATFORM_DIR=${PLATFORM_DIR} \
			ENABLE_JSR_120=${ENABLE_JSR_120} \
			ENABLE_JSR_75=${ENABLE_JSR_75} \
			ENABLE_DIO=${ENABLE_DIO} \
			ENABLE_CELLULAR=${ENABLE_CELLULAR} \
			ENABLE_DIRECTUI=${ENABLE_DIRECTUI} \
			ENABLE_ESP32_VOICE_SUPPORT=${ENABLE_ESP32_VOICE_SUPPORT} \
			ENABLE_MEDIA=${ENABLE_MEDIA} \
			ENABLE_DYNAMIC_PROP=${ENABLE_DYNAMIC_PROP} \
			ENABLE_TEST=${ENABLE_JAVACALL_TEST} \
			TOOLCHAIN_HOME_DIR=${TOOLCHAIN_HOME_DIR} \
			TARGET_BOARD=${TARGET_BOARD} \
			${DEBUG_OPTION}
	fi
	if [ $? != 0 ];then
			echo make failed for ${BUILD_TARGET} module ${BUILDMODULE}
			exit 1
	fi
fi

if [ "${BUILD_PCSL}" = "true" ]
then
	BUILDMODULE=${PCSL_DIR}
	cd ${BUILDMODULE}
	if [ "${CLEAN}" = "true" ]; then
		echo "Cleaning:    ${BUILDMODULE}"
		rm -rf ${PCSL_OUTPUT_DIR}
	fi
	if [ "${ERASE_BUILD_OUTPUT}" != "true" ]; then
	    echo "Building:    ${BUILDMODULE}"
		make JAVACALL_OUTPUT_DIR=${JAVACALL_OUTPUT_DIR} \
			PCSL_PLATFORM=javacall_esp32_gcc \
			PCSL_OUTPUT_DIR=${PCSL_OUTPUT_DIR} \
			TOOLCHAIN_HOME_DIR=${TOOLCHAIN_HOME_DIR} \
			GNU_TOOLS_DIR=${TOOLCHAIN_HOME_DIR} \
			GNU_TOOLS_PREFIX=xtensa-esp32-elf- \
			PLATFORM_EXTRA_MAKEFILE_DIR=${PLATFORM_EXTRA_MAKEFILE_DIR} \
			DEFAULT_POOL_SIZE=1048576 \
			${DEBUG_OPTION}
	fi
	if [ $? != 0 ];then
			echo make failed for ${BUILD_TARGET} module ${BUILDMODULE}
			exit 1
	fi
fi

if [ "${BUILD_CLDC}" = "true" ]
then
	BUILDMODULE=${CLDC_DIR}/build/javacall_esp32_gcc
	cd ${BUILDMODULE}
	if [ "${CLEAN}" = "true" ]; then
		echo "Cleaning:    ${BUILDMODULE}"
		rm -rf dist loopgen romgen target tools ../classes ../tmpclasses ../classes.zip
	fi
	
	if [ "${ERASE_BUILD_OUTPUT}" != "true" ]; then
	    echo "Building:    ${BUILDMODULE}"
		make JDK_DIR=${JDK_DIR} \
			ENABLE_CLDC_111=true \
			ENABLE_ISOLATES=true \
			ENABLE_PCSL=true \
			ENABLE_COMPILATION_WARNINGS=true \
			PCSL_OUTPUT_DIR=${PCSL_OUTPUT_DIR} \
			PCSL_DIST_DIR=${PCSL_OUTPUT_DIR}/javacall_xtensa \
			JAVACALL_OUTPUT_DIR=${JAVACALL_OUTPUT_DIR} \
			JVMWorkSpace=${CLDC_DIR} \
			ENABLE_JSR_120=${ENABLE_JSR_120} \
			JSR120_DIR=${JSR120_DIR} \
			ENABLE_JSR_75=${ENABLE_JSR_75} \
			JSR75_DIR=${JSR75_DIR} \
			ENABLE_DIO=${ENABLE_DIO} \
			DIO_DIR=${DIO_DIR} \
			ENABLE_CELLULAR=${ENABLE_CELLULAR} \
			CELLULAR_DIR=${CELLULAR_DIR} \
			ENABLE_JAMS=${ENABLE_JAMS} \
			JAMS_DIR=${JAMS_DIR} \
			ENABLE_EXTRA_PROTOCOLS=${ENABLE_EXTRA_PROTOCOLS} \
			EXTRA_PROTOCOLS_DIR=${EXTRA_PROTOCOLS_DIR} \
			ENABLE_DIRECTUI=${ENABLE_DIRECTUI} \
			DIRECTUI_DIR=${DIRECTUI_DIR} \
			ENABLE_JSR179=${ENABLE_JSR179} \
			JSR179_DIR=${JSR179_DIR} \
			ENABLE_SENSORS=${ENABLE_SENSORS} \
			SENSORS_DIR=${SENSORS_DIR} \
			ENABLE_SECURITY=${ENABLE_SECURITY} \
			SECURITY_DIR=${SECURITY_DIR} \
			ENABLE_JSR280=${ENABLE_JSR280} \
			JSR280_DIR=${JSR280_DIR} \
			XMLPARSER_DIR=${XMLPARSER_DIR} \
			LOGGING_UTIL_DIR=${LOGGING_UTIL_DIR} \
			ENABLE_JAVACALL_TEST=${ENABLE_JAVACALL_TEST} \
			ENABLE_ESP32_VOICE_SUPPORT=${ENABLE_ESP32_VOICE_SUPPORT} \
			ENABLE_MEDIA=${ENABLE_MEDIA} \
			MEDIA_DIR=${MEDIA_DIR} \
			TOOLCHAIN_HOME_DIR=${TOOLCHAIN_HOME_DIR} \
			GNU_TOOLS_DIR=${TOOLCHAIN_HOME_DIR} \
			GNU_TOOLS_PREFIX=xtensa-esp32-elf- \
			PLATFORM_EXTRA_MAKEFILE_DIR=${PLATFORM_EXTRA_MAKEFILE_DIR} \
			PLATFORM_EXTRA_LIB_DIR=${PLATFORM_EXTRA_LIB_DIR} \
			${DEBUG_OPTION}
	fi
	if [ $? != 0 ];then
			echo make failed for ${BUILD_TARGET} module ${BUILDMODULE}
			exit 1
	fi
fi

if [ "${INSTALL_LIB}" = "true" -a "${ERASE_BUILD_OUTPUT}" != "true" ]
then
	echo Installing to ${BOARD_EXT_DIR}...
	INSTALL_DEST_DIR=${BOARD_EXT_DIR}/joshvm/components/joshvm
	cp ${CLDC_DIR}/build/javacall_esp32_gcc/dist/lib/libcldc_vm.a ${INSTALL_DEST_DIR}/libjoshvm.a
	cp ${PCSL_OUTPUT_DIR}/javacall_xtensa/lib/libpcsl_*.a ${INSTALL_DEST_DIR}
	cp ${JAVACALL_OUTPUT_DIR}/lib/libjavacall.a ${INSTALL_DEST_DIR}
	echo Done
fi
