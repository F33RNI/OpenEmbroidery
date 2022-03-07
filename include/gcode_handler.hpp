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

#ifndef GCODE_HANDLER_H
#define GCODE_HANDLER_H

#include <EEPROM.h>

#define CONDITION_IMMEDIATELY 0
#define CONDITION_AFTER_MOVE 1
#define CONDITION_AFTER_INTERRUPT 2

#define ACTION_NONE 0
#define ACTION_MOVE_TO 1
#define ACTION_DISABLE_MOTOR 2
#define ACTION_ENABLE_MOTOR 3

float x_new, y_new;
float result_angle_rad, interpolation_x, interpolation_y;
uint32_t speed, acceleration_x, acceleration_y;
int command;

uint8_t next_line_condition;
uint8_t action_after_needle_interrupt, action_after_move;

uint8_t tension_;
uint8_t progress;
uint8_t paused_code;

boolean is_tensioned, is_motor_enabled;

void calculate_interpolation(void);
float gcode_parse_code(char code, float default_value);

#endif

