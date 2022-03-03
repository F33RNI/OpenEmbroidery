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

struct tm t;

/**
 * @brief Converts hour, minute, second, day, month and year to the milliseconds
 * 
 * @param hour 0 to 23
 * @param minute 0 to 59
 * @param second 0 to 59
 * @param day 1 to 31
 * @param month 0 to 11
 * @param year from 1990
 */
time_t date_time_to_epoch(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day, uint8_t month, uint16_t year) {
    // Check year
    if (year < 1900)
        return 0;
        
    // Create struct with provided numbers
    memset(&t, 0, sizeof(tm));
    t.tm_year = year - 1900;
    t.tm_mon = month;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = second;

    // Convert to epoch (add + UNIX_OFFSET if needed)
    return mktime(&t);
}
