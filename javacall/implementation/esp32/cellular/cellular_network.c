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
#include <javacall_cellular_network.h>
#include <javacall_properties.h>
#include <esp_wifi.h>

javacall_result javacall_cellular_set_accesspoint(javacall_int32 subscriber_slot, javacall_cellular_apn_info* apn){
	javacall_set_property("system.network.pdp.apn", apn->apnname, 1, JAVACALL_INTERNAL_PROPERTY);
	javacall_set_property("system.network.pdp.username", apn->username, 1, JAVACALL_INTERNAL_PROPERTY);
	javacall_set_property("system.network.pdp.password", apn->password, 1, JAVACALL_INTERNAL_PROPERTY);

	return JAVACALL_OK;

}

javacall_result javacall_cellular_get_accesspoint(javacall_int32 subscriber_slot, /*OUT*/javacall_cellular_apn_info* apn){
	char *val1, *val2, *val3;
	if (JAVACALL_OK == javacall_get_property("system.network.pdp.apn", JAVACALL_INTERNAL_PROPERTY, &val1) && val1) {
		strncpy(apn->apnname, val1, JAVACALL_MAX_APN_NAME_LENGTH);
	} else {
		apn->apnname[0] = '\0';
	}
	
	if (JAVACALL_OK == javacall_get_property("system.network.pdp.username", JAVACALL_INTERNAL_PROPERTY, &val2) && val2) {
		strncpy(apn->username, val2, JAVACALL_MAX_APN_USERNAME_LENGTH);
	} else {
		apn->username[0] = '\0';
	}
	
	if (JAVACALL_OK == javacall_get_property("system.network.pdp.password", JAVACALL_INTERNAL_PROPERTY, &val3) && val3) {
		strncpy(apn->password, val3, JAVACALL_MAX_APN_PASSWORD_LENGTH);
	} else {
		apn->password[0] = '\0';
	}

	return JAVACALL_OK;

}


javacall_const_ascii_string javacall_cellular_get_imsi(javacall_int32 subscriber_slot){
	return "111111";
}

javacall_const_ascii_string javacall_cellular_get_imei(javacall_int32 subscriber_slot){
	uint8_t mac[6];
	static char mac_addr[19];
	esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
	snprintf(mac_addr, 18, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	javacall_printf("MAC: %s\n", mac_addr);
	return mac_addr;
}

javacall_const_ascii_string javacall_cellular_get_iccid(javacall_int32 subscriber_slot){
	return NULL;
}


javacall_int32 javacall_cellular_get_network_signal_level(javacall_int32 subscriber_slot){
	return JAVACALL_NOT_IMPLEMENTED;
}

javacall_result javacall_celluar_get_network_info(javacall_int32 subscriber_slot, /*OUT*/javacall_cellular_network_info* info){
	return JAVACALL_NOT_IMPLEMENTED;
}
javacall_result javacall_celluar_get_cell_info(javacall_int32 subscriber_slot, /*OUT*/javacall_cellular_cell_info* info){
	return JAVACALL_NOT_IMPLEMENTED;
}

javacall_result javacall_cellular_init(void) {
	return JAVACALL_OK;
}

javacall_result javacall_cellular_deinit(void) {
	return JAVACALL_OK;
}
