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
#include "speed_controller.hpp"

/**
 * @brief Initializes speed controller pin as PWM output
 * 
 */
void speed_controller_setup(void) {
    pinMode(SPEED_CONTROLLER_PIN, OUTPUT);
    //TCCR5B = TCCR5B & B11111000 | B00000100;
    //TCCR5B = TCCR5B & B11111000 | B00000101;
    analogWrite(SPEED_CONTROLLER_PIN, 0);
}

/**
 * @brief Writes speed to the speed controller
 * 
 * @param speed - 0 to 100
 */
void speed_controller_write_speed(uint8_t speed) {
    digitalWrite(SPEED_CONTROLLER_PIN, speed > 0);
    //analogWrite(SPEED_CONTROLLER_PIN, map(speed, 0, 100, 0, 255));
}