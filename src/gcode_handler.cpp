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
#include "gcode_handler.hpp"


void gcode_cycle(void) {
    // Skip cycle if next_line_condition is after move and motors are running
    //if (next_line_condition == CONDITION_AFTER_MOVE && !is_motors_decelerating_or_stopped())
    //    return;


    // next_line_condition is after motors stop moving
    if (next_line_condition == CONDITION_AFTER_MOVE) {
        if (is_motors_decelerating_or_stopped()) {
            switch (action_after_move)
            {
            case ACTION_ENABLE_MOTOR:
                // Clear interrupt flag
                needle_sensor_clear_interrupt_flag();

                // Enable and start main motor
                motors_enable_z();
                motors_start_z();

                // Disable motor after needle interrupt
                next_line_condition = CONDITION_AFTER_INTERRUPT;
                action_after_needle_interrupt = ACTION_DISABLE_MOTOR;
                return;

                break;
            
            default:
                break;
            }
        }

        // Motors are still moving - just skip this cycle
        else
            return;

    }

    // next_line_condition is after needle interrupt
    else if (next_line_condition == CONDITION_AFTER_INTERRUPT) {
        // If interrupt occurs
        if (needle_sensor_get_interrupt_flag()) {
            switch (action_after_needle_interrupt)
            {
            case ACTION_MOVE_TO:
                /*// Move to new position
                motors_move_to_position(&x_new, &y_new);

                // Enable motor after move to create stitch
                if (motor_enabled) {
                    needle_sensor_clear_interrupt_flag();
                    next_line_condition = CONDITION_AFTER_MOVE;
                    action_after_move = ACTION_ENABLE_MOTOR;
                    return;
                }*/
                break;

            case ACTION_DISABLE_MOTOR:
                motors_stop_z();
                break;
            
            default:
                break;
            }
        }

        // No needle interrupt - just skip this cycle
        else
            return;
    }

    // Reset needle interrupt flag
    needle_sensor_clear_interrupt_flag();

    // Reset next command condition
    next_line_condition = CONDITION_IMMEDIATELY;

    // Clear interrupt action
    action_after_needle_interrupt = ACTION_NONE;

    // Clear after move action
    action_after_move = ACTION_NONE;

    // Read line from file
    if (sd_card_read_next_line()) {
        ///////////////////////////////////
        //            G-codes            //
        ///////////////////////////////////
        command = gcode_parse_code('G', -1);

        switch (command)
        {
            case 0:
            case 1: {
                // G0, G1 - interpolate motion
                // G0 - regular move
                // G1 - move after needle interrupt

                x_new = gcode_parse_code('X', motors_get_x());
                y_new = gcode_parse_code('Y', motors_get_y());
                speed = gcode_parse_code('F', speed);

                // Calculate interpolation factors
                calculate_interpolation();

                // Update speed and accelertion
                motors_set_speed_x(speed * interpolation_x);
                motors_set_speed_y(speed * interpolation_y);
                motors_set_acceleration_x(acceleration_x * interpolation_x);
                motors_set_acceleration_y(acceleration_y * interpolation_y);

                // G1
                if (command == 1) {
                    motors_move_to_position(&x_new, &y_new);

                    // Enable motor after move to create stitch
                    if (is_motor_enabled) {
                        needle_sensor_clear_interrupt_flag();
                        next_line_condition = CONDITION_AFTER_MOVE;
                        action_after_move = ACTION_ENABLE_MOTOR;
                        return;
                    }
                    
                    /*action_after_needle_interrupt = ACTION_MOVE_TO;
                    next_line_condition = CONDITION_AFTER_INTERRUPT;*/
                }

                // G0
                else {
                    motors_move_to_position(&x_new, &y_new);
                    next_line_condition = CONDITION_AFTER_MOVE;
                }
                break;
            }

            case 4:
                // G4 - Delay (Dwell)
                delay(gcode_parse_code('P', 0));
                break;
            
            default:
                break;
        }

        ///////////////////////////////////
        //            M-codes            //
        ///////////////////////////////////
        command = gcode_parse_code('M', -1);
        switch (command)
        {
            case 0:
                // M0 - Pause
                // Parse paused code
                paused_code = gcode_parse_code('C', 0);

                // Pause motors
                gcode_pause();

                // Draw paused menu
                menu_pause_file();

                // Reset paused_code after pause
                paused_code = 0;
                break;

            case 3:
                // M3 - Enable tool
                motors_enable_z();
                is_motor_enabled = true;
                break;

            case 5:
                // M5 - Disable tool
                motors_disable_z();
                motors_stop_z();
                is_motor_enabled = false;
                break;

            case 17:
                // M17 - Enable steppers
                motors_enable();
                break;

            case 18:
                // M18 - Disable steppers
                motors_disable();
                break;

            case 41:
                // Remove thread tension
                servo_set_tension(0);
                is_tensioned = false;
                break;

            case 42:
                // Set high thread tension
                servo_set_tension(tension_);
                is_tensioned = true;
                break;
                
            case 73:
                // M73 - Set progress
                progress = gcode_parse_code('P', progress);
                if (progress > 100)
                    progress = 100;
                lcd_print_progress();
                break;

            case 201:
                // M201 - Set acceleration
                acceleration_x = gcode_parse_code('X', acceleration_x);
                acceleration_y = gcode_parse_code('Y', acceleration_y);

                motors_set_acceleration_x(acceleration_x);
                motors_set_acceleration_y(acceleration_y);
                break;
            
            default:
                break;
        }
    }

    // End of file
    else
        menu_stop_file();
}

/**
 * @brief Returns current thread tension value
 * 
 * @return uint8_t - tension 0 to 100
 */
uint8_t gcode_get_tension() {
    return tension_;
}

/**
 * @brief Sets maximum tension and writes it to the EEPROM
 * 
 * @param tension - maximum tension 0 to 100 
 */
void gcode_set_tension(uint8_t tension) {
    tension_ = tension;
    EEPROM.write(0, tension_);

    if (is_tensioned)
        servo_set_tension(tension_);
}

/**
 * @brief Returns current work progress
 * 
 * @return uint8_t - progress 0 to 100
 */
uint8_t gcode_get_progress() {
    return progress;
}

/**
 * @brief Returns paused code
 * 
 * @return uint8_t - current paused code (0 - no code)
 */
uint8_t gcode_get_paused_code() {
    return paused_code;
}

/**
 * @brief Clears and resets variables
 * 
 */
void gcode_clear(void) {
    // Read tension from EEPROM
    tension_ = EEPROM.read(0);
    if (tension_ > 100 || tension_ % 5 != 0) {
        tension_ = 90;
        EEPROM.write(0, tension_);
    }

    // Reset variables
    x_new = 0;
    y_new = 0;
    interpolation_x = 1;
    interpolation_y = 1;
    command = 0;
    progress = 0;
    paused_code = 0;
    is_motor_enabled = 0;
    is_tensioned = 0;

    // Reset line condition
    next_line_condition = CONDITION_IMMEDIATELY;

    // Reset action
    action_after_move = ACTION_NONE;
    action_after_needle_interrupt = ACTION_NONE;

    // Reset needle interrupt flag
    needle_sensor_clear_interrupt_flag();

    // Set initial speed and accelerations
    speed = SPEED_INITIAL_MM_S;
    acceleration_x = ACCELERATION_INITIAL_X_MM_S;
    acceleration_y = ACCELERATION_INITIAL_Y_MM_S;
}

void gcode_pause(void) {
    // Stop motors
    motors_stop();

    // Stop main motor
    if (is_motor_enabled) {
        motors_stop_z();
        motors_disable_z();
    }

    // Reset line condition
    next_line_condition = CONDITION_IMMEDIATELY;
}

void gcode_resume(void) {
    // Reset line condition
    next_line_condition = CONDITION_IMMEDIATELY;
}

void gcode_stop(void) {
    // Reset line condition
    next_line_condition = CONDITION_IMMEDIATELY;

    // Stop motors
    motors_abort_and_reset();

    // Set current file position to 0
    sd_card_file_rewind();

    // Remove thread tension
    servo_set_tension(0);
    is_tensioned = false;

    // Stop main motor
    if (is_motor_enabled) {
        motors_stop_z();
        motors_disable_z();
    }
}

void calculate_interpolation(void) {
    // Calculate angle of motion
    result_angle_rad = atan2f(y_new - motors_get_y(), x_new - motors_get_x());

    // Calculate X interpolation as cos
    interpolation_x = abs(cosf(result_angle_rad));

    // Calculate Y interpolation as sin
    interpolation_y = abs(sinf(result_angle_rad));
}

float gcode_parse_code(char code, float default_value) {
    // Start at the beginning of buffer
	char *ptr = sd_card_get_buffer();

    // Walk to the end
	while ((long)ptr > 1 && (*ptr) && (long)ptr < (long)sd_card_get_buffer() + strlen(sd_card_get_buffer())) {
        // Convert the digits that follow to a floating point number and return it if the code is found
		if (*ptr == code)
			return atof(ptr + 1);

        // Return default value if the comment char is found
        else if (*ptr == ';')
            return default_value;

        // Take a step from here to the letter after the next space
		ptr = strchr(ptr, ' ') + 1;
	}

    // End reached, nothing found, return default_value.
	return default_value;
}
