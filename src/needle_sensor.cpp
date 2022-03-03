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
#include "needle_sensor.hpp"

/**
 * @brief Initializes needle sensor pin
 * 
 */
void needle_sensor_setup(void) {
    // Initialize pins as pulled up inputs
    pinMode(PIN_NEEDLE_SENSOR, INPUT_PULLUP);

    // Attach sensor interrupt on falling edge (HIGH -> LOW)
    attachInterrupt(digitalPinToInterrupt(PIN_NEEDLE_SENSOR), needle_sensor_callback, FALLING);
}

/**
 * @brief Sets needle_interrupt_flag flag
 * 
 */
void needle_sensor_callback(void) {
    needle_interrupt_flag = true;
#ifdef DEBUG
    serial->println("Needle interrupt");
#endif
}

/**
 * @brief Returns needle_interrupt_flag
 * 
 * @return boolean - true if interrupt occurred
 */
boolean needle_sensor_get_interrupt_flag() {
    return needle_interrupt_flag;
}

/**
 * @brief Clears needle interrupt flag
 * 
 */
void needle_sensor_clear_interrupt_flag(void) {
    needle_interrupt_flag = false;
}