/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef JOSHVM_AUDIO_WRAPPER_H
#define JOSHVM_AUDIO_WRAPPER_H
#include "joshvm_esp32_media.h"


/*
 * @brief DCS init function
 *
 */
joshvm_err_t joshvm_audio_wrapper_init(joshvm_media_t* handle);
void joshvm_audio_player_destroy();

/**
 * DESC:
 * Developer needs to implement this interface to play audio.
 *
 * PARAM:
 * @param[in] url: the url of the audio need to play
 *
 * @RETURN: .
 */
int joshvm_audio_play_handler(const char *url);

audio_err_t joshvm_audio_pause(void);
audio_err_t joshvm_audio_resume_handler(const char *url);
audio_err_t joshvm_audio_stop_handler(joshvm_media_t* handle);
int joshvm_audio_get_state();
audio_err_t joshvm_audio_time_get(int *time);

audio_err_t joshvm_volume_get_handler(int *volume);
audio_err_t joshvm_volume_set_handler(int volume);
void joshvm_volume_adjust_handler(int volume);


#if 0

/*
 * @brief DCS audio on_started callback function
 *
 * @param flag: duer_audio_play_type_t
 *
 * @return success:0
 *         fail: others
 */
int duer_dcs_audio_on_started_cb(duer_audio_play_type_t flag);

/*
 * @brief DCS audio on_finished callback function
 *
 * @param flag: duer_audio_play_type_t
 *
 * @return success:0
 *         fail: others
 */
int duer_dcs_audio_on_finished_cb(duer_audio_play_type_t flag);

/*
 * @brief Get DCS audio type
 *
 * @return duer_audio_type_t
 *
 */
duer_audio_type_t duer_dcs_get_player_type();

/*
 * @brief Set DCS audio type
 *
 * @param num: duer_audio_type_t
 *
 * @return success:0
 *         fail: others
 */
int duer_dcs_set_player_type(duer_audio_type_t num);

/*
 * @brief Send DCS_PAUSE_CMD to DCS
 *
 */
void duer_dcs_audio_active_paused();

/*
 * @brief Send DCS_PLAY_CMD to DCS
 *
 */
void duer_dcs_audio_active_play();

/*
 * @brief Send DCS_PREVIOUS_CMD to DCS
 *
 */
void duer_dcs_audio_active_previous();

/*
 * @brief Send DCS_NEXT_CMD to DCS
 *
 */
void duer_dcs_audio_active_next();

/*
 * @brief Audio wrapper pause
 *
 */
void duer_audio_wrapper_pause();

/*
 * @brief Get wrapper state
 *
 * @return : `esp_audio_status_t`
 */
int duer_audio_wrapper_get_state();
#endif

#endif //
