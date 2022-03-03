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

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>


/*******************************/
/*            DEBUG            */
/*******************************/
#define DEBUG

#ifdef DEBUG
#define DEBUG_SERIAL Serial
#define DEBUG_SERIAL_SPEED 115200
#endif


/**********************************************/
/*            CNC Shield v1.0 pins            */
/**********************************************/
// Step pins
const uint8_t PIN_X_STP PROGMEM = 2;
const uint8_t PIN_Y_STP PROGMEM = 3;

// Dir pins
const uint8_t PIN_X_DIR PROGMEM = 5;
const uint8_t PIN_Y_DIR PROGMEM = 6;

// Enable pin
const uint8_t PIN_ENABLE PROGMEM = 8;


/*********************************/
/*            Encoder            */
/*********************************/
const uint8_t PIN_ENC_A PROGMEM = A8;
const uint8_t PIN_ENC_B PROGMEM = A10;
const uint8_t PIN_ENC_BTN PROGMEM = A11;
const uint8_t PIN_ENC_GND PROGMEM = A9;
#define ENCODER_PCINT_VECT PCINT2_vect
// 0 or 1
#define ENC_TYPE 0
#define MIN_BTN_PRESSED_TIME 100


/********************************************/
/*            Needle sensor pins            */
/********************************************/
const uint8_t PIN_NEEDLE_SENSOR PROGMEM = 19;


/***************************************/
/*            Tension servo            */
/***************************************/
const uint8_t PIN_SERVO PROGMEM = 11;
# define SERVO_LOW_TENSION_US 800
# define SERVO_HIGH_TENSION_US 2200


/******************************************/
/*            Speed controller            */
/******************************************/
const uint8_t SPEED_CONTROLLER_PIN PROGMEM = 46;


/*********************************/
/*            SD Card            */
/*********************************/
const uint8_t PIN_SD_CARD_CS = 53;
// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 0
#define FILE_EXT_UPPER ".GCODE"
#define FILE_EXT_LOWER ".gcode"
#define MAX_FILE_NAME_LENGTH 50
#define MAX_GCODE_LINE_LENGTH 50


/****************************************/
/*            Stepper motors            */
/****************************************/
#define FAS_TIMER_MODULE 3
const int32_t STEPS_PER_MM_X PROGMEM = 90;
const int32_t STEPS_PER_MM_Y PROGMEM = 65;
#define SPEED_INITIAL_MM_S 10
#define ACCELERATION_INITIAL_X_MM_S 300
#define ACCELERATION_INITIAL_Y_MM_S 500

#endif
