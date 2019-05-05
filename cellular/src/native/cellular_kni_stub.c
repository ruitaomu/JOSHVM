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

#define NOT_IMPLEMENTED_AND_RETURN_OBJECT 	KNI_StartHandles(1); \
    KNI_DeclareHandle(h); \
    KNI_ThrowNew(KNIIOException, "Not implemented!"); \
    KNI_EndHandlesAndReturnObject(h);

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_setAPN0()
{
	KNI_ThrowNew(KNIIOException, "Not implemented!"); 
	KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_OBJECT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getCurrentAPNName0()
{
	NOT_IMPLEMENTED_AND_RETURN_OBJECT
}

KNIEXPORT
KNI_RETURNTYPE_OBJECT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getCurrentAPNUsername0()
{
	NOT_IMPLEMENTED_AND_RETURN_OBJECT
}

KNIEXPORT
KNI_RETURNTYPE_OBJECT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getCurrentAPNPassword0()
{
	NOT_IMPLEMENTED_AND_RETURN_OBJECT
}


KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getLAC0()
{
	KNI_ReturnInt(0);
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getMNC0()
{
	KNI_ReturnInt(0);
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getMCC0()
{
	KNI_ReturnInt(0);
}

KNIEXPORT
KNI_RETURNTYPE_OBJECT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getIMSI0()
{
	NOT_IMPLEMENTED_AND_RETURN_OBJECT
}

KNIEXPORT
KNI_RETURNTYPE_OBJECT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getIMEI0()
{
	NOT_IMPLEMENTED_AND_RETURN_OBJECT
}

KNIEXPORT
KNI_RETURNTYPE_OBJECT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getICCID0()
{
	NOT_IMPLEMENTED_AND_RETURN_OBJECT
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_cellular_CellularDeviceInfo_getNetworkSignalLevel()
{
	KNI_ReturnInt(100);
}



