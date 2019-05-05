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

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_directUI_DefaultDisplayDevice_getDisplayWidth0()
{
	KNI_ReturnInt(0);
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_directUI_DefaultDisplayDevice_getDisplayHeight0()
{
	KNI_ReturnInt(0);
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_directUI_DefaultDisplayDevice_clear0()
{
	KNI_ThrowNew(KNIIOException, "Not implemented!");
	KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_directUI_DefaultDisplayDevice_update0()
{
	KNI_ThrowNew(KNIIOException, "Not implemented!");
	KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_directUI_DefaultDisplayDevice_showText0()
{
	KNI_ThrowNew(KNIIOException, "Not implemented!");
	KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_directUI_DefaultDisplayDevice_showImage0()
{
	KNI_ThrowNew(KNIIOException, "Not implemented!");
	KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_directUI_DefaultDisplayDevice_flush0()
{
	KNI_ThrowNew(KNIIOException, "Not implemented!");
	KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_directUI_DefaultDisplayDevice_setBacklight0()
{
	KNI_ThrowNew(KNIIOException, "Not implemented!");
	KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_directUI_Text_getWidth0()
{
	KNI_ReturnInt(0);
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_directUI_Text_getHeight0()
{
	KNI_ReturnInt(0);
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_directUI_Image_getWidth0()
{
	KNI_ReturnInt(0);
}

KNIEXPORT
KNI_RETURNTYPE_INT
Java_org_joshvm_j2me_directUI_Image_getHeight0()
{
	KNI_ReturnInt(0);
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_directUI_KeypadThread_waitingForKeyEvent()
{
	KNI_ThrowNew(KNIIOException, "Not implemented!");
	KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_directUI_KeypadThread_initEvent()
{
    KNI_ReturnVoid();
}

KNIEXPORT
KNI_RETURNTYPE_VOID
Java_org_joshvm_j2me_directUI_KeypadThread_finalizeEvent()
{
    KNI_ReturnVoid();
}
