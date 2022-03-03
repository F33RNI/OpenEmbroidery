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

volatile uint8_t system_state;

#ifdef DEBUG
HardwareSerial* serial;
#endif

void setup() {
  // Initialize debug serial port
#ifdef DEBUG
  Serial.begin(DEBUG_SERIAL_SPEED);
  serial = &DEBUG_SERIAL;
#endif

  // Initialize LCD and print startup message
  lcd_setup();

  // Initialize rotary encoder
  encoder_setup();

  // Initialize stepper motors
  if (!motors_setup()) {
    // Print error if motors setup error
    lcd_print_error(F("Motors init. failed"));

    // Enter infinite loop
    while (1);
  }

  // Initialize needle sensor
  needle_sensor_setup();

  // Initialize servo
  servo_setup();

  // Initialize speed controller
  speed_controller_setup();

  // Initialize gcode handler
  gcode_clear();

  // Initialize SD card
  if (!sd_card_setup()) {
    // Print error if SD card is not detected or the card cannot be opened
    lcd_print_error(F("SD card init. failed"));

    // Enter infinite loop
    while (1);
  }

  // Count number of files on SD card
  sd_card_count_files();

  // Wait some time to finish initialization (also, delay for startup message)
  delay(500);

  // Reset encoder
  encoder_reset_counter();
  encoder_clear_button_flag();

  // Show SD-card menu
  menu_sd_card_init();
}

void loop() {
  switch (system_state)
  {
  case 1:
    // Pre-start menu
    menu_pre_start();
    break;

  case 2:
  case 3:
    // Working
    gcode_cycle();
    if (system_state == 3)
      menu_tension();
    else
      menu_work();
    break;

  case 4:
    // Pause
    menu_pause();
    break;
  
  default:
    // SD-card menu
    menu_sd_card();
    break;
  }
}
