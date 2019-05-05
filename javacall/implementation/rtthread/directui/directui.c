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

#include "javacall_directui.h"

#ifdef __cplusplus
extern "C"{
#endif

javacall_result javacall_directui_init(void) {
    return JAVACALL_OK;
}

javacall_result javacall_directui_finalize(void) {
    return JAVACALL_OK;
}

javacall_result javacall_directui_get_screen(int* screen_width, int* screen_height) {
    *screen_width = 0;
    *screen_height = 0;
    return JAVACALL_FAIL;
}

void javacall_directui_clear(int rgb) {
}

javacall_result javacall_directui_flush() {
    return JAVACALL_OK;
}

javacall_result javacall_directui_flush_region(int xstart, int ystart, int xend, int yend) {
	return JAVACALL_OK;
}

javacall_result javacall_directui_textout(int font, int color, int x, int y,
        const javacall_utf16* text, int textLen, int delayed) {
    return JAVACALL_OK;
}

javacall_result javacall_directui_text_getsize(int font, const javacall_utf16* text,
        int textLen, int* width, int* height) {
    return JAVACALL_FAIL;
}

javacall_result javacall_directui_image_getsize(javacall_uint8* image_data,
        int data_len, javacall_directui_image_type type, int* width, int* height) {
    return JAVACALL_FAIL;
}

javacall_result javacall_directui_drawimage(int x, int y, javacall_uint8* image_data,
        int data_len, javacall_directui_image_type type, int delayed) {
    return JAVACALL_OK;
}

javacall_result javacall_directui_drawrawdata(int x, int y, javacall_uint8* image_data,
        int w, int h, int delayed) {

    return JAVACALL_OK;
}

javacall_result javacall_directui_key_event_init() {
    return JAVACALL_OK;
}

javacall_result javacall_directui_key_event_finalize() {
    return JAVACALL_OK;
}

javacall_result javacall_directui_key_event_get(javacall_keypress_code* key, javacall_keypress_type* type) {
	return JAVACALL_OK;
}

#ifdef __cplusplus
}
#endif
