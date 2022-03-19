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
#include "menu.hpp"


/**
 * @brief Prints first files to the lcd (initializes menu)
 * 
 */
void menu_sd_card_init(void) {
    // Add first files
    while(sd_card_read_prev_file())
        lcd_append_file_top();

    // Select first file
    lcd_print_cursor(0);

    // Reset files counter
    sd_card_reset_files();

    // Select first file
    sd_card_read_next_file();
}


/**
 * @brief Provides sd card file selection
 * 
 */
void menu_sd_card(void) {
    // Get current encoder state
    menu_encoder_counter_temp = encoder_get_counter();

    // If value changed
    if (menu_encoder_counter_temp != menu_encoder_counter) {
        // Scroll to the bottom (next file)
        if (menu_encoder_counter_temp > menu_encoder_counter) {

            if (file_index < sd_card_get_number_of_files() - 1) {
                // Read next file
                sd_card_read_next_file();
                file_index++;

                // Cursor on the bottom -> append file name to the bottom
                if (selector_cursor == 3)
                    lcd_append_file_bottom();

                // Cursor in normal position
                else
                    selector_cursor++;
            }

        }

        // Scroll to the top (previous file)
        else if (file_index > 0) {
            // Read previous file
            sd_card_read_prev_file();
            file_index--;

            // Cursor on the top -> append file name to the top
            if (selector_cursor == 0)
                lcd_append_file_top();

            // Cursor in normal position
            else
                selector_cursor--;
        }

        // Mark selected file with cursor
        lcd_print_cursor(selector_cursor);

        // Store for next cycle
        menu_encoder_counter = menu_encoder_counter_temp;

#ifdef DEBUG
        serial->print(F("File: "));
        serial->println(sd_card_get_file_name());
#endif
    }

    // Button pressed (file selected)
    if (encoder_get_button_flag()) {
        // Change system state to pre-run menu
        system_state = STATE_PRE_START;

        // Draw pre-start menu
        lcd_print_pre_start();
        sub_menu_cursor = 2;
        lcd_print_cursor(sub_menu_cursor);

        // Clear button flag
        encoder_clear_button_flag();
    }
}

void menu_pre_start(void) {
    // Get current encoder state
    menu_encoder_counter_temp = encoder_get_counter();

    // If value changed
    if (menu_encoder_counter_temp != menu_encoder_counter) {
        // Cycle only between 2 values (start and back)
        if (menu_encoder_counter_temp > menu_encoder_counter)
            sub_menu_cursor = 3;
        else
            sub_menu_cursor = 2;

        // Mark selected option with cursor
        lcd_print_cursor(sub_menu_cursor);

        // Store for next cycle
        menu_encoder_counter = menu_encoder_counter_temp;
    }

    // Button pressed
    if (encoder_get_button_flag()) {
        // Start selected file
        if (sub_menu_cursor == 2) {
            // Reset gcode variables
            gcode_clear();

            // Draw work menu
            lcd_print_work();
            sub_menu_cursor = 2;
            lcd_print_cursor(sub_menu_cursor);

            // Set system state to working
            system_state = STATE_WORK;
        }

        // Go back
        else {
            // Return to main menu
            system_state = STATE_SD_MENU;
            lcd_print_selector();
            lcd_print_cursor(selector_cursor);
        }

        // Clear button flag
        encoder_clear_button_flag();
    }
}

void menu_work(void) {
    // Get current encoder state
    menu_encoder_counter_temp = encoder_get_counter();

    // If value changed
    if (menu_encoder_counter_temp != menu_encoder_counter) {
        // Cycle only between 2 values (pause and change tension)
        if (menu_encoder_counter_temp > menu_encoder_counter)
            sub_menu_cursor = 3;
        else
            sub_menu_cursor = 2;

        // Mark selected option with cursor
        lcd_print_cursor(sub_menu_cursor);

        // Store for next cycle
        menu_encoder_counter = menu_encoder_counter_temp;
    }

    // Button pressed
    if (encoder_get_button_flag()) {
        // Pause
        if (sub_menu_cursor == 2)
            menu_pause_file();

        // Change tension
        else {
            // Change system_state to tension setup
            system_state = STATE_TENSION_SETUP;

            // Clear left cursor
            lcd_print_cursor(5);

            // Print bottom right cursor
            lcd_print_bottom_right_cursor(true);
        }

        // Clear button flag
        encoder_clear_button_flag();
    }
}

void menu_tension(void) {
    // Get current encoder state
    menu_encoder_counter_temp = encoder_get_counter();

    // If value changed
    if (menu_encoder_counter_temp != menu_encoder_counter) {
        // Increment tension
        if (menu_encoder_counter_temp > menu_encoder_counter) {
            if (gcode_get_tension() <= 95)
                gcode_set_tension(gcode_get_tension() + 5);
        }

        // Decrement tension
        else {
            if (gcode_get_tension() >= 10)
                gcode_set_tension(gcode_get_tension() - 5);
        }

        // Print tension
        lcd_print_tension();

        // Store for next cycle
        menu_encoder_counter = menu_encoder_counter_temp;
    }

    // Button pressed (return to normal printing)
    if (encoder_get_button_flag()) {
        // Return to default work menu
        system_state = STATE_WORK;
        lcd_print_cursor(sub_menu_cursor);
        lcd_print_bottom_right_cursor(false);

        // Clear button flag
        encoder_clear_button_flag();
    }
}

void menu_pause(void) {
    // Get current encoder state
    menu_encoder_counter_temp = encoder_get_counter();

    // If value changed
    if (menu_encoder_counter_temp != menu_encoder_counter) {
        // Cycle only between 2 values (resume and stop)
        if (menu_encoder_counter_temp > menu_encoder_counter)
            sub_menu_cursor = 3;
        else
            sub_menu_cursor = 2;

        // Mark selected option with cursor
        lcd_print_cursor(sub_menu_cursor);

        // Store for next cycle
        menu_encoder_counter = menu_encoder_counter_temp;
    }

    // Button pressed
    if (encoder_get_button_flag()) {
        // Resume
        if (sub_menu_cursor == 2) {
            // Draw work menu
            lcd_print_work();
            sub_menu_cursor = 2;
            lcd_print_cursor(sub_menu_cursor);

            // Set system state to working
            system_state = STATE_WORK;

            // Resume work
            gcode_resume();
        }

        // Stop
        else {
            // Draw stop menu
            lcd_print_stop();
            sub_menu_cursor = 3;
            lcd_print_cursor(sub_menu_cursor);

            // Set system state to stop confirmation
            system_state = STATE_STOP_CONFIRMATION;
        }

        // Clear button flag
        encoder_clear_button_flag();
    }
}

void menu_stop_confirmation(void) {
    // Get current encoder state
    menu_encoder_counter_temp = encoder_get_counter();

    // If value changed
    if (menu_encoder_counter_temp != menu_encoder_counter) {
        // Cycle only between 2 values (stop and back)
        if (menu_encoder_counter_temp > menu_encoder_counter)
            sub_menu_cursor = 3;
        else
            sub_menu_cursor = 2;

        // Mark selected option with cursor
        lcd_print_cursor(sub_menu_cursor);

        // Store for next cycle
        menu_encoder_counter = menu_encoder_counter_temp;
    }

    // Button pressed
    if (encoder_get_button_flag()) {
        // Stop
        if (sub_menu_cursor == 2)
            menu_stop_file();

        // Back
        else {
            // Print pause menu
            sub_menu_cursor = 2;
            lcd_print_pause();
            lcd_print_cursor(sub_menu_cursor);
            
            // Change system_state to pause
            system_state = STATE_PAUSE;
        }

        // Clear button flag
        encoder_clear_button_flag();
    }
}

void menu_stop_file(void) {
    // Stop current work
    gcode_stop();

    // Clear variables
    gcode_clear();

    // Return to main menu
    system_state = STATE_SD_MENU;
    lcd_print_selector();
    lcd_print_cursor(selector_cursor);
}

void menu_pause_file(void) {
    // Change system_state to pause
    system_state = STATE_PAUSE;

    // Pause current work
    gcode_pause();

    // Print pause menu
    sub_menu_cursor = 2;
    lcd_print_pause();
    lcd_print_cursor(sub_menu_cursor);
}
