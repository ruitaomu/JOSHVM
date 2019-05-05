/*
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
#include <kni.h>

//-----------------------------------------------------------------------------

/*
 * public native void create() throws IOException;
 *
 * Note: 'file is not directory' is checked in J2ME code
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_create()
{
    KNI_ReturnVoid();
}

/*
 * public native void delete() throws IOException;
 *
 * Note: input stream, output stream and file itself are closed in J2ME code
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_delete()
{
    KNI_ReturnVoid();
}

/*
 * private native void rename0(String newName) throws IOException;
 *
 * Note: input stream, output stream and file itself are closed in J2ME code.
 * newName is not null, newName contains no path specification and newName
 * is an valid filename for the platform are also checked in J2ME code.
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_rename0()
{
    KNI_ReturnVoid();
}

/*
 * public native void truncate(long byteOffset) throws IOException;
 *
 * Note: byteOffset is geater than zero is checked in J2ME code.
 * Open write stream is also flushed in J2ME code.
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_truncate()
{
    KNI_ReturnVoid();
}

/*
 * public native long fileSize() throws IOException;
 * 
 * Note: 'file is not directory' is checked in J2ME code.
 */
KNIEXPORT KNI_RETURNTYPE_LONG
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_fileSize()
{
    KNI_ReturnLong(0);
}

/*
 * public native long directorySize(boolean includeSubDirs) throws IOException;
 * 
 * Note: 'file is a directory' is checked in J2ME code.
 */
KNIEXPORT KNI_RETURNTYPE_LONG
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_directorySize()
{
    KNI_ReturnLong(-1);
}

/*
 * public native void mkdir() throws IOException;    
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_mkdir()
{
    KNI_ReturnVoid();
}

/*
 * public native boolean exists();
 */
KNIEXPORT KNI_RETURNTYPE_BOOLEAN
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_exists()
{
    KNI_ReturnBoolean(KNI_FALSE);
}

/*
 * public native boolean isDirectory();
 */
KNIEXPORT KNI_RETURNTYPE_BOOLEAN
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_isDirectory()
{
    KNI_ReturnBoolean(KNI_FALSE);
}

/*
 * public native boolean canRead();
 */
KNIEXPORT KNI_RETURNTYPE_BOOLEAN
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_canRead()
{
    KNI_ReturnBoolean(KNI_FALSE);
}

/*
 * public native void setReadable(boolean readable);
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_setReadable()
{
    KNI_ReturnVoid();
}

/*
 * public native boolean canWrite();
 */
KNIEXPORT KNI_RETURNTYPE_BOOLEAN
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_canWrite()
{
    KNI_ReturnBoolean(KNI_FALSE);
}

/*
 * public native void setWritable(boolean writable);
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_setWritable()
{
    KNI_ReturnVoid();
}

/*
 * private native boolean isHidden();
 */
KNIEXPORT KNI_RETURNTYPE_BOOLEAN
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_isHidden0()
{
    KNI_ReturnBoolean(KNI_FALSE);
}

/*
 * private native void setHidden0(boolean hidden);
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_setHidden0()
{
    KNI_ReturnVoid();
}

/*
 * private native long openDir(); 
 */
KNIEXPORT KNI_RETURNTYPE_LONG
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_openDir()
{
    KNI_ReturnLong(0);
}

/*
 * private native void closeDir(long dirHandle);
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_closeDir()
{    
    KNI_ReturnVoid();
}


/*
 * private native String dirGetNextFile(long dirHandle, boolean includeHidden);
 *
 * Note: file filter is validated and applied in J2ME code
 */
KNIEXPORT KNI_RETURNTYPE_OBJECT
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_dirGetNextFile()
{
    return((jobject)0);
}

/*
 * private native String getMountedRoots();
 */
KNIEXPORT KNI_RETURNTYPE_OBJECT
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_getMountedRoots(void)
{
    return((jobject)0);
}


/*
 * private native String getNativePathForRoot(String root);
 */
KNIEXPORT KNI_RETURNTYPE_OBJECT
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_getNativePathForRoot()
{    
    return((jobject)0);
}


/*
 * public native long lastModified();
 */
KNIEXPORT KNI_RETURNTYPE_LONG
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_lastModified()
{
    KNI_ReturnLong(0);
}

/*
 * public native void openForRead() throws IOException;
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_openForRead()
{    
    KNI_ReturnVoid();
}

/*
 * public native void openForWrite() throws IOException;
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_openForWrite()
{
    KNI_ReturnVoid();
}

/*
 * public native int read(byte b[], int off, int len) throws IOException;
 */
KNIEXPORT KNI_RETURNTYPE_INT
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_read()
{
    KNI_ReturnInt(0);
}

/*
 * public native int write(byte b[], int off, int len) throws IOException;
 */
KNIEXPORT KNI_RETURNTYPE_INT
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_write()
{
    KNI_ReturnInt(0);
}

/*
 * public native void positionForWrite(long offset) throws IOException;
 *
 * Note: 'Offset has a negative value' is checked in J2ME code.
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_positionForWrite()
{
    KNI_ReturnVoid();
}


/*
 * public native void flush() throws IOException;
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_flush()
{
    KNI_ReturnVoid();
}

/*
 * public native long availableSize
 */
KNIEXPORT KNI_RETURNTYPE_LONG
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_availableSize()
{
    KNI_ReturnLong(-1);
}

/*
 * public native long totalSize
 */
KNIEXPORT KNI_RETURNTYPE_LONG
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_totalSize()
{
    KNI_ReturnLong(-1);
}

/*
 * public native void closeForRead() throws IOException;
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_closeForRead()
{
    KNI_ReturnVoid();
}

/*
 * public native void closeForWrite() throws IOException;
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_closeForWrite()
{
    KNI_ReturnVoid();
}

/*
 * public native void closeForReadWrite() throws IOException;
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_closeForReadWrite()
{
    KNI_ReturnVoid();
}

/*
 * public native void close() throws IOException;
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_close()
{
    KNI_ReturnVoid();
}

/*
 * private static native long getNativeName(String name, long oldName);
 */
KNIEXPORT KNI_RETURNTYPE_LONG
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_getNativeName()
{
    KNI_ReturnLong(0);
}

KNIEXPORT KNI_RETURNTYPE_OBJECT
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_illegalFileNameChars0()
{
    return((jobject)0);
}

/*
 * private static native char getFileSeparator();
 */
KNIEXPORT KNI_RETURNTYPE_CHAR
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_getFileSeparator()
{   
    KNI_ReturnChar((jchar)0);
}

/*
 * private native void finalize();
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_finalize()
{
    KNI_ReturnVoid();
}

/*
 * private native static void initialize();
 */
KNIEXPORT KNI_RETURNTYPE_VOID
Java_com_sun_cldc_io_j2me_file_DefaultFileHandler_initialize()
{
    KNI_ReturnVoid();
}
