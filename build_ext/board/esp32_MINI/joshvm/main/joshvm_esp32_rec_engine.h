#ifndef JOSHVM_ESP32_REC_ENGINE_H_
#define JOSHVM_ESP32_REC_ENGINE_H_



//---define



//---enum


//---struct



//---fun

int joshvm_esp32_wakeup_get_word_count(void);
int joshvm_esp32_wakeup_get_word(int pos, int* index, char* wordbuf, int wordlen, char* descbuf, int desclen);


/**
 * @brief enable wakeup
 *
 * @note 
 * @param 	callback: run when wakeup
 *			
 *
 * @return (error code)
 *     - 0: ok
 *     	 -1: fail
 *		...
 */
int joshvm_esp32_wakeup_enable(void(*callback)(int));

/**
 * @brief disable wakeup
 *
 * @note 
 * @param 	
 *			
 *
 * @return (error code)
 *     - 0: ok
 *     	 -1: fail
 *		...
 */
int joshvm_esp32_wakeup_disable();


int joshvm_esp32_vad_start(void(*callback)(int));
int joshvm_esp32_vad_pause();
int joshvm_esp32_vad_resume();
int joshvm_esp32_vad_stop();
int joshvm_esp32_vad_set_timeout(int ms);




//-------------test
//void test_rec_engine(void);
//void test_vad_callback(int index);



#endif


