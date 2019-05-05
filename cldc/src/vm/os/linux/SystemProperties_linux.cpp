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

#include "incls/_precompiled.incl"
#include "incls/_SystemProperties_linux.cpp.incl"

struct SystemProperty {
   SystemProperty * next;
   char *name;
   char *value;
};

static SystemProperty * system_properties = NULL;
static SystemProperty * internal_properties = NULL;

static void setProperty(const char *property_name, const char *property_value, SystemProperty** list) {

  SystemProperty *prop =
      (SystemProperty*)OsMemory_allocate(sizeof(SystemProperty));
  prop->next = *list;
  *list = prop;

  prop->name  = (char*)OsMemory_allocate(jvm_strlen(property_name + 1));
  prop->value = (char*)OsMemory_allocate(jvm_strlen(property_value + 1));
  jvm_strcpy(prop->name,  property_name);
  jvm_strcpy(prop->value, property_value);
}

static char *getProperty(const char *property_name, SystemProperty* list) {
  SystemProperty *prop;

  for (prop = list; prop; prop = prop->next) {
    if (jvm_strcmp(property_name, prop->name) == 0) {
      return prop->value;
    }
  }
  return NULL;
}

/**
 * Sets a property key to the specified value in the internal
 * property set.
 *
 * @param key The key to set
 * @param value The value to set <tt>key</tt> to
 */
void setInternalProperty(const char* key , const char* value) {
	setProperty(key, value, &internal_properties);    
}

/**
 * Sets a property key to the specified value in the application
 * property set.
 *
 * @param key The key to set
 * @param value The value to set <tt>key<tt> to
 */
void setSystemProperty(const char *key, const char *value) {
	setProperty(key, value, &system_properties);
}


/**
 * Gets the value of the specified property key in the internal
 * property set. If the key is not found in the internal property
 * set, the application  property set is searched.
 *
 * @param key The key to search for
 *
 * @return The value associated with <tt>key</tt> if found, otherwise
 *         <tt>NULL</tt>
 */
const char* getInternalProperty(const char* key) {
	return getProperty(key, internal_properties);

}

/**
 * Gets the value of the specified property key in the application
 * property set.
 *
 * @param key The key to search for
 *
 * @return The value associated with <tt>key<tt> if found, otherwise
 *         <tt>NULL<tt>
 */
const char *getSystemProperty(const char *key) {
	return getProperty(key, system_properties);
}

/**
 * Gets the integer value of the specified property key in the internal
 * property set.  
 *
 * @param key The key to search for
 *
 * @return The value associated with <tt>key</tt> if found, otherwise
 *         <tt>0</tt>
 */
int getInternalPropertyInt(const char* key) {
    const char *tmp;    

    tmp = getInternalProperty(key); 

    return(NULL == tmp) ? 0 : atoi(tmp);
}


