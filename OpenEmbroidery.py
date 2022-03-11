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

import math
import os
import os.path
import shutil
import sys
import tempfile

import pyembroidery
from PyQt5 import uic, QtGui, Qt
from PyQt5.QtGui import QPen, QColor
from PyQt5.QtWidgets import QApplication, QFileDialog, QGraphicsEllipseItem, \
    QGraphicsRectItem
from PyQt5.QtWidgets import QMainWindow

from GCodeReader import GCodeReader


def _map(x, in_min, in_max, out_min, out_max):
    """
    Re-maps a number from one range to another
    :param x: the number to map
    :param in_min: the lower bound of the value’s current range
    :param in_max: the upper bound of the value’s current range
    :param out_min: the lower bound of the value’s target range
    :param out_max: the upper bound of the value’s target range
    :return: the mapped value
    """
    return int((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)


class Window(QMainWindow):
    def __init__(self):
        super(Window, self).__init__()

        # Source embroidery pattern
        self.pattern = None

        # Input file name
        self.input_file_name = ''

        # GCode temp file
        self.gcode_file = None
        self.gcode_io = None

        # Hoop Qt item
        self.hoop = None

        # Number of colors
        self.number_of_colors = 0

        # GCode flags
        self.is_thread_pulled_out = False
        self.is_thread_tensioned = False

        # Counter for trim
        self.stitch_counter = 0

        # Load GUI file
        uic.loadUi('gui.ui', self)
        self.show()

        # Connect GUI controls
        self.btn_browse_input_file.clicked.connect(self.browse_input_file)
        self.btn_generate.clicked.connect(self.gcode_generate)
        self.btn_export.clicked.connect(self.gcode_export)
        self.hoop_square.clicked.connect(self.draw_hoop)
        self.hoop_round.clicked.connect(self.draw_hoop)
        self.hoop_none.clicked.connect(self.draw_hoop)
        self.hoop_width.valueChanged.connect(self.draw_hoop)
        self.hoop_height.valueChanged.connect(self.draw_hoop)

        # Initialize PyQTGraph
        self.init_graph()

        # Draw hoop
        self.draw_hoop()

    def init_graph(self):
        """
        Initializes PyQTGraph widget
        :return:
        """
        self.graphWidget.setBackground(QtGui.QColor('white'))
        self.graphWidget.getViewBox().invertY(True)
        self.graphWidget.showGrid(x=True, y=True, alpha=1.0)
        self.graphWidget.setAspectLocked(True)
        self.graphWidget.addLegend()

    def draw_hoop(self):
        """
        Draws hoop
        :return:
        """
        # Remove previous hoop
        if self.hoop is not None:
            self.graphWidget.removeItem(self.hoop)
            self.hoop = None

        # Get width and height
        width = self.hoop_width.value()
        height = self.hoop_height.value()

        # Square hoop
        if self.hoop_square.isChecked():
            self.hoop = QGraphicsRectItem(-width / 2, -height / 2, width, height)

        # Round hoop
        elif self.hoop_round.isChecked():
            self.hoop = QGraphicsEllipseItem(-width / 2, -height / 2, width, height)

        # No hoop
        if self.hoop_square.isChecked() or self.hoop_round.isChecked():
            pen = Qt.QPen()
            pen.setWidth(1)
            self.hoop.setPen(pen)
            self.graphWidget.addItem(self.hoop)

    def browse_input_file(self):
        """
        Opens a file selection dialog
        :return:
        """
        # Select file
        options = QFileDialog.Options()
        filename, _ = QFileDialog.getOpenFileName(self, 'Browse input file', '',
                                                  'Embroidery file (*.pes *.dst *.exp *.jef *.vp3)', options=options)

        if filename:
            # Print selected file
            self.input_file.setText(filename)
            print('Selected file: ' + filename)

            # Save filename without extension
            self.input_file_name = os.path.splitext(filename)[0]

    def gcode_generate(self):
        """
        Converts file to g-code
        :return:
        """
        filename = self.input_file.text()
        if os.path.isfile(filename):
            # Read input file
            self.pattern = pyembroidery.read(filename)

            # Delete previous temp file
            if self.gcode_file is not None:
                try:
                    os.remove(self.gcode_file)
                except Exception as e:
                    print('Error removing previous temp file', e)

            # Create new temp file
            self.gcode_file = tempfile.NamedTemporaryFile(delete=True).name
            self.gcode_io = open(self.gcode_file, 'w+')
            print('Temp file: ', self.gcode_file)

            # Reset flags
            self.is_thread_pulled_out = False
            self.is_thread_tensioned = False

            # Reset stitch counter
            self.stitch_counter = 0

            # Color counter
            color_counter = 1

            # Progress storage to update every %
            progress_last = 0

            # Get speeds from spin boxes
            jump_speed = int(self.spinbox_speed_jump.value())
            stitch_speed = int(self.spinbox_speed_stitch.value())
            z_low_speed = int(self.spinbox_speed_z_low.value())
            z_high_speed = int(self.spinbox_speed_z_high.value())

            # Get accelerations from spin boxes
            acceleration_x = int(self.spinbox_acc_x.value())
            acceleration_y = int(self.spinbox_acc_y.value())
            acceleration_z_low = int(self.spinbox_acc_z_max.value() / math.pow(z_high_speed / z_low_speed, 2))
            acceleration_z_high = int(self.spinbox_acc_z_max.value())

            # Get clearance from spinbox
            if self.checkbox_clearance.isChecked():
                clearance = float(self.spinbox_clearance.value())
            else:
                clearance = 0.

            # X and Y position storage for clearance
            x_prev = 0
            y_prev = 0

            # Get scaling factor from spin box
            scaling_factor = self.spinbox_scaling_factor.value()

            # Write input file name to gcode as comment
            self.write_gcode_line('; ' + os.path.basename(self.input_file_name).lower())

            # Enable motors
            self.write_gcode_line('M17')

            # Reset progress
            self.write_gcode_line('M73 P0')

            # Set accelerations
            self.write_gcode_line('M201 X' + str(acceleration_x) + ' Y' + str(acceleration_y)
                                  + ' Z' + str(acceleration_z_low))

            # Wait some time
            self.write_gcode_line('G4 P500')

            # Calculate number of colors and min and max points
            self.number_of_colors = 1
            x_min = math.inf
            y_min = math.inf
            x_max = -math.inf
            y_max = -math.inf

            # Loop all stitches
            for stitch in self.pattern.get_as_stitches():
                # Get stitch
                stitch_x = stitch[1]
                stitch_y = stitch[2]
                stitch_type = stitch[3]

                # New color -> increment number of colors
                if stitch_type == pyembroidery.COLOR_CHANGE:
                    self.number_of_colors += 1

                # Convert to x and y
                x = round(float(stitch_x / scaling_factor), 2)
                y = round(float(stitch_y / scaling_factor), 2)

                # Calculate min and max points
                if x < x_min:
                    x_min = x
                if y < y_min:
                    y_min = y
                if x > x_max:
                    x_max = x
                if y > y_max:
                    y_max = y

            # Whether the moving to extreme points is enabled
            if self.checkbox_move_min_max.isChecked():
                # Cycle all min-max points
                self.write_gcode_line('G0 X' + str(x_min) + ' Y' + str(y_min) + ' F' + str(jump_speed))
                self.write_gcode_line('G4 P500')
                self.write_gcode_line('G0 X' + str(x_min) + ' Y' + str(y_max) + ' F' + str(jump_speed))
                self.write_gcode_line('G4 P500')
                self.write_gcode_line('G0 X' + str(x_max) + ' Y' + str(y_max) + ' F' + str(jump_speed))
                self.write_gcode_line('G4 P500')
                self.write_gcode_line('G0 X' + str(x_max) + ' Y' + str(y_min) + ' F' + str(jump_speed))
                self.write_gcode_line('G4 P1000')

            # Move to the center
            self.write_gcode_line('G0 X0 Y0 ' + 'F' + str(jump_speed))
            self.write_gcode_line('G4 P500')

            # Request first color
            self.write_gcode_line('M0 C' + str(color_counter))

            # Loop all stitches
            for stitch in self.pattern.get_as_stitches():
                stitch_index = stitch[0]
                stitch_x = stitch[1]
                stitch_y = stitch[2]
                stitch_type = stitch[3]

                x = float(stitch_x / scaling_factor)
                y = float(stitch_y / scaling_factor)
                x_str = str(round(x, 2))
                y_str = str(round(y, 2))
                progress = int(_map(stitch_index, 0, self.pattern.count_stitches(), 0, 100))

                # Update progress every 1%
                if abs(progress - progress_last) >= 1:
                    progress_last = progress
                    self.write_gcode_line('M73 P' + str(progress))

                # Stitch type is anything but STITCH
                if stitch_type != pyembroidery.STITCH:
                    # Clear stitch counter
                    self.stitch_counter = 0

                    # Stop and disable main motor
                    self.write_gcode_line('M5')

                    # Decrease tension
                    self.gcode_set_thread_tension(False)

                    # Reset previous stitch position
                    x_prev = 0
                    y_prev = 0

                ################
                #     JUMP     #
                ################
                if stitch_type == pyembroidery.JUMP:
                    # Jump to position
                    self.write_gcode_line('G0 X' + x_str + ' Y' + y_str + ' F' + str(jump_speed))

                ##################
                #     STITCH     #
                ##################
                elif stitch_type == pyembroidery.STITCH:
                    # Thread is not pulled out
                    if not self.is_thread_pulled_out:
                        # Jump to stitch position
                        self.write_gcode_line('G0 X' + x_str + ' Y' + y_str + ' F' + str(jump_speed))

                        # Decrease tension
                        self.gcode_set_thread_tension(False)

                        # Pause to insert and pull out the thread (code A)
                        self.write_gcode_line('M0 C100')

                        # Set pulled out flag
                        self.is_thread_pulled_out = True

                        # Request tension increase
                        self.is_thread_tensioned = False

                    # Calculate distance between stitches
                    distance = math.sqrt(pow(x - x_prev, 2) + pow(y - y_prev, 2))

                    # Check spacing between stitches
                    if distance >= clearance or (x_prev == 0 and y_prev == 0):
                        # Store position for next cycle
                        x_prev = x
                        y_prev = y

                        # Increase tension
                        self.gcode_set_thread_tension(True)

                        # Move to stitch position
                        self.write_gcode_line('G1 X' + x_str + ' Y' + y_str + ' F' + str(stitch_speed))

                        # Set low acceleration
                        if self.stitch_counter == 0:
                            self.write_gcode_line('M201 X' + str(acceleration_x) + ' Y' + str(acceleration_y)
                                                  + ' Z' + str(acceleration_z_low))

                        # Make stitch with low speed
                        if self.stitch_counter <= 5:
                            self.write_gcode_line('M3 S' + str(z_low_speed) + ' I1')
                            self.stitch_counter += 1

                        # Make stitch with high speed
                        else:
                            # Set high acceleration
                            if self.stitch_counter == 6:
                                self.write_gcode_line('M201 X' + str(acceleration_x) + ' Y' + str(acceleration_y)
                                                      + ' Z' + str(acceleration_z_high))
                                self.stitch_counter += 1

                            # Make stitch
                            self.write_gcode_line('M3 S' + str(z_high_speed) + ' I1')

                        # Pause to trim
                        if self.stitch_counter == 5:
                            self.write_gcode_line('M0 C101')
                            self.stitch_counter += 1

                    # Spacing between stitches is too small
                    else:
                        # Jump to stitch position
                        self.write_gcode_line('G0 X' + x_str + ' Y' + y_str + ' F' + str(jump_speed))

                ################
                #     TRIM     #
                ################
                elif stitch_type == pyembroidery.TRIM:
                    # Jump to position
                    self.write_gcode_line('G0 X' + x_str + ' Y' + y_str + ' F' + str(jump_speed))

                    # Pause to trim
                    self.write_gcode_line('M0 C101')

                    # Request pause to insert and pull out the thread
                    self.is_thread_pulled_out = False

                ########################
                #     COLOR_CHANGE     #
                ########################
                elif stitch_type == pyembroidery.COLOR_CHANGE:
                    # Change color
                    color_counter += 1
                    self.write_gcode_line('M0 C' + str(color_counter))

                    # Jump to position
                    self.write_gcode_line('G0 X' + x_str + ' Y' + y_str + ' F' + str(jump_speed))

                    # Request pause to insert and pull out the thread
                    self.is_thread_pulled_out = False

                ###############
                #     END     #
                ###############
                elif stitch_type == pyembroidery.END:
                    # Jump to position
                    self.write_gcode_line('G0 X' + x_str + ' Y' + y_str + ' F' + str(jump_speed))

            # End of file. Stop machine
            self.write_gcode_line('M5')

            # Disable motors
            self.write_gcode_line('M18')

            # Close file
            self.gcode_io.flush()
            self.gcode_io.close()

            # Draw G-code
            gcode_reader = GCodeReader(self.gcode_file, self.number_of_colors)
            points, colors = gcode_reader.parse_to_points()
            self.draw_gcode(points, colors)

    def gcode_set_thread_tension(self, tension):
        # Increase tension
        if tension:
            if not self.is_thread_tensioned:
                self.write_gcode_line('M42')
                self.write_gcode_line('G4 P500')
                self.is_thread_tensioned = True

        # Decrease tension
        else:
            if self.is_thread_tensioned:
                self.write_gcode_line('M41')
                self.write_gcode_line('G4 P500')
                self.is_thread_tensioned = False

    def write_gcode_line(self, line):
        """
        Writes line to file
        :param line: line as String
        :return:
        """
        if self.gcode_file is not None and self.gcode_io is not None:
            self.gcode_io.write(line + '\n')

    def gcode_export(self):
        """
        Exports generated G-code file
        :return:
        """
        if self.gcode_file is not None:
            options = QFileDialog.Options()
            filename, _ = QFileDialog.getSaveFileName(self, 'Export G-code file', self.input_file_name,
                                                      'G-code file (*.gcode)',
                                                      options=options)

            if filename:
                shutil.copyfile(self.gcode_file, filename)
                # Print selected file
                print('Exported to: ' + filename)

    def draw_gcode(self, points, colors):
        """
        Draws generated G-code
        :param points: array of points
        :param colors: array of colors
        :return:
        """
        if points is not None:
            # Clear graph
            self.graphWidget.clear()

            for i in range(len(points)):
                color_points = points[i]
                color = colors[i]

                pen = QPen(QColor(color[0], color[1], color[2]))
                pen.setWidth(0)

                # symbol ='o', symbolPen=pen, symbolBrush = 0.1
                self.graphWidget.plot(color_points[0], color_points[1], pen=pen, name='Color ' + str(i + 1))

            # Draw hoop
            self.draw_hoop()


if __name__ == '__main__':
    app = QApplication(sys.argv)
    app.setStyle('fusion')
    win = Window()
    sys.exit(app.exec_())
