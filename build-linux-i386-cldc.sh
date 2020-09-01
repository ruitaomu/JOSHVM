#!/bin/sh

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


USAGE="`basename $0`: [-CTcdghjp] [-r rootdir] [-J JDK_DIR]"
HELP="Options:
-C               performs a clean before building
-c               build CLDC HI
-d               debug build
-e               erase build output / clean
-h               print this help information
-j               build javacall_impl
-p               build pcsl
-T               compile THUMB mode
-J JDK_DIR       specify the JDK directory (i.e. c:/jdk1.4)
-r rootdir       use <rootdir> as the root of the tree for building"


BUILD_TARGET=JAVACALL_LINUX_i386
unset DEBUG_OPTION
unset CLEAN
unset BUILD_JAVACALL_IMPL
unset BUILD_PCSL
unset BUILD_CLDC
unset ERASE_BUILD_OUTPUT
ENABLE_THUMB=false
ENABLE_JSR_120=false
ENABLE_JSR_75=true
ENABLE_JAMS=false
ENABLE_JAVACALL_TEST=false
ENABLE_DIO=true
ENABLE_CELLULAR=true
ENABLE_EXTRA_PROTOCOLS=true
ENABLE_DIRECTUI=false
ENABLE_JSR179=true
ENABLE_SENSORS=true
ENABLE_SECURITY=true
ENABLE_JSR280=true
BUILD_SDK=false

while getopts \?J:CTcdDehjpr: c
do
	case $c in
	C)	CLEAN="true";;
	J)	JDK_DIR=$OPTARG
		export JDK_DIR;;
	c)	BUILD_CLDC="true";;
	d)	DEBUG_OPTION="USE_DEBUG=true";;
	D)	DEBUG_OPTION="USE_DEBUG=true";;
	e)	ERASE_BUILD_OUTPUT="true";CLEAN="true";;
	h)	echo "${USAGE}"
		echo "${HELP}"
		exit 2;;
	j)	BUILD_JAVACALL_IMPL="true";;
	p)	BUILD_PCSL="true";;
	r)	WS_ROOT=$OPTARG;;
	T)	ENABLE_THUMB=true;;
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

BUILD_ROOT_DIR=${WS_ROOT}
JAVADOC_OUTPUT_DIR=${BUILD_ROOT_DIR}/doc/output

if [ -z "${BUILD_JAVACALL_IMPL}" -a -z "${BUILD_PCSL}" -a -z "${BUILD_CLDC}" -a -z "${BUILD_MIDP}" ]
then
	BUILD_JAVACALL_IMPL=true
	BUILD_PCSL=true
	BUILD_CLDC=true
fi

JAVACALL_DIR=${WS_ROOT}/javacall
PCSL_DIR=${WS_ROOT}/pcsl
CLDC_DIR=${WS_ROOT}/cldc
RESTRICTED_CRYPTO_DIR=${WS_ROOT}/restricted_crypto
TOOLS_DIR=${WS_ROOT}/tools
JSR120_DIR=${WS_ROOT}/jsr120
JSR75_DIR=${WS_ROOT}/jsr75
JAMS_DIR=${WS_ROOT}/jams
DIO_DIR=${WS_ROOT}/dio
CELLULAR_DIR=${WS_ROOT}/cellular
EXTRA_PROTOCOLS_DIR=${WS_ROOT}/protocol
JSR179_DIR=${WS_ROOT}/jsr179
DIRECTUI_DIR=${WS_ROOT}/directui
SENSORS_DIR=${WS_ROOT}/sensors
RESTRICTED_CRYPTO_DIR=${WS_ROOT}/restricted_crypto
SECURITY_DIR=${WS_ROOT}/security
LOGGING_UTIL_DIR=${WS_ROOT}/log
JSR280_DIR=${WS_ROOT}/jsr280
XMLPARSER_DIR=${WS_ROOT}/xmlparser

JAVACALL_OUTPUT_DIR=${JAVACALL_DIR}/configuration/phoneMEFeature/linux_i386/output
PCSL_OUTPUT_DIR=${PCSL_DIR}/output

if [ "${BUILD_JAVACALL_IMPL}" = "true" ]
then
	BUILDMODULE=${JAVACALL_DIR}/configuration/phoneMEFeature/linux_i386
	cd ${BUILDMODULE}
	if [ "${CLEAN}" = "true" ]; then
		echo "Cleaning:    ${BUILDMODULE}"
		rm -rf ${JAVACALL_OUTPUT_DIR}
	fi
	if [ "${ERASE_BUILD_OUTPUT}" != "true" ]; then
	    echo "Building:    ${BUILDMODULE}"
		make USE_NATIVE_AMS=${USE_NAMS} \
			JAVACALL_DIR=${JAVACALL_DIR}  \
			JAVACALL_OUTPUT_DIR=${JAVACALL_OUTPUT_DIR} \
			ENABLE_THUMB=${ENABLE_THUMB} \
			USE_PROPERTIES_FROM_FS=false \
			ENABLE_JSR_120=${ENABLE_JSR_120} \
			ENABLE_JSR_75=${ENABLE_JSR_75} \
			ENABLE_DIO=${ENABLE_DIO} \
			ENABLE_CELLULAR=${ENABLE_CELLULAR} \
			ENABLE_TEST=${ENABLE_JAVACALL_TEST} \
			${DEBUG_OPTION}
	fi
	if [ $? != 0 ];then
			echo make failed for ${BUILD_TARGET} module ${BUILDMODULE}
			exit 1
	fi
fi
DEFAULT_POOL_SIZE=1024*1280
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
			PCSL_PLATFORM=javacall_i386_gcc \
			PCSL_OUTPUT_DIR=${PCSL_OUTPUT_DIR} \
			ENABLE_THUMB=${ENABLE_THUMB} \
			DEFAULT_POOL_SIZE=${DEFAULT_POOL_SIZE} \
			${DEBUG_OPTION}
	fi
	if [ $? != 0 ];then
			echo make failed for ${BUILD_TARGET} module ${BUILDMODULE}
			exit 1
	fi
fi

if [ "${BUILD_CLDC}" = "true" ]
then
	BUILDMODULE=${CLDC_DIR}/build/javacall_i386_gcc
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
			PCSL_DIST_DIR=${PCSL_OUTPUT_DIR}/javacall_i386 \
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
			RESTRICTED_CRYPTO_DIR=${RESTRICTED_CRYPTO_DIR} \
			ENABLE_SECURITY=${ENABLE_SECURITY} \
			SECURITY_DIR=${SECURITY_DIR} \
			ENABLE_JSR280=${ENABLE_JSR280} \
			JSR280_DIR=${JSR280_DIR} \
			XMLPARSER_DIR=${XMLPARSER_DIR} \
			LOGGING_UTIL_DIR=${LOGGING_UTIL_DIR} \
			ENABLE_JAVACALL_TEST=${ENABLE_JAVACALL_TEST} \
			${DEBUG_OPTION}
	fi
	if [ $? != 0 ];then
			echo make failed for ${BUILD_TARGET} module ${BUILDMODULE}
			exit 1
	fi
fi

if [ "${BUILD_SDK}" = "true" ]
then
	BUILDMODULE=${CLDC_DIR}/build/javacall_i386_gcc
	cd ${BUILDMODULE}
	echo "Cleaning:    Java Document"
	rm -rf ${JAVADOC_OUTPUT_DIR}
	if [ "${ERASE_BUILD_OUTPUT}" != "true" ]; then
	    echo "Building:    Java Document"
		make JDK_DIR=${JDK_DIR} \
			BUILD_ROOT_DIR=${BUILD_ROOT_DIR} \
			JAVADOC_OUTPUT_DIR=${JAVADOC_OUTPUT_DIR} \
			SDK_DIR=${SDK_DIR} \
			ENABLE_CLDC_111=true \
			ENABLE_ISOLATES=true \
			JVMWorkSpace=${CLDC_DIR} \
			ENABLE_THUMB_VM=${ENABLE_THUMB} \
			ENABLE_JSR_120=${ENABLE_JSR_120} \
			JSR120_DIR=${JSR120_DIR} \
			ENABLE_JSR_75=${ENABLE_JSR_75} \
			JSR75_DIR=${JSR75_DIR} \
			ENABLE_JSR179=${ENABLE_JSR179} \
			JSR179_DIR=${JSR179_DIR} \
			ENABLE_EXTRA_PROTOCOLS=${ENABLE_EXTRA_PROTOCOLS} \
			EXTRA_PROTOCOLS_DIR=${EXTRA_PROTOCOLS_DIR} \
			ENABLE_DIRECTUI=${ENABLE_DIRECTUI} \
			DIRECTUI_DIR=${DIRECTUI_DIR} \
			ENABLE_DIO=${ENABLE_DIO} \
			DIO_DIR=${DIO_DIR} \
			ENABLE_CELLULAR=${ENABLE_CELLULAR} \
			CELLULAR_DIR=${CELLULAR_DIR} \
			ENABLE_SENSORS=${ENABLE_SENSORS} \
			SENSORS_DIR=${SENSORS_DIR} \
			ENABLE_JAMS=${ENABLE_JAMS} \
			JAMS_DIR=${JAMS_DIR} \
			ENABLE_SECURITY=${ENABLE_SECURITY} \
			SECURITY_DIR=${SECURITY_DIR} \
			ENABLE_JSR280=${ENABLE_JSR280} \
			JSR280_DIR=${JSR280_DIR} \
			XMLPARSER_DIR=${XMLPARSER_DIR} \
			LOGGING_UTIL_DIR=${LOGGING_UTIL_DIR} \
			ENABLE_JAVACALL_TEST=${ENABLE_JAVACALL_TEST} \
			doc_all package_all
	fi
	if [ $? != 0 ];then
			echo make failed for ${BUILD_TARGET} JAVADOC
			exit 1
	fi
fi
