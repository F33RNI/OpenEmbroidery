"""
 Copyright (C) 2022 Fern H., OpenEmbroidery project

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
"""

import matplotlib.pyplot as plt


class GCodeReader:
    def __init__(self, file, number_of_colors):
        self.gcode_file = file
        self.number_of_colors = number_of_colors
        self.cmap = plt.get_cmap('hsv')

    def parse_to_points(self):
        """
        Parses GCode file to arrays of points and colors
        :return:
        """
        # Open file
        file = open(self.gcode_file, 'r')

        # Create arrays of points and colors
        points = []
        colors = []

        # Generate first color
        rgba = self.cmap(1.)
        current_color = [int(rgba[2] * 255), int(rgba[1] * 255), int(rgba[0] * 255)]
        color_counter = 0

        # First run flag
        color_started = False
        thread_inserted = False

        # Temp variables
        x = 0
        y = 0
        points_x = []
        points_y = []
        while True:
            # Read line from file
            line = file.readline()

            # Finish if no line
            if not line:
                # Finish current color
                if color_started and thread_inserted:
                    points.append([points_x, points_y])
                    colors.append(current_color)
                break

            # Strip current line
            gcode_line = line.strip()

            # Try to find G-code
            command = self.parse_code(gcode_line, 'G', -1)

            # G1
            if command == 1:
                # If color started
                if color_started and thread_inserted:
                    # Parse x and y position
                    x = self.parse_code(gcode_line, 'X', x)
                    y = self.parse_code(gcode_line, 'Y', y)

                    # Append positions to arrays
                    points_x.append(x)
                    points_y.append(y)

            # No G-code found
            else:
                # Try to find M-code
                command = self.parse_code(gcode_line, 'M', -1)

                # M0
                if command == 0:
                    # Parse pause code
                    pause_code = self.parse_code(gcode_line, 'C', 0)

                    # Current code is color changing
                    if 0 < pause_code < 100:
                        # Clear thread inserted flag
                        thread_inserted = False

                        # First run
                        if not color_started:
                            color_started = True

                        # New color
                        else:
                            # Finish current color
                            points.append([points_x, points_y])
                            colors.append(current_color)

                            # Clear X and Y arrays of points
                            points_x = []
                            points_y = []

                        # Create new color from cmap for next cycles
                        color_counter += 1
                        rgba = self.cmap(1. - color_counter / self.number_of_colors)
                        current_color = [int(rgba[2] * 255), int(rgba[1] * 255), int(rgba[0] * 255)]

                    # Current code is thread insertion
                    elif pause_code == 100:
                        thread_inserted = True

        # Close file
        file.close()

        # Return array of points and array of colors
        return points, colors

    def parse_code(self, gcode_line, code, default_value):
        """
        Tries to find the code value inside the gcode_line string. Returns a default_value on failure.
        :param gcode_line: line with gcode command (ex. G0 X50 Y100)
        :param code: gcode key (ex. X)
        :param default_value: the value to return if the key is not found
        :return: float value of the key (ex. 50.)
        """
        if code not in gcode_line:
            return default_value

        gcode_parts = gcode_line.split(' ')

        for gcode_part in gcode_parts:
            gcode_part = gcode_part.strip()
            if code in gcode_part:
                return float(gcode_part[gcode_part.index(code):].replace(code, ''))

        return default_value
