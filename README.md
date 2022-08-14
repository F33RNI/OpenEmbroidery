<h1 align="center">OpenEmbroidery</h1>

<div style="width:100%;text-align:center;">
    <p align="center">
        <img src="./PHOTOS/main.jpg" width="auto" height="400">
    </p>
</div>
<div style="width:100%;text-align:center;">
    <p align="center">
        <a href="https://twitter.com/f33rni"><img alt="Twitter" src="https://img.shields.io/twitter/url?label=My%20twitter&style=social&url=https%3A%2F%2Ftwitter.com%2Ff33rni" ></a>
        <img src="https://badges.frapsoft.com/os/v1/open-source.png?v=103" >
        <img src="https://img.shields.io/github/last-commit/XxOinvizioNxX/OpenEmbroidery" >
        <a href="https://github.com/XxOinvizioNxX/OpenEmbroidery/blob/main/LICENSE"><img src="https://img.shields.io/github/license/XxOinvizioNxX/OpenEmbroidery" ></a>
        <a href="https://github.com/XxOinvizioNxX/OpenEmbroidery/stargazers"><img src="https://img.shields.io/github/stars/XxOinvizioNxX/OpenEmbroidery" ></a>
    </p>
</div>

----------

## What is it?

Open Embroidery is an open source project that allows you to turn an ordinary sewing machine into an embroidery machine

The fabric is fixed in the hoop, which moves with the help of stepper motors. For simplicity and versatility, embroidery patterns are pre-converted to G-Code using application written in Python.  
The whole process is controlled by Arduino Mega.  

----------

## Hardware

<div style="width:100%;text-align:center;">
    <p align="center">
        <img src="./PHOTOS/motors.jpg" width="auto" height="300">
        <img src="./PHOTOS/main_motor.jpg" width="auto" height="300">
    </p>
</div>

Hoop moves by standard NEMA 17 stepper motors (as in 3D printers). As drivers, you can use A4988 or DRV8825

<div style="width:100%;text-align:center;">
    <p align="center">
        <img src="./PHOTOS/servo_tesioner.jpg" width="auto" height="300">
        <img src="./PHOTOS/needle_hall_sensor.jpg" width="auto" height="300">
    </p>
</div>

In order to move the fabric (jump) - it is necessary to loosen the tension of the thread. This is done by a servo connected to the thread tensioner.

The fabric must be moved when the needle is in a certain position (same as in normal sewing). For synchronization, a hall sensor is used. When the magnet passes near the sensor, the main motor starts to decelerate and the tissue moves

<div style="width:100%;text-align:center;">
    <p align="center">
        <img src="./PHOTOS/controller.jpg" width="auto" height="300">
        <img src="./PHOTOS/main_motor_driver.jpg" width="auto" height="300">
    </p>
</div>

G-Code files are read from the memory card. To control the entire system, one single rotary encoder with a button is used. To display information, a 20x4 display connected via the I2C bus is used.

----------

## Examples

<div style="width:100%;text-align:center;">
    <p align="center">
        <img src="./PHOTOS/pizza.jpg" width="auto" height="300">
        <img src="./PHOTOS/bee_and_glasses.jpg" width="auto" height="300">
    </p>
    <p align="center">
        <img src="./PHOTOS/sakura.jpg" width="auto" height="300">
        <img src="./PHOTOS/avocado.jpg" width="auto" height="300">
    </p>
</div>

This is just a small part of what this project can do...

.DST files can be found in `examples` folder

----------

## How to build this

Instruction is under development...

- At the start, you need to assemble a frame along which the hoop can move freely along two axes (as in the photos). In my case, drawer sliders were used. They are not very accurate, but their rigidity is usually sufficient.
- For movement, i used stepper motors with belts and racks. You need to move the hoop as quickly as possible, so it is recommended to use belts (used in 3d printers).
- To assemble the frame, you can find some parts in the 3D folder.
- Next, you need to assemble the electronics. I use 4 frame motor drivers (DRV8825 or A4988) and a DRV8825 driver at 40 volts to drive the sewing machine.
- For control panel, you need to connect an I2C display 20x4, a rotary encoder and a slot for a SD card (electronics schematic under development).
- For motors, set the current close to the maximum. (1.5-1.6A or 0.8V ref on driver for Nema17)
- After flashing the microcontroller, use OpenEmroidery.py to convert the embroidery template to a .gcode file, save it to a memory card and load it from the menu.
