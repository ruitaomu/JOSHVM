/*
 *
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

package org.joshvm.security.internal;

import java.util.Hashtable;
import java.util.Vector;

/**
 * This class is a standard list of permissions that
 * a suite can do and is used by all internal security
 * features. This class also builds a list of permission for each
 * security domain. This only class that would need to be updated in order to
 * add a new security domain.
 */
public final class Permissions {

    /** Name of the JOSHVM permission. */
    public static final String VM_PERMISSION_NAME = "org.joshvm";

    /** Name of the AMS permission. */
    public static final String AMS_PERMISSION_NAME = "org.joshvm.ams";

    /** Binding name of the Manufacturer domain. (all permissions allowed) */
    public static final String MANUFACTURER_DOMAIN_BINDING = "manufacturer";

    /** com.sun.midp permission ID. */
    public static final int JOSHVM = 0;

    /** com.sun.midp.ams permission ID. */
    public static final int AMS = 1;

    /** Deny the permission. */
    public static final byte DENY = 0;

    /** Allow an permission with out asking the user. */
    public static final byte ALLOW = 1;

    /** Number of permissions. */
    public static int NUMBER_OF_PERMISSIONS = -1;

    /* Permissions index lookup table */
    private static Hashtable permissionsHash;

    /**
     * Get the ID of a permission.
     *
     * @param name permission name
     *
     * @return permission ID
     *
     * @exception SecurityException if the permission is invalid
     */
    public static int getId(String name) {
        int index;
		if (permissionsHash == null) {
			init();
		}
		
        try {
            index = ((Integer)permissionsHash.get(name)).intValue();
            return index;
        } catch (Exception e){
            throw new SecurityException(SecurityToken.STD_EX_MSG);
        }
    }

    /**
     * Create a list of permission groups a domain is permitted to perform.
     *
     * @param name binding name of domain
     *
     * @return array of permission settings
     */
    public static byte[] forDomain(String name) {
    	if (NUMBER_OF_PERMISSIONS == -1) {
			init();
		}
		
        byte[] permissions = new byte[NUMBER_OF_PERMISSIONS];

        if (MANUFACTURER_DOMAIN_BINDING.equals(name)) {
            for (int i1 = 0; i1 < NUMBER_OF_PERMISSIONS; i1 ++) {
                permissions[i1] = ALLOW;
            }
            return permissions;
        }
		
        return getEmptySet();
    }

    /**
     * Create an empty list of permission groups.
     *
     * @return array containing the empty permission groups
     */
    public static byte[] getEmptySet() {
    	if (NUMBER_OF_PERMISSIONS == -1) {
			init();
		}

        byte[] permissions = new byte[NUMBER_OF_PERMISSIONS];

        // Assume perms array is non-null
        for (int i = 0; i < permissions.length; i++) {
            // This is default permission
            permissions[i] = DENY;
        }

        return permissions;
    }

	private static void init() {
		String [] permList = PermissionsStrings.PERMISSION_STRINGS;
		
		NUMBER_OF_PERMISSIONS = permList.length + 2;

		permissionsHash = new Hashtable(NUMBER_OF_PERMISSIONS);
		permissionsHash.put(VM_PERMISSION_NAME, new Integer(0));
		permissionsHash.put(AMS_PERMISSION_NAME, new Integer(1));

		for (int i1 = 2; i1 < NUMBER_OF_PERMISSIONS; i1++) {
			permissionsHash.put(permList[i1 - 2], new Integer(i1));
		}
	}
	
    private static String replaceCRLF(String value) {
        int posCRLF, pos = 0;
        String result = "";

        while ((posCRLF = value.indexOf("\\n", pos)) != -1) {
            result += value.substring(pos,  posCRLF) + "\n";
            pos = posCRLF + 2;
        }

        return result + value.substring(pos);
    }

    /**
     * Removes the given strings from the specified string array.
     *
     * @param srcArray source array of strings
     * @param elementsToRemove array of strings to remove
     *
     * @return a new array containing those strings from srcArray
     *         that are not contained in elementsToRemove
     */
    private static String[] removeElementsFromArray(String[] srcArray,
                                                    String[] elementsToRemove) {
        if (srcArray == null) {
            return null;
        }

        /*
         * This method is internal, so we can guarantee that it is never
         * called with elementsToRemove == null.
         */

        int i, j;
        Vector res = new Vector(srcArray.length);

        for (i = 0; i < srcArray.length; i++) {
            for (j = 0; j < elementsToRemove.length; j++) {
                if (srcArray[i].equals(elementsToRemove[j])) {
                    break;
                }
            }

            if (j < elementsToRemove.length) {
                continue;
            }
            res.addElement(srcArray[i]);
        }

        String[] dstArray = new String[res.size()];
        for (i = 0; i < res.size(); i++) {
            dstArray[i] = (String)res.elementAt(i);
        }

        return dstArray;
    }

}

