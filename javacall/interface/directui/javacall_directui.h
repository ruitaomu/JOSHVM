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

#ifndef __JAVACALL_DIRECTUI_H
#define __JAVACALL_DIRECTUI_H

#ifdef __cplusplus
extern "C"{
#endif

#include "javacall_defs.h"

typedef enum {
    JAVACALL_IMAGETYPE_PNG,
    JAVACALL_IMAGETYPE_JPG,
    JAVACALL_IMAGETYPE_BMP
} javacall_directui_image_type;

typedef enum {
    JAVACALL_DIRECTUI_COLOR_RGB565 = 1,
    JAVACALL_DIRECTUI_COLOR_RGB666 = 2,
    JAVACALL_DIRECTUI_COLOR_RGB888 = 3,
    JAVACALL_DIRECTUI_COLOR_ARGB8888 = 4,
    JAVACALL_DIRECTUI_COLOR_OTHER  = 199
} javacall_directui_color_encoding_type;

typedef enum {
    JAVACALL_FONT_DEFAULT = 0,
    JAVACALL_FONT_SMALL = 1,
    JAVACALL_FONT_MEDIUM = 2,
    JAVACALL_FONT_LARGE = 3
} javacall_font_type;

/**
 * The function javacall_directui_init is called during Java VM startup, allowing the
 * platform to perform device specific initializations.
 *
 * Once this API call is invoked, the VM will receive display focus.\n
 * <b>Note:</b> Consider the case where the platform tries to assume control
 * over the display while the VM is running by pausing the Java platform.
 * In this case, the platform is required to save the VRAM screen buffer:
 * Whenever Java is resumed, the stored screen buffers must be restored to
 * original state.
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    fail
 */
javacall_result javacall_directui_init(void);

/**
 * The function javacall_directui_finalize is called during Java VM shutdown,
 * allowing the platform to perform device specific lcd-related shutdown
 * operations.
 * The VM guarantees not to call other lcd functions before calling
 * javacall_directui_init( ) again.
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    fail
 */
javacall_result javacall_directui_finalize(void);

/**
 * This function is used to query the supported screen capabilities
 * - Display Width
 * - Display Height
 *
 * @param screenWidth output paramter to hold width of screen
 * @param screenHeight output paramter to hold height of screen
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    fail
 */
javacall_result javacall_directui_get_screen(int* screenWidth, int* screenHeight);

void javacall_directui_clear(int rgb);

/**
 * The following function is used to flush the image from the Video RAM raster
 * to the LCD display. \n
 * The function call should not be CPU time expensive, and should return
 * immediately. It should avoid memory bulk copying of the entire raster.
 *
 * @retval JAVACALL_OK   success
 * @retval JAVACALL_FAIL fail
 */
javacall_result javacall_directui_flush();

/**
 * Flush the screen raster to the display.
 * This function should not be CPU intensive and should not perform bulk memory
 * copy operations.
 * The following API uses partial flushing of the VRAM, thus may reduce the
 * runtime of the expensive flush operation.
 *
 * @param xstart start horizon scan line to start from
 * @param ystart start vertical scan line to start from
 * @param xend last horizon scan line to refresh
 * @param yend last vertical scan line to refresh
 *
 * @retval JAVACALL_OK      success
 * @retval JAVACALL_FAIL    fail
 */
javacall_result javacall_directui_flush_region(int xstart, int ystart, int xend, int yend);

javacall_result javacall_directui_setBacklight(int on);

/**
 * Draws the first textLen characters to display screen specified using the current font,
 * color.
 *
 * @param color color of font
 * @param x The x coordinate of the top left font coordinate
 * @param y The y coordinate of the top left font coordinate
 * @param text Pointer to the characters to be drawn
 * @param textLen The number of characters to be drawn
 * @return <tt>JAVACALL_OK</tt> if font rendered successfully,
 *         <tt>JAVACALL_FAIL</tt> or negative value on error or not supported
 */
javacall_result javacall_directui_textout(int font,
                        int color,
                        int                         x,
                        int                         y,
                        const javacall_utf16*     text,
                        int                         textLen,
                        int delayed);

javacall_result javacall_directui_text_getsize(int font, const javacall_utf16* text,
        int textLen, int* width, int* height);


javacall_result javacall_directui_image_getsize(javacall_uint8* imagedata,
        int datalen, javacall_directui_image_type type, int* width, int* height);

/**
 * Draws encoded image to display screen
 *
 * @param x The x coordinate of the top left font coordinate
 * @param y The y coordinate of the top left font coordinate
 * @param imagedata The encode image data to be displayed
 * @param datalen Length of the encode image data
 * @param type Type of imagedata
 * @return <tt>JAVACALL_OK</tt> if font rendered successfully,
 *         <tt>JAVACALL_FAIL</tt> or negative value on decode error or format not supported
 */
javacall_result javacall_directui_drawimage(int x, int y, javacall_uint8* imagedata,
        int datalen, javacall_directui_image_type type, int delayed);

javacall_result javacall_directui_drawrawdata(int x, int y, javacall_uint8* image_data,
        int w, int h, int delayed);


typedef enum {
    JAVACALL_KEYPRESSED  = 1,
    JAVACALL_KEYRELEASED = 2
} javacall_keypress_type;

typedef enum {
    JAVACALL_KEY_0          = '0',
    JAVACALL_KEY_1          = '1',
    JAVACALL_KEY_2          = '2',
    JAVACALL_KEY_3          = '3',
    JAVACALL_KEY_4          = '4',
    JAVACALL_KEY_5          = '5',
    JAVACALL_KEY_6          = '6',
    JAVACALL_KEY_7          = '7',
    JAVACALL_KEY_8          = '8',
    JAVACALL_KEY_9          = '9',
    JAVACALL_KEY_ASTERISK   = '*',
    JAVACALL_KEY_POUND      = '#',
    JAVACALL_KEY_DOT        = '.',
    JAVACALL_KEY_BACK       = 0x00A0,
    JAVACALL_KEY_OK         = 0x00A1,
    JAVACALL_KEY_POWER      = 0x00A2,
    JAVACALL_KEY_USER1      = 0x00C1,
    JAVACALL_KEY_USER2      = 0x00C2,
    JAVACALL_KEY_USER3      = 0x00C3,
    JAVACALL_KEY_USER4      = 0x00C4,
    JAVACALL_KEY_USER5      = 0x00C5,
    JAVACALL_KEY_USER6      = 0x00C6,
    JAVACALL_KEY_FUNC1      = 0x00F1,
    JAVACALL_KEY_FUNC2      = 0x00F2,
    JAVACALL_KEY_FUNC3      = 0x00F3,
    JAVACALL_KEY_FUNC4      = 0x00F4,
    JAVACALL_KEY_FUNC5      = 0x00F5,
    JAVACALL_KEY_FUNC6      = 0x00F6,
    JAVACALL_KEY_INVALID    = 0x00FF
} javacall_keypress_code;

javacall_result javacall_directui_key_event_init();
javacall_result javacall_directui_key_event_finalize();
javacall_result javacall_directui_key_event_get(javacall_keypress_code* key,
        javacall_keypress_type* type);

/**
 * The notification function to be called by platform for keypress
 * occurences.
 * The platfrom will invoke the call back in platform context for
 * each key press, key release occurence
 * @param key the key that was pressed
 * @param type <tt>JAVACALL_KEYPRESSED</tt> when key was pressed
 *             <tt>JAVACALL_KEYRELEASED</tt> when key was released
 */
void javanotify_key_event(javacall_keypress_code key, javacall_keypress_type type);

#ifdef __cplusplus
}
#endif

#endif
