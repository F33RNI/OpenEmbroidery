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

#ifndef ENCODER_H
#define ENCODER_H

volatile uint8_t *enc_a_in_reg;
volatile uint8_t *enc_b_in_reg;
volatile uint8_t *enc_btn_in_reg;
uint8_t enc_a_mask, enc_b_mask, enc_btn_mask;
boolean enc_a_state, enc_a_state_last;
boolean enc_b_state;
boolean enc_btn_state, enc_btn_state_last;
boolean enc_turn_flag;

uint64_t button_timer;
boolean button_pressed_flag;

int32_t encoder_counter;

#endif
