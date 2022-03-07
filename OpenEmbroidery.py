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
                except:
                    print('Error removing previous temp file')

            # Create new temp file
            self.gcode_file = tempfile.NamedTemporaryFile().name
            self.gcode_io = open(self.gcode_file, 'w+')
            print('Temp file: ', self.gcode_file)

            # Current machine state (is machine running)
            machine_enabled = False

            # Current tension state
            machine_tension = False

            # Color counter
            color_counter = 1

            # Progress storage to update every 5%
            progress_last = 0

            # Get speeds from spin boxes
            jump_speed = self.spinbox_speed_jump.value()
            stitch_speed = self.spinbox_speed_stitch.value()

            # Get accelerations from spin boxes
            acceleration_x = self.spinbox_acc_x.value()
            acceleration_y = self.spinbox_acc_y.value()

            # Get scaling factor from spin box
            scaling_factor = self.spinbox_scaling_factor.value()

            # thread is inserted
            is_thread_inserted = False

            # Enable motors
            self.write_gcode_line('M17')

            # Reset progress
            self.write_gcode_line('M73 P0')

            # Set accelerations
            self.write_gcode_line('M201 X' + str(acceleration_x) + ' Y' + str(acceleration_y))

            # Wait some time
            self.write_gcode_line('G4 P1000')

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

            # Cycle all min-max points
            if self.checkbox_move_min_max.isChecked():
                self.write_gcode_line('G0 X' + str(x_min) + ' Y' + str(y_min) + ' F' + str(jump_speed))
                self.write_gcode_line('G4 P500')
                self.write_gcode_line('G0 X' + str(x_min) + ' Y' + str(y_max) + ' F' + str(jump_speed))
                self.write_gcode_line('G4 P500')
                self.write_gcode_line('G0 X' + str(x_max) + ' Y' + str(y_max) + ' F' + str(jump_speed))
                self.write_gcode_line('G4 P500')
                self.write_gcode_line('G0 X' + str(x_max) + ' Y' + str(y_min) + ' F' + str(jump_speed))
                self.write_gcode_line('G4 P500')

            # Move to the center
            self.write_gcode_line('G0 X0 Y0 ' + 'F' + str(jump_speed))

            # Request first color
            self.write_gcode_line('M0 C' + str(color_counter))

            # Loop all stitches
            for stitch in self.pattern.get_as_stitches():
                stitch_index = stitch[0]
                stitch_x = stitch[1]
                stitch_y = stitch[2]
                stitch_type = stitch[3]

                x = str(round(float(stitch_x / scaling_factor), 2))
                y = str(round(float(stitch_y / scaling_factor), 2))
                progress = int(_map(stitch_index, 0, self.pattern.count_stitches(), 0, 100))

                # Update progress every 5%
                if abs(progress - progress_last) >= 5:
                    progress_last = progress
                    self.write_gcode_line('M73 P' + str(progress))

                ################
                #     JUMP     #
                ################
                if stitch_type == pyembroidery.JUMP:
                    # Disable machine
                    if machine_enabled:
                        self.write_gcode_line('M5')
                        machine_enabled = False

                    # Decrease tension
                    if machine_tension:
                        self.write_gcode_line('M41')
                        self.write_gcode_line('G4 P500')
                        machine_tension = False

                    # Jump to position
                    self.write_gcode_line('G0 X' + x + ' Y' + y + ' F' + str(jump_speed))

                ##################
                #     STITCH     #
                ##################
                elif stitch_type == pyembroidery.STITCH:
                    # If threaded
                    if is_thread_inserted:
                        # Increase tension
                        if not machine_tension:
                            self.write_gcode_line('M42')
                            self.write_gcode_line('G4 P500')
                            machine_tension = True

                        # Enable machine
                        if not machine_enabled:
                            self.write_gcode_line('M3')
                            machine_enabled = True

                        # Move to position and make stitch
                        self.write_gcode_line('G1 X' + x + ' Y' + y + ' F' + str(stitch_speed))

                    # Jump to stitch and make pause to thread
                    else:
                        # Jump to stitch position
                        self.write_gcode_line('G0 X' + x + ' Y' + y + ' F' + str(stitch_speed))

                        # Decrease tension
                        self.write_gcode_line('M41')

                        # Pause to thread (code A)
                        self.write_gcode_line('M0 C100')

                        # Set threaded flag
                        is_thread_inserted = True

                        # Request tension increase
                        machine_tension = False

                ########################
                #     COLOR_CHANGE     #
                ########################
                elif stitch_type == pyembroidery.COLOR_CHANGE:
                    # Disable machine
                    if machine_enabled:
                        self.write_gcode_line('M5')
                        machine_enabled = False

                    # Decrease tension
                    if machine_tension:
                        self.write_gcode_line('M41')
                        self.write_gcode_line('G4 P500')
                        machine_tension = False

                    # Change color
                    color_counter += 1
                    self.write_gcode_line('M0 C' + str(color_counter))

                    # Request pause to insert thread
                    is_thread_inserted = False

                    # Jump to position
                    self.write_gcode_line('G0 X' + x + ' Y' + y + ' F' + str(jump_speed))

                ###############
                #     END     #
                ###############
                elif stitch_type == pyembroidery.END:
                    # Disable machine
                    if machine_enabled:
                        self.write_gcode_line('M5')
                        machine_enabled = False

                    # Decrease tension
                    if machine_tension:
                        self.write_gcode_line('M41')
                        self.write_gcode_line('G4 P500')
                        machine_tension = False

                    # Jump to position
                    self.write_gcode_line('G0 X' + x + ' Y' + y + ' F' + str(jump_speed))

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

    def write_gcode_line(self, line):
        """
        Writes line to file
        :param line: line as String
        :return:
        """
        if self.gcode_file is not None and self.gcode_io is not None:
            self.gcode_io.write(line + '\n')

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
