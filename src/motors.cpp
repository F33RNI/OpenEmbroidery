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
#include "motors.hpp"

/**
 * @brief Initializes stepper motors with FastAccelStepper library
 * 
 * @return boolean - true is successful
 */
boolean motors_setup() {
    // Add motors to the FastAccelStepper
    engine.init();
    stepper_x = engine.stepperConnectToPin(PIN_X_STP);
    stepper_y = engine.stepperConnectToPin(PIN_Y_STP);
    stepper_z = engine.stepperConnectToPin(PIN_Z_STP);

    // Return false if initialization fails
    if (!stepper_x || !stepper_y || !stepper_z)
        return false;

    // Set enable and direction pins
    stepper_x->setDirectionPin(PIN_X_DIR);
    stepper_x->setEnablePin(PIN_ENABLE);
    stepper_y->setDirectionPin(PIN_Y_DIR);
    stepper_y->setEnablePin(PIN_ENABLE);
    stepper_z->setEnablePin(PIN_ENABLE_Z);

    // Disable motors
    motors_disable();
    stepper_z->disableOutputs();

    // Set default speed and acceleration
    stepper_x->setSpeedInHz(SPEED_INITIAL_XY_MM_S * STEPS_PER_MM_X);
    stepper_y->setSpeedInHz(SPEED_INITIAL_XY_MM_S * STEPS_PER_MM_Y);
    stepper_z->setSpeedInHz(SPEED_INITIAL_Z_HZ);
    stepper_x->setAcceleration(ACCELERATION_INITIAL_X_MM_S * STEPS_PER_MM_X);
    stepper_y->setAcceleration(ACCELERATION_INITIAL_Y_MM_S * STEPS_PER_MM_Y);
    stepper_z->setAcceleration(ACCELERATION_INITIAL_Z_HZ);
    
    // Return successful value
    return true;
}

/**
 * @brief Gets current x motor position
 * 
 * @return float - motor position in mm
 */
float motors_get_x() {
    return (float)stepper_x->getCurrentPosition() / (float)STEPS_PER_MM_X;
}

/**
 * @brief Gets current y motor position
 * 
 * @return float - motor position in mm
 */
float motors_get_y() {
    return (float)stepper_y->getCurrentPosition() / (float)STEPS_PER_MM_Y;
}

/**
 * @brief Sets X motor speed
 * 
 * @param speed_mm_s - speed in mm/s
 */
void motors_set_speed_x(float speed_mm_s) {
    stepper_x->setSpeedInHz(speed_mm_s * STEPS_PER_MM_X);
}

/**
 * @brief Sets Y motor speed
 * 
 * @param speed_mm_s - speed in mm/s
 */
void motors_set_speed_y(float speed_mm_s) {
    stepper_y->setSpeedInHz(speed_mm_s * STEPS_PER_MM_Y);
}

/**
 * @brief Sets Z motor speed
 * 
 * @param speed_hz - speed in steps/s
 */
void motors_set_speed_z(uint32_t speed_hz) {
    stepper_z->setSpeedInHz(speed_hz);
}

/**
 * @brief Sets X motor acceleration
 * 
 * @param acceleration_mm_s - acceleration in mm/s^2
 */
void motors_set_acceleration_x(float acceleration_mm_s) {
    stepper_x->setAcceleration(acceleration_mm_s * STEPS_PER_MM_X);
}

/**
 * @brief Sets Y motor acceleration
 * 
 * @param acceleration_mm_s - acceleration in mm/s^2
 */
void motors_set_acceleration_y(float acceleration_mm_s) {
    stepper_y->setAcceleration(acceleration_mm_s * STEPS_PER_MM_Y);
}

/**
 * @brief Sets Z motor acceleration
 * 
 * @param acceleration_steps_s - acceleration in steps/s^2
 */
void motors_set_acceleration_z(int32_t acceleration_steps_s) {
    stepper_z->setAcceleration(acceleration_steps_s);
}

/**
 * @brief Starts moving to absolute position
 * Attention! Blocks the thread until the steppers start moving
 * 
 * @param x - new absolute X position in mm
 * @param y - new absolute Y position in mm
 */
void motors_move_to_position(float *x, float *y) {
    // Calculate new position in steps
    new_position_x_steps = *x * (float)STEPS_PER_MM_X;
    new_position_y_steps = *y * (float)STEPS_PER_MM_Y;

    // Check if it is a new position
    if (new_position_x_steps != stepper_x->getCurrentPosition()
        || new_position_y_steps != stepper_y->getCurrentPosition()) {
            // Move to new position
            stepper_x->moveTo(new_position_x_steps);
            stepper_y->moveTo(new_position_y_steps);

            // Wait for stepper to start moving
            while (!stepper_x->isRunning() && !stepper_y->isRunning());
        }
}

/**
 * @brief Enables motor drivers
 * 
 */
void motors_enable(void) {
    // Enable motor drivers
    stepper_x->enableOutputs();
    stepper_y->enableOutputs();
}

/**
 * @brief Disables motor drivers
 * 
 */
void motors_disable(void) {
    // Disable motor drivers
    stepper_x->disableOutputs();
    stepper_y->disableOutputs();
}

/**
 * @brief Checks if motors are stopped
 * 
 * @return boolean - true if motors are stopped
 */
boolean is_motors_stopped() {
    /*boolean stepper_x_decelerating_or_stopped = !stepper_x->isRunning()
        || stepper_x->rampState() == RAMP_STATE_DECELERATE
        || stepper_x->rampState() == RAMP_STATE_DECELERATE_TO_STOP
        || stepper_x->rampState() == RAMP_STATE_DECELERATING_FLAG;

    boolean stepper_y_decelerating_or_stopped = !stepper_y->isRunning()
        || stepper_y->rampState() == RAMP_STATE_DECELERATE
        || stepper_y->rampState() == RAMP_STATE_DECELERATE_TO_STOP
        || stepper_y->rampState() == RAMP_STATE_DECELERATING_FLAG;

    return stepper_x_decelerating_or_stopped && stepper_y_decelerating_or_stopped;*/

    return !stepper_x->isRunning() && !stepper_y->isRunning();
}

/**
 * @brief Stops motors with deceleration
 * 
 */
void motors_stop(void) {
    stepper_x->stopMove();
    stepper_y->stopMove();
}

/**
 * @brief Force stops and disables motors
 * 
 */
void motors_abort_and_reset(void) {
    // Stop motors without deceleration
    stepper_x->forceStop();
    stepper_y->forceStop();

    // Disable motor drivers
    motors_disable();

    // Reset absolute position
    stepper_x->setCurrentPosition(0);
    stepper_y->setCurrentPosition(0);
}

/**
 * @brief Enable output of Z motor
 * 
 */
void motors_enable_z(void) {
    stepper_z->enableOutputs();
}

/**
 * @brief Disables output of Z motor
 * 
 */
void motors_disable_z(void) {
    stepper_z->disableOutputs();
}

/**
 * @brief Start spinning z motor
 * 
 */
void motors_start_z(void) {
    stepper_z->moveByAcceleration(stepper_z->getAcceleration());
}

/**
 * @brief Stops spinning z motor
 * 
 */
void motors_stop_z(void) {
    stepper_z->stopMove();
}

/**
 * @brief Checks if Z motor is stopped
 * 
 * @return boolean - true if motor is stopped
 */
boolean is_motor_z_stopped() {
    return !stepper_z->isRunning();
}
