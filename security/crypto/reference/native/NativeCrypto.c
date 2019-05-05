/*
 *   
 *
 * Copyright  1990-2007 Sun Microsystems, Inc. All Rights Reserved.
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
 *
 * IMPL_NOTE:file definition
 */

/*
 * WARNING - THIS IS AN EXPERIMENTAL FEATURE OF KVM THAT MAY, OR MAY NOT
 * EXIST IN A FUTURE VERSION OF THIS PRODUCT. IT IS NOT A PART OF THE
 * CLDC SPECIFICATION AND IS PROVIDED FOR ILLUSTRATIVE PURPOSES ONLY
 */

#include <kni.h>
#include <sni.h>
#include <sni_event.h>
#include <kni_globals.h>
#include <bn.h>
#if ENABLE_PCSL
#include <javacall_security.h>
#include <pcsl_memory.h>
#endif

#define MAX(x,y) (((x)>(y))?(x):(y))

typedef struct {
	unsigned char* bufData;
	unsigned char* bufExp;
	unsigned char* bufMod;
	int dataLen;
	int expLen;
	int modLen;
	int resLen;
	int maxLen;
	BIGNUM* d;
}BN_MODEXP_OPER_HANDLE;


static void BN_mod_exp_mont_finish(BN_MODEXP_OPER_HANDLE* handle);
static BN_MODEXP_OPER_HANDLE* BN_mod_exp_mont_start(unsigned char* bufData, int dataLen, 
	                                                  unsigned char* bufExp, int expLen, 
	                                                  unsigned char* bufMod, int modLen,
	                                                  int resLen, int maxLen);

#if ENABLE_PCSL
extern void BN_mod_exp_mont_do(BN_MODEXP_OPER_HANDLE* handle);
#endif

/*=========================================================================
 * FUNCTION:      modExp([B[B[B[B[B)V (STATIC)
 * CLASS:         com/sun/midp/crypto/RSA
 * TYPE:          static native function
 * OVERVIEW:      Perform modular exponentiation.
 * INTERFACE (operand stack manipulation):
 *   parameters:  data      contains the data on which exponentiation is to
 *                           be performed
 *                exponent  contains the exponent, e.g. 65537 (decimal) is 
 *                           written as a three-byte array containing 
 *                           0x01, 0x00, 0x01
 *                modulus   contains the modulus
 *                result    the result of the modular exponentiation is 
 *                           returned in this array
 *   returns: the length of the result
 *=======================================================================*/
KNIEXPORT KNI_RETURNTYPE_INT
Java_com_sun_midp_crypto_RSA_modExp() {
#if ENABLE_PCSL
    jint dataLen, expLen, modLen, resLen;
    jint maxLen;
    INTEGER numbytes = 0;
    unsigned char *bufRes, *bufMod, *bufExp, *bufData;
	BN_MODEXP_OPER_HANDLE* handle;
	SNIReentryData* info;
	int waitFlag = 0;

    KNI_StartHandles(4);

    KNI_DeclareHandle(ires);
    KNI_DeclareHandle(imod);
    KNI_DeclareHandle(iexp);
    KNI_DeclareHandle(idata);

	
    KNI_GetParameterAsObject(4, ires);
    KNI_GetParameterAsObject(3, imod);
    KNI_GetParameterAsObject(2, iexp);
    KNI_GetParameterAsObject(1, idata);

    resLen    = KNI_GetArrayLength(ires);
    modLen    = KNI_GetArrayLength(imod);
    expLen    = KNI_GetArrayLength(iexp);
    dataLen   = KNI_GetArrayLength(idata);

	/* Find which parameter is largest and allocate that much space */
    maxLen = MAX(MAX(MAX(resLen, modLen), expLen), dataLen);
	
	info = (SNIReentryData*)SNI_GetReentryData(NULL);
	if (info == NULL) {
	    
	    if (maxLen > (BN_MAX_INTEGER / 8)) {
	        /* The number of BITS must fit in a BN integer. */
	        KNI_ThrowNew(KNIIllegalArgumentException, "arg too long");
			numbytes = -1;
	    } else {
			bufMod = (unsigned char *) pcsl_mem_malloc(modLen * sizeof(unsigned char));
			bufExp = (unsigned char *) pcsl_mem_malloc(expLen * sizeof(unsigned char));
			bufData = (unsigned char *) pcsl_mem_malloc(dataLen * sizeof(unsigned char));

			if (!bufMod || !bufExp || !bufData) {
				if (bufMod) pcsl_mem_free(bufMod);
				if (bufExp) pcsl_mem_free(bufExp);
				if (bufData) pcsl_mem_free(bufData);
				
				KNI_ThrowNew(KNIOutOfMemoryError, NULL);
			} else {

	            KNI_GetRawArrayRegion(idata, 0, dataLen, (jbyte*)bufData);            
	            KNI_GetRawArrayRegion(iexp, 0, expLen, (jbyte*)bufExp);
	            KNI_GetRawArrayRegion(imod, 0, modLen, (jbyte*)bufMod);

				if (NULL != (handle = BN_mod_exp_mont_start(bufData, dataLen, bufExp, expLen, bufMod, modLen, resLen, maxLen))) {
					if (handle->d) {
						/* Covert result from BIGNUM d to char array */
						bufRes = (unsigned char *) pcsl_mem_malloc(maxLen * sizeof(unsigned char));
					    numbytes = BN_bn2bin(handle->d, bufRes);
					    KNI_SetRawArrayRegion(ires, 0, numbytes, (jbyte*)bufRes);
						pcsl_mem_free(bufRes);
						BN_mod_exp_mont_finish(handle);
					} else {
						waitFlag = 1;
						SNIEVT_wait(BN_CALC_COMP_SIGNAL, (int)handle, 0);
					}
				} else {
					pcsl_mem_free(bufMod);
					pcsl_mem_free(bufExp);
					pcsl_mem_free(bufData);
	                /* assume out of mem */
	                KNI_ThrowNew(KNIOutOfMemoryError, "Mod Exp start");
				}
				
				
	        }
	    }
    } else {
    	handle = (BN_MODEXP_OPER_HANDLE*)info->descriptor;
		if (handle) {
			if (handle->d) {
				/* Covert result from BIGNUM d to char array */
				bufRes = (unsigned char *) pcsl_mem_malloc(maxLen * sizeof(unsigned char));
			    numbytes = BN_bn2bin(handle->d, bufRes);
			    KNI_SetRawArrayRegion(ires, 0, numbytes, (jbyte*)bufRes);
				pcsl_mem_free(bufRes);
			}
			
			BN_mod_exp_mont_finish(handle);
		}
	}

    KNI_EndHandles();

	if (numbytes == 0 && !waitFlag) {
		KNI_ThrowNew(KNIOutOfMemoryError, "Mod Exp");
	}

    KNI_ReturnInt((jint)numbytes);
#else
	KNI_ReturnInt((jint)0);
#endif
}

#if ENABLE_PCSL

static BN_MODEXP_OPER_HANDLE* BN_mod_exp_mont_start(unsigned char* bufData, int dataLen, 
	                                                  unsigned char* bufExp, int expLen, 
	                                                  unsigned char* bufMod, int modLen,
	                                                  int resLen, int maxLen) {
	javacall_result res;

	BN_MODEXP_OPER_HANDLE* handle = (BN_MODEXP_OPER_HANDLE *)pcsl_mem_malloc(sizeof(BN_MODEXP_OPER_HANDLE));
	if (handle == NULL) {
		return NULL;
	}
	
	handle->bufData = bufData;
	handle->dataLen = dataLen;
	handle->bufExp = bufExp;
	handle->expLen = expLen;
	handle->bufMod = bufMod;
	handle->modLen = modLen;
	handle->resLen = resLen;
	handle->maxLen = maxLen;
	handle->d = NULL;

	res = javacall_secure_modexp(handle);
	if (JAVACALL_NOT_IMPLEMENTED== res) {
		BN_mod_exp_mont_do(handle);
		if (handle->d == NULL) {
			pcsl_mem_free(handle);
			return NULL;
		} else {
			return handle;
		}
	} else if ((JAVACALL_OK == res) || (JAVACALL_WOULD_BLOCK == res)) {
		return handle;
	} else {
		//Failed
		pcsl_mem_free(handle);
		return NULL;
	}
}

static void BN_mod_exp_mont_finish(BN_MODEXP_OPER_HANDLE* handle) {
	//Free all buffers, d and handle itself
	if(handle->bufData) pcsl_mem_free(handle->bufData);
	if(handle->bufExp) pcsl_mem_free(handle->bufExp);
	if(handle->bufMod) pcsl_mem_free(handle->bufMod);
	
	BN_free(handle->d);
	handle->d=NULL;
	handle->bufData=NULL;
	handle->bufExp=NULL;
	handle->bufMod=NULL;
	
	pcsl_mem_free(handle);
}

void BN_mod_exp_mont_do(BN_MODEXP_OPER_HANDLE* handle) {

    BIGNUM *a, *b, *c, *d;
	BN_CTX *ctx;

	if (handle->d) {
		return;
	}
	
	a = BN_bin2bn(handle->bufData, (INTEGER)handle->dataLen, NULL);
	b = BN_bin2bn(handle->bufExp, (INTEGER)handle->expLen, NULL);
	c = BN_bin2bn(handle->bufMod, (INTEGER)handle->modLen, NULL);
	d = BN_new((INTEGER)handle->resLen);
	ctx = BN_CTX_new((INTEGER)handle->maxLen);
	/* do the actual exponentiation */
	if (a != NULL && b != NULL && c != NULL && d != NULL &&
		ctx != NULL && BN_mod_exp_mont(d,a,b,c,ctx)) {
	} else {
		BN_free(d);
		d = NULL;		
	}
					
	BN_free(a);
	BN_free(b);
	BN_free(c);
	
	BN_CTX_free(ctx);

	handle->d = d;
}

#endif

