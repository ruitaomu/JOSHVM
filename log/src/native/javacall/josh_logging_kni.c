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

/*
 *
 * Copyright  1990-2009 Sun Microsystems, Inc. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 only, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is
 * included at /legal/license.txt).
 * 
 * You should have received a copy of the GNU General Public License
 * version 2 along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 * 
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa
 * Clara, CA 95054 or visit www.sun.com if you need additional
 * information or have any questions.
 */

/**
 * @file
 * The glue code between the Java interface to the
 * logging/tracing service and the native provider code
 * defined in midp_logging.h and implemented in logging.c
 */
#include <stdio.h>
#include <kni.h>
#include <josh_logging.h>
#include <pcsl_string.h>
#include <javacall_logging.h>

/**========================================================================
 * FUNCTION:      report(II,java/lang/String)V
 * CLASS:         com.sun.midp.services.Logging
 * TYPE:          virtual native function
 * OVERVIEW:      calls a system specific report function
 * INTERFACE (operand stack manipulation):
 *   parameters:  severity   constant shows severity of the error message
 *                channelID  allows log to be parsed by area/module
 *                message    String to be appended to the report log
 *   returns:     <nothing>
 *=======================================================================*/
KNIEXPORT KNI_RETURNTYPE_VOID
KNIDECL(com_sun_midp_log_LoggingBase_report) {
    reportToLog(LOG_ERROR, 0, "%s", "LoggingBase.report was called directly\n");
    KNI_ReturnVoid();
}

KNIEXPORT KNI_RETURNTYPE_VOID
KNIDECL(com_sun_midp_log_Logging_report0) {
    KNI_StartHandles(1);
	KNI_DeclareHandle(message_str);
	KNI_GetParameterAsObject(3, message_str);
    {
        const jbyte * tmpmsg;
		jsize msglen;
		int channelID;
		int severity;
		pcsl_string pstr;
		static jchar buf[256]={(jchar)0};
		
		msglen = KNI_GetStringLength(message_str);
		if (msglen > 255) msglen = 255;
		KNI_GetStringRegion(message_str, 0, msglen, buf);
		if (pcsl_string_convert_from_utf16(buf, msglen, &pstr) == PCSL_STRING_OK) {
			tmpmsg = pcsl_string_get_utf8_data(&pstr);
			if (NULL != tmpmsg) {
			
		        channelID = KNI_GetParameterAsInt(2);
		        severity = KNI_GetParameterAsInt(1);

		        /*
		         * This call format is  needed because if s contains %,
		         * printf(s) fails, but
		         * printf("%s", s) is safe
		         */
		        reportToLog(severity, channelID, "%s", tmpmsg);

		        pcsl_string_release_utf8_data(tmpmsg,&pstr);
			}
			pcsl_string_free(&pstr);
		}
    }
    KNI_EndHandles();
    KNI_ReturnVoid();
}

KNIEXPORT KNI_RETURNTYPE_VOID
KNIDECL(com_sun_midp_log_Logging_setReportLevel0) {
	int report_level = KNI_GetParameterAsInt(1);
	javacall_logging_setLevel(report_level);
	KNI_ReturnVoid();
}

KNIEXPORT KNI_RETURNTYPE_INT
KNIDECL(com_sun_midp_log_Logging_getAllowedSeverity) {
    int severity;
    int channelID; 

    channelID = KNI_GetParameterAsInt(1);
    severity = get_allowed_severity_c(channelID);

    KNI_ReturnInt(severity);
}
