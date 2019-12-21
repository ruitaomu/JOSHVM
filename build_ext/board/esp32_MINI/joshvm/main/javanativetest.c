#include <stdio.h>

typedef enum SNIsignalType {
    NO_SIGNAL = 0,
    NETWORK_READ_SIGNAL,
    NETWORK_WRITE_SIGNAL,
    NETWORK_EXCEPTION_SIGNAL,
    COMM_OPEN_SIGNAL,
    COMM_READ_SIGNAL,
    COMM_WRITE_SIGNAL,
    COMM_CLOSE_SIGNAL,
    DEBUG_SIGNAL,
    VM_DEBUG_SIGNAL,
    HOST_NAME_LOOKUP_SIGNAL,
    SECURITY_CHECK_SIGNAL,
    WMA_SMS_READ_SIGNAL,
    WMA_SMS_WRITE_SIGNAL,
    PUSH_SIGNAL,
    NETWORK_UP_SIGNAL,
    NETWORK_DOWN_SIGNAL,
    GPIO_PIN_INPUT_SIGNAL,
    KEY_SIGNAL,
    BN_CALC_COMP_SIGNAL,
    ESP32_VOICE_SIGNAL
} SNIsignalType;

/**
 * Structure to hold contextual information across thread blocking, awakening, 
 * and native method reinvocation.
 * 
 * This structure is used at three different times:
 * 
 * (1) It is populated at the time the thread is blocked.
 *
 * (2) It is read at the time an event occurs and the notifying code wishes to 
 * awaken one or more threads. 
 *
 * (3) It is available to the native method when it is reinvoked.
 *
 * The waitingFor field defines the namespace within which descriptor values 
 * reside. When searching for threads to be signaled, both the waitingFor and 
 * descriptor fields must match the arguments to the signal call.
 *
 * The status field can be set at time (2) by the notifying code in order to 
 * pass status to the reinvoked native method at time (3).
 *
 * The pResult field can be set at time (1) for use at time (3). The notifying
 * code at time (2) should not set or get this field. The reason is that
 * pResult may require native memory allocation, and notification may occur on
 * zero or many threads, which makes dealing with native memory allocation 
 * difficult.
 */
typedef struct _SNIReentryData {
    SNIsignalType waitingFor;   /**< type of signal */
    int descriptor;              /**< platform specific handle */
    int status;                  /**< error code produced by the operation
                                      that unblocked the thread */
    void* pContext;               /**< platform specific context info used
                                       by thread wait functions etc. */
} SNIReentryData;

/**
 * @enum javacall_result
 * @brief javacall error results
 */
typedef enum {
   /** Generic success */
   JAVACALL_OK = 0,
   /** Generic failure */
   JAVACALL_FAIL = -1,
   /** Not implemented */
   JAVACALL_NOT_IMPLEMENTED = -2,
   /** Out of memory */
   JAVACALL_OUT_OF_MEMORY = -3,
   /** Invalid argument */
   JAVACALL_INVALID_ARGUMENT = -4,
   /** Would block */
   JAVACALL_WOULD_BLOCK = -5,
   /** Connection not found */
   JAVACALL_CONNECTION_NOT_FOUND = -6,
   /** Operation is interrupted */
   JAVACALL_INTERRUPTED = -7,
   /** Return by javacall read on  
       SoS connections or socket in
       Non-Delay mode. Caller should
       reinvoke the read function 
       to retry reading data */
   JAVACALL_NO_DATA_AVAILABLE = -8,
   /** File not found in the given path */
   JAVACALL_FILE_NOT_FOUND = -9,
   /** bad file name */
   JAVACALL_BAD_FILE_NAME = -10,
   /** End of file */
   JAVACALL_END_OF_FILE = -11,
   /** I/O error occured */
   JAVACALL_IO_ERROR = -12,
   /** bad properties in jad file, 
    * either a missing required property or
    * incorrectly formatted property */
   JAVACALL_BAD_JAD_PROPERTIES = -13,
	/** javacall properties db value not found */
   JAVACALL_VALUE_NOT_FOUND = -14,
	/** Invalid state */
   JAVACALL_INVALID_STATE = -15,
   /** Timeout elapsed */
   JAVACALL_TIMEOUT = -16
} javacall_result;

extern javacall_result javacall_event_receive(
                            long                     timeTowaitInMillisec,
                            /*OUT*/ unsigned char*  binaryBuffer,
                            /*IN*/  int             binaryBufferMaxLen,
                            /*OUT*/ int*            outEventLen);
extern javacall_result javacall_events_init(void);

#define RECORD_VOICE 0
#define VOICE_WAKEUP 1
#define PLAYBACK_VOICE 2

#define WAKEUP_STATUS_ERROR (-1)
#define WAKEUP_STATUS_BEGIN 0
#define WAKEUP_STATUS_END 1
#define WAKEUP_STATUS_VADSTART 2
#define WAKEUP_STATUS_VADSTOP 3

char* STATUS_STRING[] = {"WAKEUP_STATUS_BEGIN", "WAKEUP_STATUS_END", "WAKEUP_STATUS_VADSTART", "WAKEUP_STATUS_VADSTOP"};

extern int esp32_record_voicefile(unsigned char* filename, int time);
extern int esp32_playback_voice(int index);
extern void esp32_device_control(int);
extern int esp32_read_voice_buffer(unsigned char*, int);
extern int esp32_playback_voice_url(const char *url);
extern void esp32_stop_playback(void);
extern void esp32_stop_record(void);
extern void JavaNativeTest();

void JavaNativeTest() {
	SNIReentryData msg;
	int len;
	int recording;
	int rawdatalength;
	static unsigned char buf[2880];
						
	javacall_events_init();
	while (1) {
		printf("receiving events...\n");
		if (JAVACALL_OK == javacall_event_receive(-1, (unsigned char*)&msg, sizeof(SNIReentryData), &len)) {
			if (msg.waitingFor == ESP32_VOICE_SIGNAL){
				printf("Received ESP32_VOICE_SIGNAL: %d, %s\n", msg.descriptor, msg.status>=0 && msg.status<=3 ? STATUS_STRING[msg.status]:"UNKNOWN");
				switch (msg.descriptor) {
				case VOICE_WAKEUP:
					switch (msg.status) {
					case WAKEUP_STATUS_BEGIN:
						esp32_device_control(0); //pause
						esp32_device_control(1); //resume
					break;
					case WAKEUP_STATUS_END:
					break;
					case WAKEUP_STATUS_VADSTART:
						recording = 1;
						rawdatalength = 0;
						printf("Start recording VAD audio raw data...\n");
						while (recording) {
							printf("Fetching audio raw data...\n");
							int ret = esp32_read_voice_buffer((unsigned char*)&buf[0], 2880);
							printf("Fetched ret=%d\n", ret);
							if (ret > 0) {
								rawdatalength += ret;
							}
							if (JAVACALL_OK == javacall_event_receive(500, (unsigned char*)&msg, sizeof(SNIReentryData), &len)) {
								if ((msg.waitingFor == ESP32_VOICE_SIGNAL) && (msg.descriptor == VOICE_WAKEUP) && (msg.status == WAKEUP_STATUS_VADSTOP)) {
									recording = 0;
									printf("End of recording VAD audio raw data, length = %d\n", rawdatalength);
								}
							}
						}
					break;
					case WAKEUP_STATUS_VADSTOP:
					break;
					default:
					break;
					}
				break;
				case PLAYBACK_VOICE:
				break;
				case RECORD_VOICE:
				break;
				default: //Unknown
				break;
				}
			}
		}
	}
}
