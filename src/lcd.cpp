/*
 * Copyright (C) 2022 Fern H., OpenEmbroidery project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "config.hpp"
#include "datatypes.hpp"
#include "lcd.hpp"

/**
 * @brief Initializes LCD and print startup message
 * 
 */
void lcd_setup(void) {
#ifdef DEBUG
    serial->println("Initializing LCD");
#endif
    // Initialize LCD
    lcd.begin();
    lcd.noBacklight();

    // Print startup message
    lcd.print(F("--------------------"));
    lcd.setCursor(0, 1);
    lcd.print(F("OpenEmbroidery  v"));
    lcd.print(SOFTWARE_VERSION);
    lcd.setCursor(0, 2);
    lcd.print(F("Booting up..."));
    lcd.setCursor(0, 3);
    lcd.print(F("--------------------"));
    lcd.backlight();
}

/**
 * @brief Prints error message
 * 
 * @param message error message (up to 20 symbols)
 */
void lcd_print_error(const char *message) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Error:"));
    lcd.setCursor(0, 2);
    lcd.print(message);
}

/**
 * @brief Prints error message
 * 
 * @param message error message (up to 20 symbols)
 */
void lcd_print_error(const __FlashStringHelper *message) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Error:"));
    lcd.setCursor(0, 2);
    lcd.print(message);
}

void lcd_append_file_top(void) {
    // Move lines down
    memcpy(selector_lines[3], selector_lines[2], sizeof(selector_lines[3]) - 1);
    memcpy(selector_lines[2], selector_lines[1], sizeof(selector_lines[2]) - 1);
    memcpy(selector_lines[1], selector_lines[0], sizeof(selector_lines[1]) - 1);

    // Copy current file name
    memcpy(selector_lines[0], sd_card_get_file_name(), sizeof(selector_lines[0]) - 1);

    // Print lines on lcd
    lcd_print_selector();
}

void lcd_append_file_bottom(void) {
    // Move lines up
    memcpy(selector_lines[0], selector_lines[1], sizeof(selector_lines[0]) - 1);
    memcpy(selector_lines[1], selector_lines[2], sizeof(selector_lines[1]) - 1);
    memcpy(selector_lines[2], selector_lines[3], sizeof(selector_lines[2]) - 1);

    // Copy current file name
    memcpy(selector_lines[3], sd_card_get_file_name(), sizeof(selector_lines[3]) - 1);

    // Print lines on lcd
    lcd_print_selector();
}





void lcd_print_selector(void) {
    // Clear screen
    lcd.clear();

    // Print selector line 1
    lcd.setCursor(1, 0);
    lcd.print(selector_lines[0]);

    // Print selector line 2
    lcd.setCursor(1, 1);
    lcd.print(selector_lines[1]);

    // Print selector line 3
    lcd.setCursor(1, 2);
    lcd.print(selector_lines[2]);

    // Print selector line 4
    lcd.setCursor(1, 3);
    lcd.print(selector_lines[3]);
}

void lcd_print_cursor(uint8_t selector_cursor) {
    for (uint8_t i = 0; i < 4; i++) {
        lcd.setCursor(0, i);
        lcd.write(0x20);
    }

    if (selector_cursor < 4) {
        lcd.setCursor(0, selector_cursor);
        lcd.write(0x7E);
    }
}

void lcd_print_bottom_right_cursor(boolean is_cursor) {
    lcd.setCursor(19, 3);
    lcd.write(is_cursor ? 0x7F : 0x20);
}

/**
 * @brief Prints selected file name on top of the screen
 * 
 */
void lcd_print_file_name(void) {
    // Clear first row
    lcd.setCursor(0, 0);
    lcd.print(F("                    "));

    // Print file name
    lcd.setCursor(1, 0);
    for (uint8_t i = 0; i < 19; i++) {
        if (sd_card_get_file_name()[i] < 20)
            break;
        lcd.print(sd_card_get_file_name()[i]);
    }
}

void lcd_print_pre_start(void) {
    lcd.clear();
    lcd_print_file_name();

    lcd.setCursor(1, 2);
    lcd.print(F("Start"));

    lcd.setCursor(1, 3);
    lcd.print(F("Back"));

}

void lcd_print_work(void) {
    lcd.clear();
    lcd_print_file_name();

    lcd.setCursor(1, 1);
    lcd.print(F("-------"));
    lcd.setCursor(14, 1);
    lcd.print(F("-----"));
    lcd_print_progress();

    lcd.setCursor(1, 2);
    lcd.print(F("Pause"));

    lcd.setCursor(1, 3);
    lcd.print(F("Tension: "));
    lcd_print_tension();
}

void lcd_print_progress(void) {
    lcd.setCursor(9, 1);
    lcd.print(gcode_get_progress());
    lcd.print(F("% ---"));
}

void lcd_print_tension(void) {
    lcd.setCursor(10, 3);
    lcd.print(gcode_get_tension());
    lcd.print(F("%   "));
}

void lcd_print_pause(void) {
    lcd.clear();
    lcd_print_file_name();

    if (gcode_get_paused_code() > 0) {
        lcd.setCursor(1, 1);
        lcd.print(F("- PAUSED -"));
        lcd.setCursor(12, 1);
        lcd.print(F("Code: "));

        if (gcode_get_paused_code() < 100)
            lcd.print(gcode_get_paused_code());
        else if (gcode_get_paused_code() < 126)
            lcd.print("ABCDEFGHIJKLMNOPQRSTUVWXYZ"[gcode_get_paused_code() - (uint8_t)100]);
    }

    else {
        lcd.setCursor(1, 1);
        lcd.print(F("----- PAUSED -----"));
    }

    lcd.setCursor(1, 2);
    lcd.print(F("Resume"));

    lcd.setCursor(1, 3);
    lcd.print(F("Stop"));
}

void lcd_print_stop(void) {
    lcd.clear();
    lcd_print_file_name();

    lcd.setCursor(1, 1);
    lcd.print(F("- Are you sure? -"));

    lcd.setCursor(1, 2);
    lcd.print(F("Stop"));

    lcd.setCursor(1, 3);
    lcd.print(F("Back"));
}