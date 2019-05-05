/*
 * Copyright (C) Max Mu
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 * Please visit www.joshvm.org if you need additional information or
 * have any questions.
 */
#include <kni.h>
#include <kni_globals.h>

#include <javacall_logging.h>
#include <javacall_cellular_network.h>
#include <pcsl_string.h>
#include <pcsl_memory.h>

/**
 * Create pcsl_string from the specified Java String object.
 * The caller is responsible for freeing the created pcsl_string when done.
 *
 * @param java_str pointer to the Java String instance
 * @param pcsl_str pointer to the pcsl_string instance
 * @return status of the operation
 */
static pcsl_string_status jstring_to_pcsl_string(jstring java_str,
					       pcsl_string * pcsl_str) {
  if (pcsl_str == NULL) {
    return PCSL_STRING_EINVAL;
  }

  if (KNI_IsNullHandle(java_str)) {
    * pcsl_str = PCSL_STRING_NULL;
    return PCSL_STRING_OK;
  } else {
    const jsize length  = KNI_GetStringLength(java_str);

    if (length < 0) {
      * pcsl_str = PCSL_STRING_NULL;
      return PCSL_STRING_ERR;
    } else if (length == 0) {
      * pcsl_str = PCSL_STRING_EMPTY;
      return PCSL_STRING_OK;
    } else {
      jchar * buffer = (jchar*)pcsl_mem_malloc(length * sizeof(jchar));

      if (buffer == NULL) {
	    * pcsl_str = PCSL_STRING_NULL;
	    return PCSL_STRING_ENOMEM;
      }

      KNI_GetStringRegion(java_str, 0, length, buffer);

      {
	    pcsl_string_status status =
	    pcsl_string_convert_from_utf16(buffer, length, pcsl_str);

	    pcsl_mem_free(buffer);

	    return status;
      }
    }
  }
}

/**
 * The method gets 'pcsl_string' from 'Java String'.
 *
 * Note: the caller is responsible for calling
 * 'pcsl_string_free(*string);
 *  pcsl_mem_free(*string);'
 * after use.
 *
 * @param stringHandle The input 'Java String' handle
 * @param string The output 'pcsl_string'
 * @return  0 if conversion passed successfully,
 *         -1 if any error occurred.
 */
static int get_pcsl_string(jobject stringHandle, pcsl_string ** string)
{
    if (KNI_IsNullHandle(stringHandle)) {
        KNI_ThrowNew(KNINullPointerException, "Try to convert null string");
        return -1;
    } else {
		pcsl_string * str = (pcsl_string *)pcsl_mem_malloc(sizeof(pcsl_string));    
	        
		if (str == NULL) {
		    KNI_ThrowNew(KNIOutOfMemoryError, NULL);
		    return -1;
		}

	 	if (jstring_to_pcsl_string(stringHandle, str) != PCSL_STRING_OK) {
		    pcsl_mem_free(str);
			return -1;
		}

		* string = str;	
    }
    return 0;
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_setAPN0()
{
	javacall_cellular_apn_info apn;
	pcsl_string *pstrAPNName = NULL;
	pcsl_string *pstrUsername = NULL;
	pcsl_string *pstrPassword = NULL;
	const jbyte *p1, *p2, *p3;

	KNI_StartHandles(3);
    KNI_DeclareHandle(stringAPNNameHandle);
	KNI_DeclareHandle(stringUsernameHandle);
	KNI_DeclareHandle(stringPasswordHandle);
	
	KNI_GetParameterAsObject(1, stringAPNNameHandle);
	KNI_GetParameterAsObject(2, stringUsernameHandle);
	KNI_GetParameterAsObject(3, stringPasswordHandle);

	if (!get_pcsl_string(stringAPNNameHandle, &pstrAPNName) &&
		!get_pcsl_string(stringUsernameHandle, &pstrUsername) &&
		!get_pcsl_string(stringPasswordHandle, &pstrPassword)) {

		p1 = pcsl_string_get_utf8_data(pstrAPNName);
		if (p1 != NULL) {
			javautil_strncpy(apn.apnname, (const char*)p1, JAVACALL_MAX_APN_NAME_LENGTH);
			pcsl_string_release_utf8_data(p1, pstrAPNName);
		} else {
			apn.apnname[0] = '\0';
		}

		p2 = pcsl_string_get_utf8_data(pstrUsername);
		if (p2 != NULL) {
			javautil_strncpy(apn.username, (const char*)p2, JAVACALL_MAX_APN_USERNAME_LENGTH);
			pcsl_string_release_utf8_data(p2, pstrUsername);
		} else {
			apn.username[0] = '\0';
		}

		p3 = pcsl_string_get_utf8_data(pstrPassword);
		if (p3 != NULL) {
			javautil_strncpy(apn.password, (const char*)p3, JAVACALL_MAX_APN_PASSWORD_LENGTH);
			pcsl_string_release_utf8_data(p3, pstrPassword);
		} else {
			apn.password[0] = '\0';
		}
		
		javacall_cellular_set_accesspoint(JAVACALL_DEFAULT_SUBSCRIBER_SLOT, &apn);
	}

	if (pstrAPNName) pcsl_string_free(pstrAPNName);
	if (pstrUsername) pcsl_string_free(pstrUsername);
	if (pstrPassword) pcsl_string_free(pstrPassword);
	
	KNI_EndHandles();
	KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_OBJECT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getCurrentAPNName0()
{
	int	apnname_len;
	javacall_cellular_apn_info apn;
	pcsl_string pcslstr_apnname;

	KNI_StartHandles(1);
    KNI_DeclareHandle(apnnameHandle);

	if (JAVACALL_OK == javacall_cellular_get_accesspoint(JAVACALL_DEFAULT_SUBSCRIBER_SLOT, &apn)) {
		apnname_len = javautil_strlen(apn.apnname);
		pcsl_string_convert_from_utf8((const jbyte*)apn.apnname, apnname_len, &pcslstr_apnname);
		
		KNI_NewString(pcsl_string_get_utf16_data(&pcslstr_apnname), apnname_len, apnnameHandle);

		pcsl_string_free(&pcslstr_apnname);
	} else {
		KNI_ThrowNew(KNIIOException, "Get current APN name error");
	}
	
	KNI_EndHandlesAndReturnObject(apnnameHandle);
}

KNIEXPORT
KNI_RETURNTYPE_OBJECT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getCurrentAPNUsername0()
{
	int	username_len;
	javacall_cellular_apn_info apn;
	pcsl_string pcslstr_username;

	KNI_StartHandles(1);
    KNI_DeclareHandle(usernameHandle);

	if (JAVACALL_OK == javacall_cellular_get_accesspoint(JAVACALL_DEFAULT_SUBSCRIBER_SLOT, &apn)) {
		username_len = javautil_strlen(apn.username);
		pcsl_string_convert_from_utf8((const jbyte*)apn.username, username_len, &pcslstr_username);
		
		KNI_NewString(pcsl_string_get_utf16_data(&pcslstr_username), username_len, usernameHandle);

		pcsl_string_free(&pcslstr_username);
	} else {
		KNI_ThrowNew(KNIIOException, "Get current APN user name error");
	}
	
	KNI_EndHandlesAndReturnObject(usernameHandle);

}

KNIEXPORT
KNI_RETURNTYPE_OBJECT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getCurrentAPNPassword0()
{
	int	password_len;
	javacall_cellular_apn_info apn;
	pcsl_string pcslstr_password;

	KNI_StartHandles(1);
    KNI_DeclareHandle(passwordHandle);

	if (JAVACALL_OK == javacall_cellular_get_accesspoint(JAVACALL_DEFAULT_SUBSCRIBER_SLOT, &apn)) {
		password_len = javautil_strlen(apn.password);
		pcsl_string_convert_from_utf8((const jbyte*)apn.password, password_len, &pcslstr_password);
		
		KNI_NewString(pcsl_string_get_utf16_data(&pcslstr_password), password_len, passwordHandle);

		pcsl_string_free(&pcslstr_password);
	} else {
		KNI_ThrowNew(KNIIOException, "Get current APN password error");
	}
	
	KNI_EndHandlesAndReturnObject(passwordHandle);

}


KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getLAC0()
{
	javacall_cellular_cell_info info;

	if (JAVACALL_OK == javacall_celluar_get_cell_info(JAVACALL_DEFAULT_SUBSCRIBER_SLOT, &info)) {
		KNI_ReturnInt(info.lac);
	} else {
		KNI_ReturnInt(-1);
	}
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getMNC0()
{
	javacall_cellular_network_info info;

	if (JAVACALL_OK == javacall_celluar_get_network_info(JAVACALL_DEFAULT_SUBSCRIBER_SLOT, &info)) {
		KNI_ReturnInt(info.network_id.mnc_mcc.mnc);
	} else {
		KNI_ReturnInt(-1);
	}	
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getMCC0()
{
	javacall_cellular_network_info info;

	if (JAVACALL_OK == javacall_celluar_get_network_info(JAVACALL_DEFAULT_SUBSCRIBER_SLOT, &info)) {
		KNI_ReturnInt(info.network_id.mnc_mcc.mcc);
	} else {
		KNI_ReturnInt(-1);
	}	
}

KNIEXPORT
KNI_RETURNTYPE_OBJECT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getIMSI0()
{
	javacall_const_ascii_string str;
	int str_len;
	pcsl_string pstr;

	KNI_StartHandles(1);
    KNI_DeclareHandle(strHandle);

	if ((NULL != (str = javacall_cellular_get_imsi(JAVACALL_DEFAULT_SUBSCRIBER_SLOT))) &&
		((str_len = javautil_strlen(str)) > 0)) {
		pcsl_string_convert_from_utf8((const jbyte*)str, str_len, &pstr);
		
		KNI_NewString(pcsl_string_get_utf16_data(&pstr), str_len, strHandle);

		pcsl_string_free(&pstr);
	} else {
		KNI_ThrowNew(KNIIOException, "Get IMSI error");
	}
	
	KNI_EndHandlesAndReturnObject(strHandle);

}

KNIEXPORT
KNI_RETURNTYPE_OBJECT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getIMEI0()
{
	javacall_const_ascii_string str;
	int str_len;
	pcsl_string pstr;

	KNI_StartHandles(1);
    KNI_DeclareHandle(strHandle);

	if ((NULL != (str = javacall_cellular_get_imei(JAVACALL_DEFAULT_SUBSCRIBER_SLOT))) &&
		((str_len = javautil_strlen(str)) > 0)) {
		pcsl_string_convert_from_utf8((const jbyte*)str, str_len, &pstr);
		
		KNI_NewString(pcsl_string_get_utf16_data(&pstr), str_len, strHandle);

		pcsl_string_free(&pstr);
	} else {
		KNI_ThrowNew(KNIIOException, "Get IMEI error");
	}
	
	KNI_EndHandlesAndReturnObject(strHandle);

}

KNIEXPORT
KNI_RETURNTYPE_OBJECT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getICCID0()
{
	javacall_const_ascii_string str;
	int str_len;
	pcsl_string pstr;

	KNI_StartHandles(1);
    KNI_DeclareHandle(strHandle);

	if ((NULL != (str = javacall_cellular_get_iccid(JAVACALL_DEFAULT_SUBSCRIBER_SLOT))) &&
		((str_len = javautil_strlen(str)) > 0)) {
		pcsl_string_convert_from_utf8((const jbyte*)str, str_len, &pstr);
		
		KNI_NewString(pcsl_string_get_utf16_data(&pstr), str_len, strHandle);

		pcsl_string_free(&pstr);
	} else {
		KNI_ThrowNew(KNIIOException, "Get ICCID error");
	}
	
	KNI_EndHandlesAndReturnObject(strHandle);
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getNetworkSignalLevel()
{
	KNI_ReturnInt(javacall_cellular_get_network_signal_level(JAVACALL_DEFAULT_SUBSCRIBER_SLOT));
}


