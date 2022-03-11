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

#ifndef DATATYPES_H
#define DATATYPES_H

#include <Arduino.h>
#include <time.h>

#define SOFTWARE_VERSION "1.0"

// System status (0 - sd card menu, 1 - prerun menu, 2 - working, 3 - tension setup, 4 - pause menu)
extern volatile uint8_t system_state;

// Debug serial
#ifdef DEBUG
extern HardwareSerial* serial;
#endif

// Encoder
void encoder_setup(void);
int32_t encoder_get_counter();
boolean encoder_get_button_flag();
void encoder_reset_counter(void);
void encoder_clear_button_flag(void);

// Gcode-handler
void gcode_cycle(void);
uint8_t gcode_get_tension();
void gcode_set_tension(uint8_t tension);
uint8_t gcode_get_progress();
uint8_t gcode_get_paused_code();
void gcode_clear(void);
void gcode_pause(void);
void gcode_resume(void);
void gcode_stop(void);

// LCD
void lcd_setup(void);
void lcd_print_error(const char *message);
void lcd_print_error(const __FlashStringHelper *message);
void lcd_append_file_top(void);
void lcd_append_file_bottom(void);
void lcd_print_selector(void);
void lcd_print_cursor(uint8_t selector_cursor);
void lcd_print_bottom_right_cursor(boolean is_cursor);
void lcd_print_file_name(void);
void lcd_print_pre_start(void);
void lcd_print_work(void);
void lcd_print_progress(void);
void lcd_print_tension(void);
void lcd_print_pause(void);

// Menu
void menu_sd_card_init(void);
void menu_sd_card(void);
void menu_pre_start(void);
void menu_work(void);
void menu_tension(void);
void menu_pause(void);
void menu_stop_file(void);
void menu_pause_file(void);

// Motors
boolean motors_setup();
float motors_get_x();
float motors_get_y();
void motors_set_speed_x(float speed_mm_s);
void motors_set_speed_y(float speed_mm_s);
void motors_set_speed_z(uint32_t speed_hz);
void motors_set_acceleration_x(float acceleration_mm_s);
void motors_set_acceleration_y(float acceleration_mm_s);
void motors_set_acceleration_z(int32_t acceleration_steps_s);
void motors_move_to_position(float *x, float *y);
void motors_enable(void);
void motors_disable(void);
boolean is_motors_stopped();
void motors_stop(void);
void motors_abort_and_reset(void);
void motors_enable_z(void);
void motors_disable_z(void);
void motors_start_z(void);
void motors_stop_z(void);
boolean is_motor_z_stopped();

// Needle sensor
void needle_sensor_setup(void);
boolean needle_sensor_get_interrupt_flag();
void needle_sensor_clear_interrupt_flag(void);

// SD card
boolean sd_card_setup();
void sd_card_count_files(void);
uint32_t sd_card_get_number_of_files();
boolean sd_card_read_next_file();
boolean sd_card_read_prev_file();
char *sd_card_get_file_name();
void sd_card_reset_files(void);
void sd_card_file_rewind(void);
boolean sd_card_check_selected_file();
boolean sd_card_read_next_line();
char *sd_card_get_buffer();

// Servo
void servo_setup(void);
void servo_set_tension(uint8_t tension);

// Time converter
time_t date_time_to_epoch(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day, uint8_t month, uint16_t year);

#endif
