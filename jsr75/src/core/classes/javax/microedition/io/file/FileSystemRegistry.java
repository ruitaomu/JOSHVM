/*
 *   
 *
 * Portions Copyright  2000-2007 Sun Microsystems, Inc. All Rights
 * Reserved.  Use is subject to license terms.
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

/*
 * Copyright (C) 2002-2003 PalmSource, Inc.  All Rights Reserved.
 */

package javax.microedition.io.file;

import com.sun.cldc.io.j2me.file.Protocol;
import java.util.Enumeration;
import java.util.Vector;

import java.util.NoSuchElementException;

/**
 * This class is defined by the JSR-75 specification
 * <em>PDA Optional Packages for the J2ME&trade; Platform</em>
 */
// JAVADOC COMMENT ELIDED
public class FileSystemRegistry {

    /** Constructor. */
    FileSystemRegistry() {
    }

    // ALWAYS FAIL. NOT SUPPORTTED
    public static boolean addFileSystemListener(FileSystemListener listener) {
        if (listener == null) {
            throw new NullPointerException();
        }

        checkReadPermission();

        return false;
    }

    // ALWAYS FAIL. NOT SUPPORTTED
    public static boolean removeFileSystemListener(
            FileSystemListener listener) {
        if (listener == null) {
            throw new NullPointerException();
        }

        return false;
    }

    // JAVADOC COMMENT ELIDED
    public static Enumeration listRoots() {
        checkReadPermission();
        // retrieve up-to-date list of mounted roots
        return Protocol.listRoots().elements();
    }

    /**
     * Checks the read permission.
     * @throws SecurityException if read is not allowed
     */
    private static void checkReadPermission() {
        /**
		* No permission check!
		**/
    }
}
