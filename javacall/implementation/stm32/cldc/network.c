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
/**
 * @file
 *
 * Implementation of pcsl_network.h for platforms that support the winsock 
 * API.
 *
 * For all functions, the "handle" is the winsock handle (an int)
 * cast to void *.  Since winsock reads and writes to sockets are synchronous,
 * the context for reading and writing is always set to NULL.
 */

#include "javacall_network.h"
#include "javacall_socket.h"
#include "javacall_datagram.h"
#include "javacall_events.h"
#include "javacall_logging.h"

javacall_result javacall_socket_open(javacall_ip_version ip_version,
                                     javacall_handle* pHandle) {

	return JAVACALL_FAIL;
}

/**
 * See pcsl_network.h for definition.
 */

javacall_result javacall_socket_connect_start(
	javacall_handle handle,
    javacall_configuration_id config_id,
    javacall_ip_version ip_version,
    unsigned char *ipBytes, 
	int port, 
	void **pContext)
{
	return JAVACALL_FAIL;
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_connect_finish(javacall_handle handle,void *context){
	return JAVACALL_FAIL;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_read_start(javacall_handle handle,
                                           unsigned char *pData,
                                           int len,  
                                           int *pBytesRead,
                                           void **pContext){
                                           
	return JAVACALL_FAIL;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_read_finish(javacall_handle handle,
                                            unsigned char *pData,
                                            int len,
                                            int *pBytesRead,
                                            void *context){
	return JAVACALL_FAIL;
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_write_start(javacall_handle handle,
                                            char *pData,
                                            int len,
                                            int *pBytesWritten,
                                            void **pContext){

    return JAVACALL_FAIL;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_write_finish(javacall_handle handle,
                                             char *pData,
                                             int len,
                                             int *pBytesWritten,
                                             void *context){

	return JAVACALL_FAIL;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_available(javacall_handle handle, int *pBytesAvailable){

	return JAVACALL_FAIL;
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_shutdown_output(javacall_handle handle) {

	return JAVACALL_FAIL;
}


/**
 * See pcsl_network.h for definition.
 *
 * Note that this function NEVER returns PCSL_NET_WOULDBLOCK. Therefore, the 
 * finish() function should never be called and does nothing.
 */
javacall_result javacall_socket_close_start(javacall_handle handle, javacall_bool abort,
                                            void **pContext){

	return JAVACALL_FAIL;

}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_socket_close_finish(javacall_handle handle,
                                             void *context){
                                             
	return JAVACALL_FAIL;
}

javacall_result javacall_network_gethostbyname_start(char *hostname,
                                                     javacall_ip_version ip_version,
                                                     javacall_configuration_id id,
                                                     unsigned char *pAddress, 
                                                     int maxLen,
                                                     int *pLen,
                                                     void **pHandle,
                                                     void **pContext){

	return JAVACALL_FAIL;
}


/**
 * See pcsl_network.h for definition.
 *
 * Since the start function never returns PCSL_NET_WOULDBLOCK, this
 * function should never be called.
 */
javacall_result javacall_network_gethostbyname_finish(
                                                      javacall_ip_version ip_version,
                                                      javacall_configuration_id id,
                                                      unsigned char *pAddress,
                                                      int maxLen,
                                                      int *pLen,
                                                      void *handle,
                                                      void *context){
	
	return JAVACALL_FAIL;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_network_getsockopt(void *handle,
                                            javacall_socket_option flag,
                                            int *pOptval){

    return JAVACALL_FAIL;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_network_setsockopt(void *handle,
                                            javacall_socket_option flag,
                                            int optval){

    return JAVACALL_FAIL;
}

/**
 * Gets the string representation of the local device's IP address.
 * This function returns dotted quad IP address as a string in the 
 * output parameter and not the host name.
 *
 * @param pLocalIPAddress base of char array to receive the local
 *        device's IP address
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    if there is a network error
 */

javacall_result 
javacall_network_get_local_ip_address_as_string(javacall_ip_version ip_version,
                                                                    /*OUT*/ char *pLocalIPAddress){

    //javacall_printf("javacall_network_get_local_ip_address_as_string: trace\n");
	return JAVACALL_FAIL;
}

/**
 * See javacall_network.h for definition.
 */
javacall_result javacall_network_init_start(void) {

   return JAVACALL_OK;
}

/**
 * See javacall_network.h for definition.
 */
javacall_result javacall_network_init_finish() {
   	return JAVACALL_OK;
}

/**
 * Performs platform-specific finalization of the networking system.
 * Will be called ONCE during VM shutdown.
 *
 * @retval JAVACALL_WOULD_BLOCK caller must call xxx_finalize_finish
 *         function to complete the operation
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    fail
 */
javacall_result javacall_network_finalize_start(void)
{
   
	return JAVACALL_OK;
}

/**
 * Finishes platform-specific finalize of the networking system.
 * The function is invoked be the JVM after receiving JAVACALL_NETWORK_DOWN
 * notification from the platform.
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    fail
 */
javacall_result javacall_network_finalize_finish(void)
{
   return JAVACALL_OK;
}

/**
 * See pcsl_network.h for definition.
 */
int javacall_network_error(void *handle){

   //javacall_printf("javacall_network_error: trace\n");
   
   (void)handle;
    return 0;
}


/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_network_get_local_host_name(char *pLocalHost){
    //javacall_printf("javacall_network_get_local_host_name: trace\n");
    return JAVACALL_FAIL;
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result /*OPTIONAL*/ javacall_server_socket_open_start(
    javacall_configuration_id config_id,
    javacall_ip_version ip_version,
	int port, 
	void **pHandle, 
	void **pContext) {    

	//javacall_printf("javacall_server_socket_open_start: trace\n");
    return JAVACALL_FAIL; 
}

/**
 * See pcsl_network.h for definition.
 */
javacall_result javacall_server_socket_open_finish(void *handle, void *context){

	//javacall_printf("javacall_server_socket_open_finish: trace\n");
	return JAVACALL_OK;
}

/**
 * See javacall_socket.h for definition.
 */
javacall_result /*OPTIONAL*/ javacall_server_socket_accept_start(
      javacall_handle handle, 
      javacall_handle *pNewhandle) {
   
    //javacall_printf("javacall_server_socket_accept_start: trace\n");
	return JAVACALL_FAIL;
}

/**
 * See javacall_socket.h for definition.
 */
javacall_result /*OPTIONAL*/ javacall_server_socket_accept_finish(
	javacall_handle handle, 
	javacall_handle *pNewhandle) {

    //javacall_printf("javacall_server_socket_accept_finish: trace\n");
	return JAVACALL_FAIL;
}

/**
 * See javacall_network.h for definition.
 */
javacall_result javacall_server_socket_set_notifier(javacall_handle handle, javacall_bool set) {
	//javacall_printf("javacall_server_socket_set_notifier: trace\n");
	return JAVACALL_FAIL;
}

/**
 * Gets the http / https proxy address. This method is
 * called when the <tt>com.sun.midp.io.http.proxy</tt> or 
 <tt>com.sun.midp.io.https.proxy</tt> internal property
 * is retrieved.
 *
 * @param pHttpProxy base of char array to receive the hostname followed 
 *          by ':' and port. - ex) webcache.thecompany.com:8080.  
 *          Size of the pHttpProxy should be (MAX_HOST_LENGTH + 6).
 * @param pHttpsProxy base of char array to receive the hostname followed 
 *          by ':' and port. - ex) webcache.thecompany.com:8080.  
 *          Size of the pHttpsProxy should be (MAX_HOST_LENGTH + 6).
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    if there is a network error
 */
javacall_result /*OPTIONAL*/ javacall_network_get_http_proxy(/*OUT*/ char *pHttpProxy) {
	return JAVACALL_FAIL;
}

/**
 * Gets the port number of the local socket endpoint.
 *
 * @param handle handle of an open connection
 * @param pPortNumber returns the local port number
 * 
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    if there was an error
 */
javacall_result /*OPTIONAL*/ javacall_socket_getlocalport(
        javacall_handle handle,
        int *pPortNumber) {
        
    return JAVACALL_FAIL;
}

/**
 * Gets the port number of the remote socket endpoint.
 *
 * @param handle handle of an open connection
 * @param pPortNumber returns the local port number
 * 
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    if there was an error
 */
javacall_result javacall_socket_getremoteport(
    void *handle,
    int *pPortNumber)
{
    return JAVACALL_FAIL;
}

/**
 * Gets the IP address of the local socket endpoint.
 *
 * @param handle handle of an open connection
 * @param pAddress base of byte array to receive the address
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    if there was an error
 */
javacall_result /*OPTIONAL*/ javacall_socket_getlocaladdr(
    javacall_handle handle,
    char *pAddress)
{
	return JAVACALL_FAIL;
}

/**
 * Gets the IP address of the remote socket endpoint.
 *
 * @param handle handle of an open connection
 * @param pAddress base of byte array to receive the address
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    if there was an error
 */
javacall_result /*OPTIONAL*/ javacall_socket_getremoteaddr(
    void *handle,
    char *pAddress)
{
	return JAVACALL_FAIL;
}

char* javacall_inet_ntoa(javacall_ip_version ip_version, void *address) {
	return "";
}

void javacall_network_config() {
}
