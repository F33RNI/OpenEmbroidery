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
#include "servo.hpp"

/**
 * @brief Initializes ServoTimer2 class object and sets initial tension (0)
 * 
 */
void servo_setup(void) {
    // Initialize servo
    pinMode(PIN_SERVO, OUTPUT);
    servo.attach(PIN_SERVO);

    // Write initial tension
    servo_set_tension(0);
}

/**
 * @brief Sets tension of the thread
 * 
 * @param tension - 0 to 100
 */
void servo_set_tension(uint8_t tension) {
    // Convert tension to servo pulse
    servo_pulse = map(tension, 0, 100, SERVO_LOW_TENSION_US, SERVO_HIGH_TENSION_US);

    // Write pulse to the servo
    servo.write(servo_pulse);
}
