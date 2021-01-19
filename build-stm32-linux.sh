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


BUILD_TARGET=JAVACALL_stm32
unset DEBUG_OPTION
unset CLEAN
unset BUILD_JAVACALL_IMPL
unset BUILD_PCSL
unset BUILD_CLDC
unset ERASE_BUILD_OUTPUT
unset TOOLCHAIN_HOME_DIR
ENABLE_THUMB=true
ENABLE_JSR_120=false
ENABLE_JSR_75=false
ENABLE_JAMS=false
ENABLE_JAVACALL_TEST=true
ENABLE_DIO=false
ENABLE_CELLULAR=false
ENABLE_EXTRA_PROTOCOLS=true
#NOT SUPPORT
ENABLE_DIRECTUI=false
#NOT SUPPORT
ENABLE_JSR179=false
#NOT SUPPORT
ENABLE_SENSORS=false
ENABLE_SECURITY=false
ENABLE_JSR280=false

while getopts \?J:Ct:cP:dDIehjpr:B: c
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
	r)	WS_ROOT=$OPTARG;;
	B)	BOARD_NAME=$OPTARG;;
	\?)	echo "${USAGE}"
		echo "${HELP}"
		exit 2;;
	esac
done

echo
echo
echo "-------------- BUILD ${BUILD_TARGET}----------------"

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

if [ -z "${TOOLCHAIN_HOME_DIR}" ]
then
	echo TOOLCHAIN_HOME_DIR should be set to cross compiler tools home
fi

BUILD_EXT_DIR=${WS_ROOT}/build_ext
JAVACALL_DIR=${WS_ROOT}/javacall
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

JAVACALL_OUTPUT_DIR=${JAVACALL_DIR}/configuration/phoneMEFeature/stm32/output
PCSL_OUTPUT_DIR=${PCSL_DIR}/output
PLATFORM_EXTRA_MAKEFILE_DIR=${BUILD_EXT_DIR}/board/${BOARD_NAME}

if [ "${BUILD_JAVACALL_IMPL}" = "true" ]
then
	BUILDMODULE=${JAVACALL_DIR}/configuration/phoneMEFeature/stm32
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
			ENABLE_THUMB=${ENABLE_THUMB} \
			USE_PROPERTIES_FROM_FS=true \
			PLATFORM_DIR=${PLATFORM_DIR} \
			ENABLE_JSR_120=${ENABLE_JSR_120} \
			ENABLE_JSR_75=${ENABLE_JSR_75} \
			ENABLE_DIO=${ENABLE_DIO} \
			ENABLE_CELLULAR=${ENABLE_CELLULAR} \
			ENABLE_DIRECTUI=${ENABLE_DIRECTUI} \
			ENABLE_TEST=${ENABLE_JAVACALL_TEST} \
			TOOLCHAIN_HOME_DIR=${TOOLCHAIN_HOME_DIR} \
			BOARD_NAME=${BOARD_NAME} \
			${DEBUG_OPTION}
	fi
	if [ $? != 0 ];then
			echo make failed for ${BUILD_TARGET} module ${BUILDMODULE}
			exit 1
	fi
fi

DEFAULT_POOL_SIZE=-1
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
			PCSL_PLATFORM=javacall_arm_gcc \
			PCSL_OUTPUT_DIR=${PCSL_OUTPUT_DIR} \
			ENABLE_THUMB=${ENABLE_THUMB} \
            DEFAULT_POOL_SIZE=${DEFAULT_POOL_SIZE} \
			TOOLCHAIN_HOME_DIR=${TOOLCHAIN_HOME_DIR}/.. \
			GNU_TOOLS_PREFIX=arm-none-eabi- \
			PLATFORM_EXTRA_MAKEFILE_DIR=${PLATFORM_EXTRA_MAKEFILE_DIR} \
			${DEBUG_OPTION}
	fi
	if [ $? != 0 ];then
			echo make failed for ${BUILD_TARGET} module ${BUILDMODULE}
			exit 1
	fi
fi

if [ "${BUILD_CLDC}" = "true" ]
then
	BUILDMODULE=${CLDC_DIR}/build/javacall_armv7m_gcc
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
			PCSL_DIST_DIR=${PCSL_OUTPUT_DIR}/javacall_arm \
			JAVACALL_OUTPUT_DIR=${JAVACALL_OUTPUT_DIR} \
			JVMWorkSpace=${CLDC_DIR} \
			ENABLE_THUMB_VM=${ENABLE_THUMB} \
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
			LOGGING_UTIL_DIR=${LOGGING_UTIL_DIR} \
			ENABLE_JAVACALL_TEST=${ENABLE_JAVACALL_TEST} \
			ENABLE_ESP32_VOICE_SUPPORT=${ENABLE_ESP32_VOICE_SUPPORT} \
			TOOLCHAIN_HOME_DIR=${TOOLCHAIN_HOME_DIR}/.. \
			GNU_TOOLS_DIR=${TOOLCHAIN_HOME_DIR}/.. \
			GNU_TOOLS_PREFIX=arm-none-eabi- \
			PLATFORM_EXTRA_MAKEFILE_DIR=${PLATFORM_EXTRA_MAKEFILE_DIR} \
			PLATFORM_EXTRA_LIB_DIR=${PLATFORM_EXTRA_LIB_DIR} \
			${DEBUG_OPTION}
	fi
	if [ $? != 0 ];then
			echo make failed for ${BUILD_TARGET} module ${BUILDMODULE}
			exit 1
	fi
fi
