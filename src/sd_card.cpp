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
#include "sd_card.hpp"

/**
 * @brief Initializes SD card
 * 
 * @return boolean - true if sd.begin is true
 */
boolean sd_card_setup() {
    // Initialize CS pin as OUTPUT
    pinMode(PIN_SD_CARD_CS, OUTPUT);

    // Start SD card and open root directory
    return sd.begin(PIN_SD_CARD_CS, SPI_HALF_SPEED);
}

/**
 * @brief Counts number of files in root directory
 * 
 */
void sd_card_count_files(void) {
    // Reset number of files
    number_of_files = 0;

    // Open root directory
    dir.open("/");

    // List all files
    while (file.openNext(&dir, O_RDONLY)) {
        // Increment counter if it's file and name is valid
        if (!file.isDir() && file.getName(file_name_temp, sizeof(file_name_temp)) && is_gcode())
            number_of_files++;
        file.close();
    }

#ifdef DEBUG
    serial->print("Counted: ");
    serial->print(number_of_files);
    serial->println(" files");
#endif
}

/**
 * @brief Gets counted number of files
 * 
 * @return uint32_t - number_of_files
 */
uint32_t sd_card_get_number_of_files() {
    return number_of_files;
}

/**
 * @brief Tries to get the next file from the memory card
 * 
 * @return boolean - true if successful
 */
boolean sd_card_read_next_file() {
    // Reset temp maximum value
    file_time_max_temp = 0;

    // Reset filename valid flag
    file_name_valid = false;

    // Set starting point at first run
    if (file_time_prev == 0)
        file_time_prev = UINT32_MAX;

    if (number_of_files > 0) {
        // Open root directory
        dir.open("/");

        // List all files
        while (file.openNext(&dir, O_RDONLY)) {
            if (!file.isDir()) {
                // Get file date time
                file.getModifyDateTime(&pdate, &ptime);

                // Convert to epoch
                file_time_current = date_time_to_epoch(FS_HOUR(ptime), FS_MINUTE(ptime), FS_SECOND(ptime), FS_DAY(pdate), FS_MONTH(pdate) - 1, FS_YEAR(pdate));

                // Find next newest file and get name of file
                if (file_time_current > file_time_max_temp && file_time_current < file_time_prev
                    && file.getName(file_name_temp, sizeof(file_name_temp)) && is_gcode()) {
                    // Store local maximum
                    file_time_max_temp = file_time_current;

                    // Store selected file
                    selected_file = file;

                    // Copy bytes of file name
                    memcpy(file_name, file_name_temp, sizeof(file_name));

                    // Set flag if at least one file exists
                    file_name_valid = true;
                }
            }
            file.close();
        }

        // Store current max file time for next call
        file_time_prev = file_time_max_temp;
    }

    // Return flag
    return file_name_valid;
}

/**
 * @brief Tries to get the previous file from the memory card
 * 
 * @return boolean - true if successful
 */
boolean sd_card_read_prev_file() {
    // Reset temp minimum value
    file_time_min_temp = UINT32_MAX;

    // Reset filename valid flag
    file_name_valid = false;

    if (number_of_files > 0) {
        // Open root directory
        dir.open("/");

        // List all files
        while (file.openNext(&dir, O_RDONLY)) {
            if (!file.isDir()) {
                // Get file date time
                file.getModifyDateTime(&pdate, &ptime);

                // Convert to epoch
                file_time_current = date_time_to_epoch(FS_HOUR(ptime), FS_MINUTE(ptime), FS_SECOND(ptime), FS_DAY(pdate), FS_MONTH(pdate) - 1, FS_YEAR(pdate));

                // Find previous newest file and get name of file
                if (file_time_current < file_time_min_temp && file_time_current > file_time_prev
                    && file.getName(file_name_temp, sizeof(file_name_temp)) && is_gcode()) {
                    // Store local minimum
                    file_time_min_temp = file_time_current;

                    // Store selected file
                    selected_file = file;

                    // Copy bytes of file name
                    memcpy(file_name, file_name_temp, sizeof(file_name));

                    // Set flag if at least one file exists
                    file_name_valid = true;
                }
            }
            file.close();
        }

        // Store current min file time for next call
        file_time_prev = file_time_min_temp;
    }

    // Return flag
    return file_name_valid;
}

/**
 * @brief Gets current file name
 * 
 * @return char* - file_name
 */
char *sd_card_get_file_name() {
    return file_name;
}

/**
 * @brief Closes current file and resets file counter
 * 
 */
void sd_card_reset_files(void) {
    selected_file.close();
    file_time_prev = 0;
}

/**
 * @brief Sets current file position to 0
 * 
 */
void sd_card_file_rewind(void) {
    selected_file.rewind();
}

/**
 * @brief Checks current file
 * 
 * @return boolean - true if isOpen() and isReadable()
 */
boolean sd_card_check_selected_file() {
    return selected_file.isOpen() && selected_file.isReadable();
}

/**
 * @brief Reads next line from file to the buffer
 * 
 * @return boolean - true if the line is read
 */
boolean sd_card_read_next_line() {
    return selected_file.fgets(buffer, sizeof(buffer)) > 1;
}

/**
 * @brief Returns current gcode line
 * 
 * @return char* - gcode line from file
 */
char *sd_card_get_buffer() {
    return buffer;
}

/**
 * @brief Checks if current file is accepted
 * 
 * @return boolean - true is file_name_temp ends with FILE_EXT_UPPER or FILE_EXT_LOWER
 */
boolean is_gcode() {
    return strcmp(FILE_EXT_UPPER, &file_name_temp[strlen(file_name_temp)-strlen(FILE_EXT_UPPER)]) == 0
            || strcmp(FILE_EXT_LOWER, &file_name_temp[strlen(file_name_temp)-strlen(FILE_EXT_LOWER)]) == 0;
}
