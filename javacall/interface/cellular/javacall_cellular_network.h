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
#include <javacall_defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file javacall_cellular_network.h
 * @ingroup DIO
 * @brief Javacall interfaces for Device IO
 */

/**
 * @def JAVACALL_MAX_SUBSCRIBERS
 * Maximal amount of a subscribers
 */
#define JAVACALL_MAX_SUBSCRIBERS    8

/**
 * @def JAVACALL_MAX_OPERATOR_NAME_LENGTH
 * Maximal length of a Subscriber operator name
 */
#define JAVACALL_MAX_OPERATOR_NAME_LENGTH    64

/**
 * @def JAVACALL_MAX_PHONE_NUMBER_LENGTH
 * Maximal length of a Subscriber phone number
 */
#define JAVACALL_MAX_PHONE_NUMBER_LENGTH    16


/**
 * @def JAVACALL_DEFAULT_SUBSCRIBER_SLOT
 * default subscriber slot number
 */
#define JAVACALL_DEFAULT_SUBSCRIBER_SLOT    0

/**
 * @def JAVACALL_MAX_APN_NAME_LENGTH
 * Maximal length of APN name
 */
#define JAVACALL_MAX_APN_NAME_LENGTH 99

/**
 * @def JAVACALL_MAX_APN_USERNAME_LENGTH
 * Maximal length of APN user name
 */
#define JAVACALL_MAX_APN_USERNAME_LENGTH 64

/**
 * @def JAVACALL_MAX_APN_PASSWORD_LENGTH
 * Maximal length of APN password
 */
#define JAVACALL_MAX_APN_PASSWORD_LENGTH 64

/**
 * @enum javacall_network_type
 * @brief cellular network type
 */
typedef enum {
    
    JAVACALL_CELLULAR_NETWORK_TYPE_3GPP  = 81,
    
    JAVACALL_CELLULAR_NETWORK_TYPE_CDMA  = 82 

} javacall_network_type;


/**
 * javacall_network_id shoud be unique for different cellular networks per 
 * subscriber. One of the way to do that - combine mcc, mnc and slot number for
 * ID generation.
 *
 * @union javacall_network_id
 * @brief SID and NID in CDMA case, MCC amd MNC in 3GPP case
 */
typedef union {

    struct {
        /** MCC (Mobile Country Code) number of the network */
        javacall_int16 mcc;

        /** MNC (Mobile Network Code) number of the network */
        javacall_int16 mnc;

    } mnc_mcc;

    struct {
        /** SID (System Identifier) number of the network */
        javacall_int16 sid;

        /** NID (Network Identifier) number of the network */
        javacall_int16 nid;

    } sid_nid;

    javacall_int32 id;

} javacall_network_id; 

/**
 * @struct javacall_cellular_network_info
 * @brief Holds the cellular network related information.
 */
typedef struct {

    /** Cellular network id */
    javacall_network_id network_id;

    /** Network display name: null terminated UTF-8 string */
    javacall_uint8 name[JAVACALL_MAX_OPERATOR_NAME_LENGTH];

    /** Network type */
    javacall_network_type network_type;

} javacall_cellular_network_info;

/**
 * @struct javacall_cellular_cell_info
 * @brief Holds the cell related information.
 */
typedef struct {

    int lac;
	int cell_id;
} javacall_cellular_cell_info;

/**
 * @struct javacall_cellular_apn_info
 * @brief Holds the APN setting information.
 */
typedef struct {
	/** Must be '\0' terminated string **/
	char apnname[JAVACALL_MAX_APN_NAME_LENGTH];

	/** Must be '\0' terminated string **/
	char username[JAVACALL_MAX_APN_USERNAME_LENGTH];

	/** Must be '\0' terminated string **/
	char password[JAVACALL_MAX_APN_PASSWORD_LENGTH];
} javacall_cellular_apn_info;

/**
 * Set APN for specified subscriber
 * @param subscriber_slot the subscriber slot number for MultiSim devices or
 *                     JAVACALL_DEFAULT_SUBSCRIBER_SLOT for SingleSIM device
 * 
 * @param apn structure pointer of APN setting information
 * @return <tt>JAVACALL_OK</tt> Setting succeed
 *         <tt>JAVACALL_FAIL</tt> or failed setting APN
 */
javacall_result javacall_cellular_set_accesspoint(javacall_int32 subscriber_slot, javacall_cellular_apn_info* apn);

/**
 * Get APN for specified subscriber
 * @param subscriber_slot the subscriber slot number for MultiSim devices or
 *                     JAVACALL_DEFAULT_SUBSCRIBER_SLOT for SingleSIM device
 * 
 * @param apn structure pointer to hold the returned APN setting information
 * @return <tt>JAVACALL_OK</tt> if succeed
 *         <tt>JAVACALL_FAIL</tt> or failed to get the APN information
 */
javacall_result javacall_cellular_get_accesspoint(javacall_int32 subscriber_slot, /*OUT*/javacall_cellular_apn_info* apn);

/**
 * Get IMSI for specified subscriber
 * @param subscriber_slot the subscriber slot number for MultiSim devices or
 *                     JAVACALL_DEFAULT_SUBSCRIBER_SLOT for SingleSIM device
 * 
 * @return the immutable string of IMSI
 */
javacall_const_ascii_string javacall_cellular_get_imsi(javacall_int32 subscriber_slot);

/**
 * Get IMEI for specified subscriber
 * @param subscriber_slot the subscriber slot number for MultiSim devices or
 *                     JAVACALL_DEFAULT_SUBSCRIBER_SLOT for SingleSIM device
 * 
 * @return the immutable string of IMEI
 */
javacall_const_ascii_string javacall_cellular_get_imei(javacall_int32 subscriber_slot);

/**
 * Get ICCID for specified subscriber
 * @param subscriber_slot the subscriber slot number for MultiSim devices or
 *                     JAVACALL_DEFAULT_SUBSCRIBER_SLOT for SingleSIM device
 * 
 * @return the immutable string of ICCID
 */
javacall_const_ascii_string javacall_cellular_get_iccid(javacall_int32 subscriber_slot);

/**
 * Get current reception signal level
 * @param subscriber_slot the subscriber slot number for MultiSim devices or
 *                     JAVACALL_DEFAULT_SUBSCRIBER_SLOT for SingleSIM device
 * 
 * @return the reception signal level
 */
javacall_int32 javacall_cellular_get_network_signal_level(javacall_int32 subscriber_slot);

/**
 * Get current celluar network information
 * @param subscriber_slot the subscriber slot number for MultiSim devices or
 *                     JAVACALL_DEFAULT_SUBSCRIBER_SLOT for SingleSIM device
 * @param the structure point to hold the returned cellular network infomation
 * @return <tt>JAVACALL_OK</tt> if succeed
 *         <tt>JAVACALL_FAIL</tt> or failed to get current celluar network infomation
 */
 javacall_result javacall_celluar_get_network_info(javacall_int32 subscriber_slot, /*OUT*/javacall_cellular_network_info* info);

/**
 * Get current cell information
 * @param subscriber_slot the subscriber slot number for MultiSim devices or
 *                     JAVACALL_DEFAULT_SUBSCRIBER_SLOT for SingleSIM device
 * @param the structure point to hold the returned cell infomation
 * @return <tt>JAVACALL_OK</tt> if succeed
 *         <tt>JAVACALL_FAIL</tt> or failed to get current cell information
 */
javacall_result javacall_celluar_get_cell_info(javacall_int32 subscriber_slot, /*OUT*/javacall_cellular_cell_info* info);

/**
 * Initialize celluar network system. Will be called ONCE when VM start.
 */
javacall_result javacall_cellular_init();

/**
 * Deinitialize celluar network system. Will be called ONCE when VM finish.
 */
javacall_result javacall_cellular_deinit();

/** @} */

#ifdef __cplusplus
}
#endif


