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
#include "encoder.hpp"

/**
 * @brief Initializes encoder pins and interrupts
 */
void encoder_setup(void) {
#ifdef DEBUG
    serial->println("Initializing encoder");
#endif
    // Initialize encoder pins as pull up inputs
    pinMode(PIN_ENC_A, INPUT_PULLUP);
    pinMode(PIN_ENC_B, INPUT_PULLUP);
    pinMode(PIN_ENC_BTN, INPUT_PULLUP);

    // Initialize encoder ground
    pinMode(PIN_ENC_GND, OUTPUT);
    digitalWrite(PIN_ENC_GND, 0);

    // Convert arduino pins to input register
    enc_a_in_reg = portInputRegister(digitalPinToPort(PIN_ENC_A));
    enc_b_in_reg = portInputRegister(digitalPinToPort(PIN_ENC_B));
    enc_btn_in_reg = portInputRegister(digitalPinToPort(PIN_ENC_BTN));

    // Convert arduino pins to masks
    enc_a_mask = digitalPinToBitMask(PIN_ENC_A);
    enc_b_mask = digitalPinToBitMask(PIN_ENC_B);
    enc_btn_mask = digitalPinToBitMask(PIN_ENC_BTN);

    // Enable PCINT
    *digitalPinToPCMSK(PIN_ENC_A) |= bit (digitalPinToPCMSKbit(PIN_ENC_A));
    *digitalPinToPCMSK(PIN_ENC_B) |= bit (digitalPinToPCMSKbit(PIN_ENC_B));
    *digitalPinToPCMSK(PIN_ENC_BTN) |= bit (digitalPinToPCMSKbit(PIN_ENC_BTN));

    // Clear any outstanding interrupt and enable interrupt for the groups
    PCIFR  |= bit (digitalPinToPCICRbit(PIN_ENC_A));
    PCICR  |= bit (digitalPinToPCICRbit(PIN_ENC_A));
    PCIFR  |= bit (digitalPinToPCICRbit(PIN_ENC_B));
    PCICR  |= bit (digitalPinToPCICRbit(PIN_ENC_B));
    PCIFR  |= bit (digitalPinToPCICRbit(PIN_ENC_BTN));
    PCICR  |= bit (digitalPinToPCICRbit(PIN_ENC_BTN));
}

/**
 * @brief Construct a new ISR object
 * 
 */
ISR(ENCODER_PCINT_VECT) {
    // Read current states
    enc_a_state = !(*enc_a_in_reg & enc_a_mask);
    enc_b_state = !(*enc_b_in_reg & enc_b_mask);
    enc_btn_state = !(*enc_btn_in_reg & enc_btn_mask);

    // If encoder state changed
    if (enc_a_state != enc_a_state_last) {
        enc_turn_flag = !enc_turn_flag;

        // Increment or decrement encoder_counter
#if (ENC_TYPE == 1)
        if (enc_turn_flag)
            encoder_counter += (enc_b_state != enc_a_state_last) ? -1 : 1;
#else
        encoder_counter += (enc_b_state != enc_a_state_last) ? -1 : 1;
#endif

        // Store A pin state for next cycle
        enc_a_state_last = enc_a_state;

    }

    // Button pressed
    if (enc_btn_state && !enc_btn_state_last) {
        // Start timer
        button_timer = millis();

        // Store button state for next cycle
        enc_btn_state_last = enc_btn_state;
    }

    // Button released
    else if (!enc_btn_state && enc_btn_state_last) {
        // Set button flag
        if (millis() - button_timer >= MIN_BTN_PRESSED_TIME)
            button_pressed_flag = true;

        // Store button state for next cycle
        enc_btn_state_last = enc_btn_state;
    }
}

/**
 * @brief Returns encoder counter
 * 
 * @return int32_t - encoder counter
 */
int32_t encoder_get_counter() {
    return encoder_counter;
}

/**
 * @brief Returns button_pressed_flag
 * 
 * @return boolean - true if button was pressed
 */
boolean encoder_get_button_flag() {
    return button_pressed_flag;
}

/**
 * @brief Clears encoder_counter
 * 
 */
void encoder_reset_counter(void) {
    encoder_counter = 0;
}

/**
 * @brief Clears button_pressed_flag
 * 
 */
void encoder_clear_button_flag(void) {
    button_pressed_flag = false;
}
